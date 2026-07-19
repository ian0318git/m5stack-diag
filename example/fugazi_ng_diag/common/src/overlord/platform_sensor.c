/* $Id: platform_sensor.c,v 1.3 2013/12/18 06:32:58 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_sensor.c,v $
 *------------------------------------------------------------------
 * Filename:  platform_sensor.c
 *
 * Description: Overlord Maxim 1617A Diode Sensor I2C device.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "platform_sensor.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "platform_intr_test.h"
#include "goofy_i2c.h"

/* #define ALERT_DEBUG */

/* Function prototypes */
static int max1617a_read_current_temp(uint32_t, uint8_t *);

int show_snsr_reg(void);
int alter_snsr_reg(void);
int gen_snsr_alert(void);
int clear_snsr_alert(void);
int snsr_read(n2g_i2c_if_t *);
int snsr_util_menu(int);
int snsr_check_mfg_id(void);
int rd_ext_env_intr(void);
int set_threshold(int);
int read_offset(int);

extern uint32_t ich_i2c_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
extern uint32_t ich_i2c_write(n2g_i2c_dev_t *, ulong, uint8_t, char *);
extern uint32 err_report(dev_object_t *, char *, uint32);


/* Global variables */
/* Max1617A registers table. This device is command based. Register offset is
 * the command written to the device.
 */
static reg_info_t sensor_reg_table[] =
{
    {"Current local temperature",  MAX1617_CMD_RLTS,  READ_ONLY,
     {0},                          0xFF,              0x00},
    {"Local THIGH limit",          MAX1617_CMD_RLHN,  READ_ONLY,
     {0},                          0xFF,              0x7F},
    {"Local TLOW limit",           MAX1617_CMD_RLLI,  READ_ONLY,
     {0},                          0xFF,              0xC9},
    {"Current remote temperature", MAX1617_CMD_RRTE,  READ_ONLY,
     {0},                          0xFF,              0x00},
    {"Remote THIGH limit",         MAX1617_CMD_RRHI,  READ_ONLY,
     {0},                          0xFF,              0x7F},
    {"Remote TLOW limit",          MAX1617_CMD_RRLS,  READ_ONLY,
     {0},                          0xFF,              0xC9},
    {"Status byte",                MAX1617_CMD_RSL,   READ_ONLY,
     {0},                          0xFF,              0x00},
    {"Configuration byte",         MAX1617_CMD_RCL,   READ_ONLY,
     {0},                          0xFF,              0x00},
    {"Conversion rate byte",       MAX1617_CMD_RCRA,  READ_ONLY,
     {0},                          0xFF,              0x02},
    {"Configuration byte",         MAX1617_CMD_WCA,   WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Conversion rate byte",       MAX1617_CMD_WCRW,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Local THIGH limit",          MAX1617_CMD_WLHO,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Local TLOW limit",           MAX1617_CMD_WLLM,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Remote THIGH limit",         MAX1617_CMD_WRHA,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Remote TLOW limit",          MAX1617_CMD_WRLN,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"One-shot command",           MAX1617_CMD_OSHT,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Software POR",               MAX1617_CMD_SPOR,  WRITE_ONLY,
     {0},                          0xFF,              0x00},
    {"Manufacturer ID code",       MAX1617_CMD_MFGID, READ_ONLY,
     {0},                          0xFF,              0x4D},
    {"Device ID code",             MAX1617_CMD_DEVID, READ_ONLY,
     {0},                          0xFF,              0x01},
    {0, 0, 0, {0}, 0, 0},
};

/* Peek-n-poke registers read/write command conversion */
static max1617a_rd_wr_t rd_wr_table[] =
{
    {MAX1617_CMD_RCL,   MAX1617_CMD_WCA},
    {MAX1617_CMD_RCRA,  MAX1617_CMD_WCRW},
    {MAX1617_CMD_RLHN,  MAX1617_CMD_WLHO},
    {MAX1617_CMD_RLLI,  MAX1617_CMD_WLLM},
    {MAX1617_CMD_RRHI,  MAX1617_CMD_WRHA},
    {MAX1617_CMD_RRLS,  MAX1617_CMD_WRLN},
    {MAX1617_CMD_OSHT,  MAX1617_CMD_OSHT},
    {MAX1617_CMD_SPOR,  MAX1617_CMD_SPOR},
    {MAX1617_CMD_DEVID, MAX1617_CMD_DEVID},  /* read/read as terminator */
};
/* Conversion-Rate Control Byte text string */
static conv_rate_t conversion_rate_table[MAX1617_CRA_8HZ + 1] =
{
    {"0.0625"},
    {"0.125"},
    {"0.25"},
    {"0.5"},
    {"1"},
    {"2"},
    {"4"},
    {"8"},
};


/*
 * Diode Sensor Menu
 */
static submenu_xtable_t snsr_menu_table[] = {
    {"Show Max1617A registers",       (PFT)show_snsr_reg,     0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter Max1617A register",       (PFT)alter_snsr_reg,    0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Check Manufacture ID",          (PFT)snsr_check_mfg_id, 0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Generate Alert",                (PFT)gen_snsr_alert,    0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Clear Alert",                   (PFT)clear_snsr_alert,  0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Read External Env Intr Status", (PFT)rd_ext_env_intr,   0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Fixed Local THIGH limit",       (PFT)set_threshold,     0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Fixed Local TLOW limit",        (PFT)set_threshold,     1,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Fixed Remote THIGH limit",      (PFT)set_threshold,     2,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Fixed Remote TLOW limit",       (PFT)set_threshold,     3,
       0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Read offset",                   (PFT)read_offset,       0,
       0, (type_t(*)())0, 0, (PFT)0, 0},
};

#define SNSR_MENU_TABLE_SIZE (sizeof(snsr_menu_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t snsr_menu_primary_items[SNSR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t snsr_menu_secondary_items[SNSR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo snsrdiag = {
    "Temp. Sensor(MAX1617A) Utility Menu",  /* title */
    0,                                      /* title string */
    (PFT)menu_show_dflags,                  /* shows major flags */
    0,                                      /* generic prompt */
    0,                                      /* size */
    snsr_menu_primary_items,
};

static struct menuinfo *snsrdiagp = &snsrdiag;


/*******************************************************************************
 *
 * Function   : build_snsr_menu
 * Description: To build Temperature Sensor (MAX1617A) menu.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_snsr_menu (void)
{
    build_primary_submenu(snsr_menu_table, SNSR_MENU_TABLE_SIZE,
                          "Diode Sensor Utility Menu", &snsrdiagp);
    build_secondary_submenu(snsr_menu_table, SNSR_MENU_TABLE_SIZE,
                            snsr_menu_secondary_items);
    menu(&snsrdiag, snsr_menu_secondary_items, 0);
}


/*******************************************************************************
 *
 * Function   : rd_ext_env_intr
 * Description: 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int rd_ext_env_intr (void) {
    unsigned int sts;
    sts = get_platform_env_intr_stat();
	  
    printf("External environment interrupt is  %s  \n",
           (sts & EXT_ENV_INTR_EN) ? "On" : "OFF");

    return (PASSED);
}

int read_offset (int select) {
    
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint8_t old_data = 0, tmp;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n", __FUNCTION__);
        return (FAILED);
    }
    
    tmp = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    i2c_if->buf = (char *)&old_data;
    
    i2c_if->offset = tmp; 
    rc = snsr_read(i2c_if); 
    msleep(10);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C read on %#x.",
                      __FUNCTION__, i2c_if->offset);
        return FAILED;
    }
  
    return PASSED;
}



int set_threshold (int select) {
    
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint8_t old_data, new_data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n", __FUNCTION__);
        return (FAILED);
    }
    
    switch(select){
     case 0: 
     	i2c_if->offset = MAX1617_CMD_RLHN;
     break; 
     case 1: 
     	i2c_if->offset = MAX1617_CMD_RLLI;
     break; 
     case 2: 
     	i2c_if->offset = MAX1617_CMD_RRHI;
     break; 
     case 3: 
     	i2c_if->offset = MAX1617_CMD_RRLS; 
     break; 
    	default: 
    		printf("not support this threhold \n");
      break;
    }

    i2c_if->buf = (char *)&old_data;
    rc = snsr_read(i2c_if); 
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C read on %#x.",
                      __FUNCTION__, i2c_if->offset);
        return FAILED;
    }
    
    new_data = gethex_answer("Enter the data:", old_data, 0, 0x7F);
    
    switch(select){
     case 0: 
     	i2c_if->offset = MAX1617_CMD_WLHO;
     break; 
     case 1: 
     	i2c_if->offset = MAX1617_CMD_WLLM;
     break; 
     case 2: 
     	i2c_if->offset = MAX1617_CMD_WRHA;
     break; 
     case 3: 
     	i2c_if->offset = MAX1617_CMD_WRLN; 
     break; 
    	default: 
    		printf("not support this threhold \n");
      break;
    }
    
    
    i2c_if->buf = (char *)&new_data;
    rc = n2g_i2c_write(i2c_if);
    if (rc == FAILED) {
        printf("n2g_i2c_awrite() on %s failed \n", __FUNCTION__);
        return FAILED;
    }
	  
	  //rc = snsr_read(i2c_if); //read back to verify. 
	  

    return rc; 
}

/*******************************************************************************
 *
 * Function   : snsr_check_mfg_id
 * Description: Check Temperature Sensor(MAX1617A) MFG ID.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int snsr_check_mfg_id (void)
{
    n2g_i2c_if_t *i2c_if;
    uint rc = FAILED;
    sn_d data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* Xformers Diode Sensor does not have any registers to initialize.
     * If it does, call the common device driver init
     */
    /* rc = snsr.base.dev_object_fvt->dev_init(dev); * */

    /* Read Manufacturing ID to verify the access. */
    i2c_if->offset = MAX1617_CMD_MFGID;
    i2c_if->buf = (char *)&data;
 
    rc = snsr_read(i2c_if);
    if (rc == PASSED) {
        /* Got the Manufacturing ID register. Check for Maxim vendor */
        if ((data != MAX1617_MFG_ID) && (data != ADM1021_MFG_ID)) {
            cterr('f', 0, "%s: Expect %#X Got %#X for Maxim Mfg ID",
                          __FUNCTION__, MAX1617_MFG_ID, data);
            return (FAILED);
        } else {
            printf("The read-out ID  =  %#X\n", data);
        }
    } else {
        cterr('f', 0, "%s: Mfg ID register I2C read failed (rc = %#X).",
                      __FUNCTION__, rc);
    }
    return (rc);
}


/*******************************************************************************
 *
 * Function   : show_snsr_reg
 * Description: Display Temperature Sensor(MAX1617A) Registers.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_snsr_reg (void)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    uint16_t data;
    char temp;
    uint32 cmd;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }
    
    /* Setup I2C API interface struct */
    i2c_if->buf = (char *)&data;
    
    printf("\n%s Registers:\n", i2c_if->dev_name);

    /* Points to the beginning of the registers table */
    reg_p = &sensor_reg_table[0];

    /* Read registers value */
    while (reg_p->name) {
        if (!(reg_p->type & WRITE_ONLY)) {
            /* Write only regiser is not readable,
             * so skip them.
             */
            i2c_if->offset = reg_p->offset;
            i2c_if->size = sizeof(data);
      
            rc = snsr_read(i2c_if);
            if (rc != PASSED) {
                cterr('f', 0, "%s: I2C read %s @ %#x rc = %#x.",
                              __FUNCTION__, reg_p->name, reg_p->offset, rc);
                return (FAILED);
            }
     
            /* Display All registers info */
#ifdef DEBUG_SHOW_BRIEF
            cmd = DEV_SHOW_BRIEF;
#else
            cmd = DEV_SHOW_ALL;
#endif /* DEBUG_SHOW_BRIEF */

            /* Display Sensor info */
            switch (cmd) {
            case DEV_SHOW_ALL:
            case DEV_SHOW_CONFIG:
            case DEV_SHOW_REGISTERS:

                switch (reg_p->offset) {
                case MAX1617_CMD_RLTS:
                case MAX1617_CMD_RLHN:
                case MAX1617_CMD_RLLI:
                case MAX1617_CMD_RRTE:
                case MAX1617_CMD_RRHI:
                case MAX1617_CMD_RRLS:
                    temp = (char)data;
                    printf("%s is %d degrees C.\n", reg_p->name, temp);
                break;
                case MAX1617_CMD_RSL:
                case MAX1617_CMD_MFGID:
                case MAX1617_CMD_DEVID:
                    printf("%s is 0x%02x\n", reg_p->name, data);
                break;
                case MAX1617_CMD_RCL:
                    printf("%s is 0x%02x\n", reg_p->name, data);
                    printf("    Alert interrupt %s.\n",
                           (data & MAX1617_RCL_MASK) ? "Disabled" : "Enabled");
                    printf("    %s mode.\n", (data & MAX1617_RCL_STOP) ?
                           "Standby" : "One-shot or timer");
                break;
                case MAX1617_CMD_RCRA:
                    printf("%s is 0x%02x - ", reg_p->name, data);
                    if (data > MAX1617_CRA_8HZ) {
                        printf("Reserved\n");
                    } else {
                        printf("%s Hz\n", conversion_rate_table[data].string);
                    }
                break;
                default:
                    cterr('f', 0, "%s: Invalid command %02x.",
                                  __FUNCTION__, reg_p->offset);
                    return (FAILED);
                break;
                } /* endof switch(offset) */
            break;
            case DEV_SHOW_BRIEF:
                printf("@ %#x = 0x%02X ", reg_p->offset, data);
            break;
            default:
                assert(!"dev_1617_show");
            break;
            } /* endof switch(cmd) */
        } /* endof if (WRITE_ONLY) */

        reg_p++;  /* Get next register */
    } /* endof while */
    return(PASSED);
}


/*******************************************************************************
 *
 * Function   : alter_snsr_reg
 * Description: To alter Temperature Sensor (MAX1617A) Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int alter_snsr_reg (void)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    max1617a_rd_wr_t *rd_wr_p;
    uint8_t cmd;
    uint8_t old_data, new_data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }
    
    /* Setup I2C API interface struct */
    i2c_if->size = sizeof(uint8_t);
    
    printf("\nRegister number:\n");

    /* Parse through the register table to search for writeable registers */
    reg_p = &sensor_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name) {
        if (!(reg_p->type & READ_ONLY)) {
            /* Only Write_only or read_write register can be altered */
            printf("   0x%02X - %s\n", reg_p->offset, reg_p->name);
        }
        reg_p++;  /* update the register table pointer */
    }   /* endof while */

    /* Get the register offset to alter */
    cmd = gethex_answer("Enter the register number:", 0, 0, MAX1617_CMD_DEVID);

    /* Traverse through read/write conversion table. Using device ID register
     * which is read only command, as the terminator.
     */ 
    rd_wr_p = &rd_wr_table[0];

    while (rd_wr_p->wr != MAX1617_CMD_DEVID) {
        if (cmd == rd_wr_p->wr) {
            /* Valid register */
            break;
        }
        rd_wr_p++;  /* Update to the next entry */
    } /* endof while */

    if (rd_wr_p->wr == MAX1617_CMD_DEVID) {
        cterr('f', 0,"%s: %#x not writeable.", __FUNCTION__, cmd);
        return (FAILED);
    }

    /* Find the register text in the register table */
    reg_p = &sensor_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != cmd) {
        /* Not requested register */
        reg_p++;  /* update the register table pointer */
    }

    /* Got the read/write pair. Some registers cannot be read, but writeable */
    switch(cmd) {
    case MAX1617_CMD_OSHT:
    case MAX1617_CMD_SPOR:
        /* Write only register */
        old_data = 0;
    break;
    default:
        /* Readable registers. Read the register first. */
        i2c_if->buf = (char *)&old_data;
        i2c_if->offset = rd_wr_p->rd;  /*** offset for sensor register ??***/

        rc = snsr_read(i2c_if); 
        if (rc != PASSED) {
            cterr('f', 0, "%s: I2C read %s cmd %#x rc = %#x.",
                          __FUNCTION__, reg_p->name, reg_p->offset, rc);
            return (FAILED);
        }
        /* Got the data */
    break;
    } /* endof switch cmd */

    /* Get the new data */
    new_data = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    /* Write the new data */
    i2c_if->buf = (char *)&new_data;
    i2c_if->offset = cmd;

#ifdef SNSR_DEBUG
    printf("\nwrite %#x with cmd %#x\n", new_data, i2c_if.offset);
#endif /* SNSR_DEBUG */

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: I2C write %s cmd %#x rc = %#x.",
                      __FUNCTION__, reg_p->name, reg_p->offset, rc);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : gen_snsr_alert
 * Description: Generate Alert of Max1617A.  
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gen_snsr_alert (void)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint8_t data, temp;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }
    
    i2c_if->offset = MAX1617_CMD_RLHN;
    i2c_if->buf = (char *)&temp;
    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s read Local High threshold register "
                      "failed with rc = %#x.", __FUNCTION__, rc);
        return (FAILED);
   }
    
    /* let the threshold of Local Thigh to zero. 
     * then the max1617 will generate interrupt 
     */
    data = 0; 
    
    i2c_if->offset = MAX1617_CMD_WLHO;
    i2c_if->buf = (char *)&data;
    rc = n2g_i2c_write(i2c_if);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C write %#x to %#x.",
                      __FUNCTION__, data, i2c_if->offset);
        return (FAILED);
    }
    
    /* wait for interrupt generate */ 
    msleep(10);

    /*the interrupt is generated, restore the setting*/ 
    i2c_if->buf = (char *)&temp;
    rc = n2g_i2c_write(i2c_if);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C write %#x to %#x.",
                      __FUNCTION__, temp, i2c_if->offset);
        return (FAILED);
    }
    
    return rc; 
}


/*******************************************************************************
 *
 * Function   : clear_snsr_alert
 * Description: Clear Alert of Max1617A.  This function is porting
 *              from max1617a_clear_alert() which placed in 
 *              dev_max1617a.c
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 * NOTE: the max1617 has another device address MB_I2C_ADDR_MB_TEMP_ALRT (0xC)
 *       for clean up the interrupt. after read this address, the interrupt 
 *       should be clear. 
 *******************************************************************************
 */
int clear_snsr_alert (void)
{
    uint32_t rc;
    char err_buf[ERR_BUF_SIZE];
#if 0  /* old method via i2c read */
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    char err_buf[ERR_BUF_SIZE];
    uint16_t data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP_ALRT);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Setup I2C API interface struct */
    i2c_if->size = sizeof(data);
    i2c_if->buf = (char *)&data;
  
    i2c_if->offset = 0;  /* not used, but clear it anyway */
    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "max1617a_clear_alert() read failed. rc = %#x", rc);
        return (FAILED);
    }
    printf("%s: Address %#x generates alert\n", __FUNCTION__, data);
    return (PASSED);
 
#endif 

    /* new method: accessing FPGA directly. */
    rc = clean_env_alert(MB_I2C_ADDR_MB_TEMP_ALRT);

    if ((rc & MSK_GFY_I2C_STAT_STD_DONE) != 0){ 
        printf("%s: FPGA status reg is 0x%x\n", __FUNCTION__, rc);
        return (PASSED);
    } else {
        sprintf(err_buf, "%s: failed. FPGA stat reg is = 0x%x", __FUNCTION__, rc);
        return (FAILED);
    }
    
}

/*********************************************************************
 *
 * Function:  show_sensor_temp
 *
 * Description:  Display sensor temperatures.
 *
 * Inputs:  err_log - cterr if TRUE. printf if FALSE.
 *    format - Display format of display_format_t in common.h
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int
show_sensor_temp(int err_log, int format)
{
    n2g_i2c_if_t *i2c_if;
    uint8_t data;
    uint32_t rc;
    char temp;
    
    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* Setup I2C API interface struct */
    i2c_if->size = sizeof(data);
    i2c_if->buf = (char *)&data;
    
#ifdef READ_LOCAL_TEMP
    /* Read local temperature */
    i2c_if->offset = MAX1617_CMD_RLTS;    
    
    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to I2C read Local temperature register "
                      "with rc = %#x", __FUNCTION__, rc);
        return (FAILED);
    }

    temp = (char)data;

    switch(format) {
    case DISPLAY_M2M:
  printf("MAX1617LTEMP:%dC\n", temp);
  break;
    case DISPLAY_HCI:
    default:
  printf("\nMax1617A chip temperature is %d degrees Celsius\n", temp);
  break;
    } /* endof switch */
#endif /* READ_LOCAL_TEMP */

    /* Read remote temperature */
    i2c_if->offset = MAX1617_CMD_RRTE;
    
    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read Remote temperature "
                      " register with rc = %#x", __FUNCTION__, rc);
        return (FAILED);
    }

    temp = (char)data;

    switch(format) {
    case DISPLAY_M2M:
  printf("MAX1617RTEMP:%dC\n", temp);
  break;
    case DISPLAY_HCI:
    default:
  printf("CPU die temperature is %d degrees Celsius\n", temp);
  break;
    } /* endof switch */

    return(PASSED);
}

/**********************************************************************
 *
 * Function:  set_1617_alert
 *
 * Description:  Set the Alert condition of the external device
 *
 * Inputs:  delta - Temperature difference below current temperature.
 *    cur_t_hi - Points to the current remote temp high save area.
 *    err_buf - Points to the error buffer.
 *
 * Output:  PASSED/FAILED.
 *
 **********************************************************************
 */
int
set_1617_alert(int delta, sn_d *cur_t_hi, char *err_buf)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc = 0;
    sn_d snsr_data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n", __FUNCTION__);
        return (FAILED);
    }

    /* fixme */
    if (rc != PASSED) {
  sprintf(err_buf, "set_1617_alert() Device attach failed. rc = %#x", rc);
    } else {
  /* Read remote T high limit */
  i2c_if->buf = (char *)cur_t_hi;
  i2c_if->offset = MAX1617_CMD_RRHI;
  
  rc = snsr_read(i2c_if);
  if (rc != PASSED) {
      cterr('f', 0, "%s: Unable to read Remote Threshold "
           "High temp. rc = %#x", __FUNCTION__, rc);
  } else {
      /* Read current remote temperature */
      i2c_if->buf = (char *)&snsr_data;
      i2c_if->offset = MAX1617_CMD_RRTE;
      
      rc = snsr_read(i2c_if);
      if (rc != PASSED) {
          cterr('f', 0, "%s: Unable to read Remote Temp. "
         "rc = %#x", __FUNCTION__, rc);
      } else {
    /* Write Remote Threshold High temperature with current
     * Remote temperature - delta.
     */
    snsr_data -= delta;
    i2c_if->offset = MAX1617_CMD_WRHA;
    printf("platform_sensor.c :Not suppreted %d\n", __LINE__);
  exit(0);
    /*
    rc = iofpga_i2c_wr(dev_p->dev_addr, dev_p->rd_hd_size, i2c_if.offset, 
           i2c_if.size, (uint32_t *) i2c_if.buf);
    */

    if (rc != PASSED) {
        sprintf(err_buf, "set_1617_alert() Unable to write Remote "
             "Threshold High temp. rc = %#x", rc);
    } /* endof if write WRHA */
      } /* endof if read RRTE */
  } /* endof if read RRHI */
    } /* endof if attach */
    
    return(rc);
}

/**********************************************************************
 *
 * Function:  restore_1617_alert
 *
 * Description:  Restore Remote Temperature High of the external device
 *
 * Inputs:  cur_t_hi - Restored remote temp high.
 *    err_buf - Points to the error buffer.
 *
 * Output:  PASSED/FAILED.
 *
 **********************************************************************
 */
int
restore_1617_alert(sn_d cur_t_hi, char *err_buf)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc = 0;
    sn_d snsr_data;

    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n", __FUNCTION__);
        return (FAILED);
    }

    /* fixme */
    if (rc != PASSED) {
  sprintf(err_buf, "restore_1617_alert() Device attach failed. rc = %#x",
    rc);
    } else {
#ifdef ALERT_DEBUG
  i2c_if->buf = (char *)&snsr_data;
  i2c_if->offset = MAX1617_CMD_RSL;
  
  rc = snsr_read(i2c_if);
  if (rc == PASSED) {
      printf("\nStatus before restoring = %#x\n", snsr_data);
  }
#endif /* ALERT_DEBUG */
  /* Restore the Remote Threshold High temperature */
  i2c_if->offset = MAX1617_CMD_WRHA;
  snsr_data = cur_t_hi;
  i2c_if->buf = (char *)&snsr_data;
  printf("platform_sensor.c :Not suppreted %d\n", __LINE__);
  exit(0);
           
  if (rc != PASSED) {
      sprintf(err_buf, "restore_1617_alert() Unable to write Remote "
                       "Threshold High temp. rc = %#x", rc);
  } else {
      /* Read status to clear the interrupt condition. */
      /* Refer to the Status Register section of ADM1021A and Status
       * Byte Functions of MAX1617A data sheets.
       */
      msleep(SNSR_CONV_TIME);
      i2c_if->buf = (char *)&snsr_data;
      i2c_if->offset = MAX1617_CMD_RSL;
      
      rc = snsr_read(i2c_if);

      if (rc != PASSED) {
          cterr('f', 0, "%s: Unable to read "
         "Status register. rc = %#x", __FUNCTION__, rc);
      } else {
#ifdef ALERT_DEBUG
    printf("\nRestored to %#x\n", cur_t_hi);
    printf("New status = %#x\n", snsr_data);
#endif /* ALERT_DEBUG */
    /* Ready to clear pending Alert condition */
    msleep(SNSR_CONV_TIME);
    rc = clear_snsr_alert();
    if (rc != PASSED) {
        sprintf(err_buf, "restore_1617_alert() Unable to clear "
             "pending Alert condition. rc = %#x", rc);
    } /* endof if clear_snsr_alert */
      } /* endof if n2g_i2c_read */
  } /* endof if write WRHA */
    } /* endof if attach */

    return(rc);
}


/*********************************************************************
 *
 * Function:	snsr_read
 *
 * Description:	Local Read Max1617 Register.
 *		Sensor IOFPGA read has 2 I2C operations. The I2C write with
 *		the register offset. Then wait for the REN_I2C_PROC_TIME
 *		milliseconds to allow the Sensor firmware to setup
 *		the data of the requested register. Then the I2C read will
 *		return the data.
 *
 * Inputs:	i2c_if - pointer to the I2C API struct.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int snsr_read (n2g_i2c_if_t *i2c_if)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc = FAILED;
    char *reg_data;
    char reg_tmp[32];

    reg_data = &reg_tmp[0];

    memcpy(&new_i2c_if, i2c_if, sizeof(new_i2c_if));
    new_i2c_if.buf = reg_data;

    rc = n2g_i2c_read(&new_i2c_if);
    if (rc != PASSED) {
	/* Unable to read data */
	printf("%s: Failed to I2C read (rc = 0x%08x).",
                __FUNCTION__, rc);
	return (rc);
    }

    *((sn_d *)i2c_if->buf) = (*(sn_d *)new_i2c_if.buf);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : max1617a_read_current_temp
 * Description:	To read current temperature from MAX1617A Register.
 * Inputs     : tmp_type - to determine local or remote temperature to read 
 *              tmp_val - buffer to put read back temperature value
 * Outputs    : PASSED/FAILED
 * 
 *******************************************************************************
 */
static int max1617a_read_current_temp (uint32_t tmp_type, uint8_t *tmp_val)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    
    /* init i2c_if for I2C */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C_ADDR_MB_TEMP);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* Setup I2C API interface struct */
    i2c_if->size = sizeof(uint8_t);
    i2c_if->buf = (char *)tmp_val;
    
    /* Read remote temperature */
    i2c_if->offset = tmp_type;
    
    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read Temperature from Reg %#x (rc = %#x).",
                      __FUNCTION__, i2c_if->offset, rc);
        return (FAILED);
    }
     
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : max1617a_read_remote_temp
 * Description:	To read remote temperature from MAX1617A Register.
 * Inputs     : temp_val - buffer to put read back current remote temperature
 * Outputs    : PASSED/FAILED
 * 
 *******************************************************************************
 */
int max1617a_read_remote_temp (uint8_t *temp_val)
{
    return (max1617a_read_current_temp(MAX1617_CMD_RRTE, temp_val));
}


/*------------------------------------------------------------------
$Log: platform_sensor.c,v $
Revision 1.3  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.2  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.6  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.5  2012/08/22 02:28:41  palin2
Add Cavium CPU temperature display in Overlord Diag boot-up message.

Revision 1.4  2012/05/31 14:24:40  palin2
Clean up compile warnings.

Revision 1.3  2012/04/25 07:50:06  alpeng
support clean alert via accessing FPGA directly

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

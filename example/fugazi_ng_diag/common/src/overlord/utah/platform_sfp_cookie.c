/* $Id: platform_sfp_cookie.c,v 1.13 2019/04/18 18:26:55 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_sfp_cookie.c,v $
 *------------------------------------------------------------------
 * platform_sfp_cookie.c
 *
 * Description: Overlord SFP Cookie I2C device.
 *              This file is ported from Informers. 
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "platform_sfp_cookie.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "pca9545a.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "i2c_address.h"
#include "nvmonvars.h"
#include "dash_fpga.h"

#define ALTER_SFP_COOKIE   1

/******************************************************************************
 *                                   Externs
 ******************************************************************************/
extern uint32 err_report (dev_object_t *dev, char *err_msg, uint32 err_type); /* in hwic_spidey_ct3.c */
extern int has_sfp(void);
extern boolean has_sfp2(void);
extern int is_sfp_present(int);
extern uint32_t api_mb_i2c_read(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern uint32_t api_mb_i2c_write(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern int init_mux(int);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);

//extern int32_t cavium_i2c_fd1;


/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/
static int sfp_i2c_test(n2g_i2c_dev_t *);
static int show_cookie_wrap(void);
static int set_pid_to_sfp(int);
static int clear_sfp_pid(int);

#ifdef ALTER_SFP_COOKIE
static int at_alter_eeprom_wrap(void);
#endif /* ALTER_SFP_COOKIE */

static int      get_sfp_mux_mask(uint);
static uint16_t get_eeprom_size(uint8_t);
static int sfp_i2c_oper(char, char, n2g_i2c_if_t *);
int set_sfp_glc_t_1000(void);


/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/
static uint ovld_sfp_no = NUMBER_OF_SFP;
static uint8_t ovld_sfp_data_type = AT24C_02; 
static n2g_i2c_dev_t i2c_sfp;
static uint8_t sfp_mux_mask = 0;
 
/*
 * SFP PID table - Each SFP has a PID locked with it.
 * Since the SFP submenu cannot pass the calling SFP enum to each utlity
 * or test, we use the invoking PID of the utility or test of the SFP
 * submenu to retrieve the SFP enum.
 * All entries are initialized to INVALID_PID (-1). When the SFP enum is
 * passed to the SFP submenu, the SFP enum is also passed. The entry of
 * the called SFP PID is updated to the calling process PID.
 * For single thread, only one entry has a valid PID.
 */
static pid_t sfp_pid_table[NUMBER_OF_SFP] = {
    INVALID_PID,	/* SFP0 */
    INVALID_PID,	/* SFP1 */
    INVALID_PID,	/* SFP2 */
    INVALID_PID,	/* SFP3 */
};


/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/*
 * SFP Cookie Menu
 */
static submenu_xtable_t cookie_menu_table[] = {
    {"Show contents",    (PFT)show_cookie_wrap,       0, 0,
     (type_t(*)())0, 0,  (PFT)show_cookie_wrap,       0},
#ifdef ALTER_SFP_COOKIE
    {"Alter SFP Cookie", (PFT)at_alter_eeprom_wrap,   0, 0,
     (type_t(*)())0, 0,  (PFT)at_alter_eeprom_wrap,   0},
#endif /* ALTER_SFP_COOKIE */
};

#define COOKIE_MENU_TABLE_SIZE (sizeof(cookie_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cookie_menu_primary_items[COOKIE_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t cookie_menu_secondary_items[COOKIE_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo cookiediag = {
    "SFP Cookie Utility Menu",	    /* title */
    0,				    /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	    /* shows major flags */
    0,				    /* generic prompt */
    0,				    /* size -- bumped by add_menu_item() */
    cookie_menu_primary_items,
};

static struct menuinfo *cookiediagp = &cookiediag;

/*
 * Contents Description table
 */
static dev_at24c0n_desc_t sfp_desc_table[] = {
    {"Identifier",                 SFP_COO_ID,    SFP_COO_ID_L,	  AT_DESC_HEX},
    {"Ext. Identifier",	           SFP_COO_X_ID,  SFP_COO_XID_L,  AT_DESC_HEX},
    {"Connector",                  SFP_COO_CNT,	  SFP_COO_CNT_L,  AT_DESC_HEX},
    {"Transceiver",                SFP_COO_XVR,	  SFP_COO_XVR_L,  AT_DESC_HEX},
    {"Encoding",                   SFP_COO_ENC,   SFP_COO_ENC_L,  AT_DESC_HEX},
    {"BR, Nominal",                SFP_COO_BR_N,  SFP_COO_BR_N_L, AT_DESC_DEC},
    {"Length(9m) - km",            SFP_COO_L_9KM, SFP_COO_L_9KM_L,AT_DESC_DEC},
    {"Length (9m)",                SFP_COO_L_9M,  SFP_COO_L_9M_L, AT_DESC_DEC},
    {"Length (50m)",               SFP_COO_L_50,  SFP_COO_L_50_L, AT_DESC_DEC},
    {"Length (62.5m)",             SFP_COO_L_62,  SFP_COO_L_62_L, AT_DESC_DEC},
    {"Length (Copper)",            SFP_COO_L_CU,  SFP_COO_L_CU_L, AT_DESC_DEC},
    {"Vendor Name",                SFP_COO_VEND,  SFP_COO_VEND_L, AT_DESC_TXT},
    {"Channel Spacing",            SFP_COO_CH_S,  SFP_COO_CH_S_L, AT_DESC_DEC},
    {"Vendor OUI",                 SFP_COO_VEN_O, SFP_COO_VEN_O_L, AT_DESC_HEX},
    {"Vendor P/N",                 SFP_COO_VEN_PN,SFP_COO_VEN_P_L,AT_DESC_TXT},
    {"Vendor Rev",	           SFP_COO_VEN_R, SFP_COO_VEN_R_L,AT_DESC_TXT},
    {"Laser Wavelength",           SFP_COO_LSR_W, SFP_COO_LSR_W_L,AT_DESC_HEX},
    {"DWDM Wavelength Fraction",   SFP_COO_DWDM_W,SFP_COO_DWDM_L, AT_DESC_HEX},
    {"CC_BASE Checksum",           SFP_COO_CC_B,  SFP_COO_CC_B_L, AT_DESC_HEX},
    {"Options",                    SFP_COO_OPT,   SFP_COO_OPT_L,  AT_DESC_HEX},
    {"BR, Max",                    SFP_COO_BR_MAX,SFP_COO_BR_MX_L,AT_DESC_DEC},
    {"BR, Min",	                   SFP_COO_BR_MIN,SFP_COO_BR_MN_L,AT_DESC_DEC},
    {"Vendor SN",                  SFP_COO_VEN_SN,SFP_COO_VEN_S_L,AT_DESC_TXT},
    {"Date code",                  SFP_COO_DATE,  SFP_COO_DATE_L, AT_DESC_DEC},
    {"Diagnostic Monitoring type", SFP_COO_DIAG,  SFP_COO_DIAG_L, AT_DESC_HEX},
    {"Enhanced Options",           SFP_COO_ENH,   SFP_COO_ENH_L,  AT_DESC_HEX},
    {"CC_EXT Checksum",	           SFP_COO_CC_X,  SFP_COO_CC_X_L, AT_DESC_HEX},
    {"Vendor Specific",	           SFP_COO_VEND_SP,SFP_COO_VN_SP_L,AT_DESC_HEX},
    {0, 0, 0, 0},	/* Terminator */
};


/******************************************************************************
 *
 * function   : build_sfp_cookie_menu
 * Description:	Build menu for SFP cookie related functions.
 * Inputs     : sfp - SFP I2C device number
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int build_sfp_cookie_menu (int sfp)
{
    char t_name[ERR_BUF_SIZE];
    uint sfp_id = ((uint)sfp - OVLD_TWSI_SFP0);
#ifdef MUX124
    uint rc = FAILED;
#endif 

    sprintf((char *)t_name, "SFP %d Cookie", sfp_id);

    testname(t_name);
#ifdef MUX124
    if (!is_goldbeach()) {
        /* Lock the PID to the SFP */
        if (set_pid_to_sfp(sfp_id) == FAILED) {
            cterr('f', 0, "build_sfp_cookie_menu Invalid SFP %#x", sfp_id);
            return (rc);
        }
        /* Record SFP No. */
        ovld_sfp_no = sfp_id;
        /* Get Mux mask */
        rc = get_sfp_mux_mask(sfp_id);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to get Mux mask (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
            return (rc);
        }
    
        /* Setup Mux channel */
        rc = set_mux_channel(&i2c_sfp, sfp_mux_mask, OVLD_SFP_I2C_MUX);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
            return (rc);
        }
    }
#endif 
    build_primary_submenu(cookie_menu_table, COOKIE_MENU_TABLE_SIZE,
                          "SFP Cookie Utility Menu", &cookiediagp);
    build_secondary_submenu(cookie_menu_table, COOKIE_MENU_TABLE_SIZE,
                            cookie_menu_secondary_items);
    menu(&cookiediag, cookie_menu_secondary_items, 0);

    /* Unlock the process to the SFP */
    clear_sfp_pid(sfp_id);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	get_sfp_mux_mask
 * Description:	Get the Mux Mask of the selected SFP device.
 * Inputs     : sfp_no - Number of the selected SFP device
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int get_sfp_mux_mask (uint sfp_no)
{
    switch (sfp_no) {
    case SFP_ZERO:
        sfp_mux_mask = I2C1_MUX_PORT0_MASK;
        break;
    case SFP_ONE:
        sfp_mux_mask = I2C1_MUX_PORT1_MASK;
        break;
    case SFP_TWO:
        sfp_mux_mask = I2C1_MUX_PORT2_MASK;
        break;
    case SFP_THREE:
        sfp_mux_mask = I2C1_MUX_PORT3_MASK;
        break;
    default:
        printf("%s:%d Unknown SFP No.\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    
    return (PASSED);
}


/******************************************************************************
 *
 * Function   : get_eeprom_size
 * Description: Get the EEPROM size - 1
 * Inputs     : dev_type
 * Output     : device size. 0, if invalid device type
 *  
 ******************************************************************************/
static uint16_t get_eeprom_size (uint8_t dev_type)
{
    uint16_t eeprom_size = 0;
    
    /* Get the EEPROM size */
    switch (dev_type) {
    case AT24C_01:
        eeprom_size = AT24C01_MAX;
        break;
    case AT24C_02:
        eeprom_size = AT24C02_MAX;
        break;
    case AT24C_04:
        eeprom_size = AT24C04_MAX;
        break;
    default:
        printf("%s:%d Unknown EEPROM type %#x",
               __FUNCTION__, __LINE__, dev_type);
        break;
    }
    return (eeprom_size);
}

#if 0 // hroni: no longer needed, now SFP mux uses FPGA I2C
/******************************************************************************
 *
 * Function   :	setup_i2c_dev_structure
 * Description:	Setup related data of SFP I2C device structure.
 * Inputs     : sfp_no - Number of the selected SFP device
 * Outputs    : I2C Address of the selected SFP device
 *
 ******************************************************************************/
static uint32_t setup_i2c_dev_structure (n2g_i2c_dev_t *i2c_sfp)
{   
    int rc = -1;
 
    /* Check SFP number */
    if (ovld_sfp_no >= NUMBER_OF_SFP) {
        printf("%s:%d Invalid SFP Number.", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Set up I2C device struct data */
    i2c_sfp->bus_no = CPU_I2C0;
    i2c_sfp->dev_addr = OVLD_SFP_I2C_ADDR;
    i2c_sfp->rd_hd_size = 1;
    i2c_sfp->wr_hd_size = 1;

    /* Set the selected SFP I2C device to SLAVE mode */
    if (cavium_i2c_fd1 <= 0) {
        printf("%s:%d /dev/i2c-octeon.1/ is not opened correctly.",
               __FUNCTION__, __LINE__);
        return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd1, I2C_SLAVE, i2c_sfp->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_sfp->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_sfp->fp = cavium_i2c_fd1;
        }
    }    
    return (PASSED);
}
#endif

/******************************************************************************
 *
 * function   : sfp_cookie_read
 * Description: Wrapper to read SFP cookie.
 * Inputs     : sfp - SFP I2C bus number
 *              offset - SFP cookie read starting offset.
 *              size - number of SFP cookie bytes to read.
 *              *data - Points to the buffer for the cookie to be read.
 *              err_log - TRUE to cterr. FALSE to printf.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int sfp_cookie_read (int sfp, int offset, int size, char * data, int err_log)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t new_i2c_if;

    /* Lock the PID to the SFP */
    if (set_pid_to_sfp(sfp) == FAILED) {
        if (err_log == TRUE) {
            cterr('f', 0, "%s:%d Invalid SFP %#x", __FUNCTION__, __LINE__, sfp);
        } else {
            printf("\n***%s:%d Invalid SFP %#x\n", __FUNCTION__, __LINE__, sfp);
        }
        return (FAILED);
    }
        
    /* Setup I2C API interface struct */
    new_i2c_if.offset = offset;
    new_i2c_if.i2c_bus_type = IOFPGA_I2C;
    new_i2c_if.i2c_dev = SFP_EEPROM_BASE; 
    new_i2c_if.i2c_ctrl = I2C_CTRL_SEVENTEEN;
    new_i2c_if.size = size;
    new_i2c_if.mux = I2C_MUX_ZERO;
    new_i2c_if.buf = data;
    
    //rc = api_mb_i2c_read(&i2c_sfp, offset, size, data);
    rc = n2g_i2c_read(&new_i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, offset, size, rc);
        return (FAILED);
    }

    /* Unlock the SFP I2C access */
    clear_sfp_pid(sfp);

    return(rc);
}


/******************************************************************************
 *
 * Function   : show_cookie
 * Description: Provide platforms with a mechanism to display some common
 *		          device information via the device print function argument.
 * Inputs     : dev_object_t pointer to the ATMEL EEPROM device
 *	            A device print function vector
 *              A dev_show_cmd_e command
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32 show_cookie (n2g_i2c_dev_t *i2c_sfp, dev_show_cmd cmd)
{
    uint32 rc;
    char data[4];   /* data bytes from ATMEL EERPOM */
    unsigned int i, j, text_i;
    uint16_t size;
    n2g_i2c_if_t new_i2c_if;
    dev_at24c0n_desc_t *pdesc;
    char text_buf[18], *buf;

    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = IOFPGA_I2C;
    new_i2c_if.i2c_dev = SFP_EEPROM_BASE; 
    new_i2c_if.i2c_ctrl = I2C_CTRL_SEVENTEEN;
    new_i2c_if.mux = I2C_MUX_ZERO;
    switch(cmd) {
    case DEV_SHOW_ALL:
        new_i2c_if.size = sizeof(data); 
        new_i2c_if.buf = &data[0];

        /* Get the EEPROM size */
        if ((size = get_eeprom_size(ovld_sfp_data_type)) == 0) {
            return (FAILED);
        }

        printf("\nSFP cookie EEPROM Contents:\n");

        for (i = 0, text_i = 0; i <= size; i += (sizeof(data))) {
            /* Read the data bytes of ATMEL EEPROM */
            new_i2c_if.offset = i;
            
#ifdef I2C_DEBUG
	          printf("%s:%d Now access offset %#x\n",
                   __FUNCTION__, __LINE__, new_i2c_if.offset);
#endif /* I2C_DEBUG */

            //rc = api_mb_i2c_read(i2c_sfp, new_i2c_if.offset,
            //                     new_i2c_if.size, (char *)new_i2c_if.buf);
            rc = n2g_i2c_read(&new_i2c_if);
            if (rc != PASSED) {
                printf("%s:%d Failed to read %#x(rc = %#x)",
                       __FUNCTION__, __LINE__, new_i2c_if.offset, rc);
                return (FAILED);
	          }

            if ((i % 16) == 0) {
                printf("\n 0x%04X : ", i);
	    }

            for (j = 0; j < sizeof(data); j++) {
	      printf(" %02X", (uchar)data[j]);
                if ((data[j] >= ' ') && (data[j] <= 'z')) {
                    text_buf[text_i + j] = data[j];
                } else {
                    text_buf[text_i + j] = '.';
                }
            }

            if (text_i == 12) {
                /* Last 4 bytes of a line. Print the text */
                text_buf[text_i + j] = '\0';
                printf("  %s", text_buf);
                text_i = 0; /* reinitialize the text buffer index */
            } else {
                printf(" ");
                text_i += sizeof(data);
            }

        } /* endof for */
        break;
    case DEV_SHOW_BRIEF:
        printf("\nSFP cookie Contents:\n");
        pdesc = &sfp_desc_table[0]; /* Get the first descriptor */

        while(pdesc->name) {
            new_i2c_if.size = pdesc->size;
            new_i2c_if.offset = pdesc->offset;

            buf = malloc(pdesc->size);
            if (buf == NULL) {
                printf("%s: malloc %d byte failed", __FUNCTION__, pdesc->size);
                return (FAILED);
            }      
            new_i2c_if.buf = buf;
            
	        //rc = api_mb_i2c_read(i2c_sfp, new_i2c_if.offset, new_i2c_if.size, (char *)new_i2c_if.buf);
            rc = n2g_i2c_read(&new_i2c_if);
            if (rc != PASSED) {
                free(buf);
                printf("%s: read %s %d bytes @ %#x failed. rc = %#x",
                       __FUNCTION__, pdesc->name, pdesc->size, pdesc->offset, rc);
                return (FAILED);
            }
            printf("%s @ %#x : ", pdesc->name, pdesc->offset);
            switch(pdesc->type) {
            case AT_DESC_HEX:
                printf("0x");
                for (i = 0; i < pdesc->size; i++) {
                    printf("%02x ", (uint8_t)buf[i]);
                } /* endof for */
                break;
            case AT_DESC_DEC:
                for (i = 0; i < pdesc->size; i++) {
                    printf("%d ", buf[i]);
                }
                break;
            case AT_DESC_TXT:
                for (i = 0; i < pdesc->size; i++) {
                    printf("%c", buf[i]);
                }
                break;
            default:
                assert(!"dev_24c0n_show - type");
                break;
            } /* endof switch type */
	          printf("\n");
            free(buf);
            pdesc++;
        } /* endof while */
        break;
    default:
        assert(!"dev_24c0n_show - cmd");
        break;
    } /* endof switch */

    return (PASSED);
}


/******************************************************************************
 *
 * Function   : show_cookie_wrap
 * Description:	Function wrap to display SFP Cookie Contents.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int show_cookie_wrap (void)
{
    uint32_t rc = FAILED;
    
    /* Display Cookie contents */
    rc = show_cookie(&i2c_sfp, DEV_SHOW_BRIEF);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to show cookie contents.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    rc = show_cookie(&i2c_sfp, DEV_SHOW_ALL); 
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to show cookie contents.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   : sfp_i2c_test
 * Description: Function for SFP i2c test. 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 * 
 ******************************************************************************/
static int sfp_i2c_test (n2g_i2c_dev_t *i2c_sfp_dev)
{
    uint32 rc;
    char data[4];   /* data bytes from ATMEL EERPOM */
    uint16_t size;
    n2g_i2c_if_t new_i2c_if;

    /* Setup I2C API interface struct */
    if (is_goldbeach()) {
        new_i2c_if.offset = 0;
        new_i2c_if.i2c_dev = SFP_EEPROM_BASE;
    } else {
        new_i2c_if.offset = 0xFFFFFFFF;
        new_i2c_if.i2c_dev = MB_I2C_ADDR_SFP_I2C_MUX;
    }
    new_i2c_if.i2c_bus_type = IOFPGA_I2C;
    new_i2c_if.i2c_ctrl = I2C_CTRL_SEVENTEEN;
    new_i2c_if.mux = I2C_MUX_ZERO;
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = &data[0];
    
    /* Get the EEPROM size */
    if ((size = get_eeprom_size(ovld_sfp_data_type)) == 0) {
        return (FAILED);
    }
    //rc = api_mb_i2c_read(i2c_sfp_dev, new_i2c_if.offset,
    //                     new_i2c_if.size, (char *)new_i2c_if.buf);
    rc = n2g_i2c_read(&new_i2c_if);
    if (rc != PASSED) {
        printf("%s:%d Failed to read %#x(rc = %#x)",
                   __FUNCTION__, __LINE__, new_i2c_if.offset, rc);
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   : sfp_i2c_test_warp
 * Description: Function for warp SFP i2c test. 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int sfp_i2c_test_warp (void)
{
    uint sfp_id;
    uint rc = FAILED, ia;
    if (is_goldbeach()) {
        /* Goldbeach without mux and only one SFP */
        /* Start i2c read test on SFP */
        rc = sfp_i2c_test(&i2c_sfp);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to test sfp i2c read.",
                          __FUNCTION__, __LINE__);
            return (rc);
        }
        return (rc);
    }
    /* init sfp and mux setting, then perform simple i2c read */
    /* for SFP0 to SFP3,OVLD_TWSI_1_INVALID = 5 */
    for (ia = OVLD_TWSI_SFP0; ia < OVLD_TWSI_1_INVALID; ia++) {
        sfp_id = ia - OVLD_TWSI_SFP0;

        /* Lock the PID to the SFP */
        if (set_pid_to_sfp(sfp_id) == FAILED) {
            cterr('f', 0, "build_sfp_cookie_menu Invalid SFP %#x", sfp_id);
            return (rc);
        }

        /* Record SFP No. */
        ovld_sfp_no = sfp_id;
        /* Get Mux mask */
        rc = get_sfp_mux_mask(sfp_id);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to get Mux mask (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
            return (rc);
        }

        /* Setup Mux channel */
        rc = set_mux_channel(&i2c_sfp, sfp_mux_mask, OVLD_SFP_I2C_MUX);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
            return (rc);
        }

        /* Start i2c read test on SFP */
        rc = sfp_i2c_test(&i2c_sfp);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to test sfp i2c read.",
                          __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }
    return (rc);
}


/******************************************************************************
 *
 * Function   : set_pid_to_sfp
 * Description:	Save the PID to the SFP/PID table.
 * Inputs     : sfp_id - SFP enum.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int set_pid_to_sfp (int sfp_id)
{
    /* Check the SFP enum to be valid */
    if (sfp_id >= NUMBER_OF_SFP) {
        cterr('f', 0, "set_pid_to_sfp() Invalid SFP %#x", sfp_id);
        return (FAILED);
    }

    if (sfp_pid_table[sfp_id] != INVALID_PID) {
        /* Another process is using it */
        if (getpid() != sfp_pid_table[sfp_id]) {
            cterr('f', 0, "set_pid_to_sfp() Process %#x is using the SFP",
                  sfp_pid_table[sfp_id]);
            return (FAILED);
        } else {
            /* Already locked with current Process */
        }
    } else {
        /* New entry */
        sfp_pid_table[sfp_id] = getpid();
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function   : clear_sfp_pid
 * Description: Reset the PID of the SFP in the SFP/PID table.
 * Inputs     : sfp_id - SFP enum
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int clear_sfp_pid (int sfp_id)
{
    /* Check the SMI enum to be valid */
    if (sfp_id >= NUMBER_OF_SFP) {
        cterr('f', 0, "clear_sfp_pid() Invalid SFP %#x", sfp_id);
        return (FAILED);
    }

    if (sfp_pid_table[sfp_id] != getpid()) {
        /* Another PID using this SFP */
        cterr('f', 0, "clear_sfp_pid(): pid %#x using SFP %#x",
                      sfp_pid_table[sfp_id], sfp_id);
        return (FAILED);
    } else {
        sfp_pid_table[sfp_id] = INVALID_PID;
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function:    sfp_i2c_oper
 *
 * Description:	Configure GLC-t 1000. Refer to vendor data sheet.
 *
 * Input:	i2c_bus - I2C bus enum.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int sfp_i2c_oper (char reg_high, char reg_low, n2g_i2c_if_t *i2c_if)
{
    uint32_t rc;
    char err_buf[ERR_BUF_SIZE * 2];
    char sfp_reg[2];
    char new_data[2];
   
    /* Ready to write the data */
    sfp_reg[0] = reg_high;
    sfp_reg[1] = reg_low;
    i2c_if->size = sizeof(sfp_reg);
    i2c_if->buf = &sfp_reg[0];

    /* Open the device */
    if ((rc = n2g_i2c_open(i2c_if)) != PASSED) {
    	cterr('f', 0, "%s() Unable to open. rc = %#x", __FUNCTION__, rc);
    	return(FAILED);
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write GLC-T SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        i2c_if->buf = &new_data[0];
        rc = n2g_i2c_read(i2c_if);
        printf("\nwrite=0x%x 0x%x,read = 0x%x 0x%x\n",sfp_reg[0],sfp_reg[1],new_data[0],new_data[1]);
    } 

    /* Close the device */
    n2g_i2c_close(i2c_if);
    if (rc != PASSED) {
    	cterr('f', 0, err_buf);
    }
    return(rc);
}
/**********************************************************************
 *
 * Function:    set_sfp_glc_t_1000
 *
 * Description:	Configure GLC-t 1000 to support to support external
 *              loopback test. Refer to Avago vendor data sheet 
 *              "Frequently Asked Questins - Question 14".
 *              (GLC-T,GLC-TE,SFP-GE-T)
 * Input:	i2c_bus - I2C bus enum.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int set_sfp_glc_t_1000 (void)
{
    n2g_i2c_if_t i2c_if;
    int repeat = 100;
    char new_data[2];

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_dev = SFP_GLC_BASE; 
    i2c_if.i2c_ctrl = I2C_CTRL_SEVENTEEN;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.size = sizeof(uint8_t);

    /* Clear all interrupts */
    i2c_if.offset = SFP_COPPER_INT_REG;
    if (sfp_i2c_oper(0x0, 0x0, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Master mode */
    i2c_if.offset = SFP_COPPER_MA_SL_CR;
    if (sfp_i2c_oper(0x1B, 0x00, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Apply soft reset and enable auto-negotiation */
    i2c_if.offset = SFP_COPPER_CONTROL;
    if (sfp_i2c_oper(0x91, 0x40, &i2c_if) == FAILED) {
        return (FAILED);
    }
    do {
        msleep(100);
        i2c_if.buf = &new_data[0];
        n2g_i2c_read(&i2c_if);
    } while((repeat-- > 0) && (new_data[0] & 0x80));
    if (repeat < 99) {
        printf("\nrepeat = %d,read = 0x%x 0x%x  \n",(99 - repeat),new_data[0],
                 new_data[1]);
    }

    msleep(SFP_PHY_RESET_DELAY);

    /* Select page 7 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_i2c_oper(0x00, 0x07, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Gigabit mode */
    i2c_if.offset = 0x1E;
    if (sfp_i2c_oper(0x08, 0x08, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 16 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_i2c_oper(0x00, 0x10, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Enable Stub loopback */
    i2c_if.offset = 0x1E;
    if (sfp_i2c_oper(0x00, 0x02, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 18 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_i2c_oper(0x00, 0x12, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Disable Near End Crosstalk (Next) canceller. */
    i2c_if.offset = 0x1E;
    if (sfp_i2c_oper(0x80, 0x01, &i2c_if) == FAILED) {
        return (FAILED);
    }
    return(PASSED);
}
#ifdef ALTER_SFP_COOKIE
/******************************************************************************
 *
 * Function   : at_alter_eeprom
 * Description:	Peek-n-poke AT24C0x byte location.
 * Inputs     : n2g_i2c_dev_t pointer to the selected SFP device
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int at_alter_eeprom (n2g_i2c_dev_t *i2c_sfp)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    uint16_t dev_size = 0;
    register char *c_ptr;
    int tmp;
    /* Bug fix note: 
     * Change val from uint to unsigned long.
     * Change inbuf[3] to inbuf[4] in ensure adequate buffer allocation.
     * Legacy function getnum() (now it is just a
     * wrapper) prototype in proto.h used (unsigned int *) in the 3rd
     * argument, but the underlying getnnum() operates on (unsigned
     * long *). This caused memory corruption in the calling routine's
     * local stack.
     */
    unsigned long val;
    char inbuf[4], done = FALSE;
    char old_data, new_data;
    uint16_t addr;

    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = IOFPGA_I2C;
    new_i2c_if.i2c_dev = SFP_EEPROM_BASE; 
    new_i2c_if.i2c_ctrl = I2C_CTRL_SEVENTEEN;
    new_i2c_if.mux = I2C_MUX_ZERO;
    new_i2c_if.size = sizeof(uint8_t);

    /* Get the device size based on EEPROM type */
    if ((dev_size = get_eeprom_size(ovld_sfp_data_type)) == 0) {
        return (FAILED);
    }

    /* Get the location to peek-n-poke */
    addr = gethex_answer("Enter the starting address:", 0, 0, dev_size);

    printf("Enter the data bytes. x or q to quit\n");

    while((addr <= dev_size) && (done == FALSE)) {
        /* Read the data first. */
        new_i2c_if.buf = &old_data;
        new_i2c_if.offset = addr;

        //rc = api_mb_i2c_read(i2c_sfp, new_i2c_if.offset,
        //                    new_i2c_if.size, (char *)new_i2c_if.buf);
        rc = n2g_i2c_read(&new_i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "%s: read offset %#x rc = %#x",
                          __FUNCTION__, addr, rc);
            return (FAILED);
        } /* endof if rc */

        printf("%#.2x @ %#x ==> ", old_data, addr);

        c_ptr = inbuf;
        get_line((char *)c_ptr, sizeof(inbuf));

        switch (*c_ptr) {
        case 'x': /* exit */
        case 'q': /* quit */
        case 'X': /* exit */
        case 'Q': /* quit */
            done = TRUE;
            break;
        case 0:	/* next location */
            break;
        default:
	  tmp = getnum(c_ptr, 16, (unsigned int*)&val);
            if (tmp == 0) {
                printf("bad value \"%s\"\n", c_ptr);
                continue; /* Same location again */
            } else {
                new_data = (char)val;

                /* Write the new data */
                new_i2c_if.buf = &new_data;

                //rc = api_mb_i2c_write(i2c_sfp, new_i2c_if.offset,
                //                      new_i2c_if.size, (char *)new_i2c_if.buf);
                rc = n2g_i2c_write(&new_i2c_if);
                if (rc != PASSED) {
                    printf("%s: write failed. rc = %#x", __FILE__, rc);
                    return (FAILED);
                } /* endof if rc */
                msleep(AT24C0X_T_WR + 1);
            } /* endof if tmp */
	          break; /* next location */
         } /* endof switch */
         addr++;
    } /* endof while */

    return(PASSED);
}


/******************************************************************************
 *
 * Function   :	at_alter_eeprom_wrap
 * Description:	Alter SFP cookie.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static int at_alter_eeprom_wrap (void)
{
    n2g_i2c_dev_t i2c_sfp;
    uint32_t rc = FAILED;
    /* Alter EEPROM data */
    rc = at_alter_eeprom(&i2c_sfp);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to alter EEPROM data.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (rc);
}
#endif /* ALTER_SFP_COOKIE */


#if 0 /* palin2 0719 masked temporarily */
/**********************************************************************
 *
 * Function:	show_sfp_type
 *
 * Description:	Display the Encoding byte from all SFPs' cookies.
 *
 * Input:	err_log - TRUE to cterr. FALSE to printf.
 *
 * Output:	None
 *
 * Assumption:	If only 1 SFP in the system, it has to start with SFP1, not
 *		SFP2.
 *
 **********************************************************************
 */
void
show_sfp_type(int err_log)
{
    int i, first_sfp, last_sfp;
    uint8_t code;

    first_sfp = GEPHY0_I2C;
    last_sfp = GEPHY1_I2C;

    for (i = first_sfp; i <= last_sfp; i++) {
	if (i == GEPHY0_I2C) {
	    /* PHY0 with SFP1 */
	    if (is_sfp_present(MB_GEMAC_PHY0) == FALSE) {
		/* SFP1 not installed */
		printf("SFP 1 is not installed\n");
		continue;
	    }
	} else {
	    /* PHY1 with SFP2 */
	    if (is_sfp_present(MB_GEMAC_PHY1) == FALSE) {
		/* SFP2 not installed */
		printf("SFP 2 is not installed\n");
		continue;
	    }
	}

	if (sfp_cookie_read(i, SFP_COO_ENC, SFP_COO_ENC_L, &code, err_log) ==
			    PASSED) {
	    switch(code) {
	    case SFP_ENCODE_8B10B:
		printf("SFP %d with 8B10B encoding\n", i - first_sfp + 1);
		break;
	    case SFP_ENCODE_SONET:
		printf("SFP %d with SONET encoding\n", i - first_sfp + 1);
		break;
	    case SFP_ENCODE_4B5B:
		printf("SFP %d with 4B5B encoding\n", i - first_sfp + 1);
		break;
	    default:
		printf("SFP %d unknown encoding of %#x\n", i - first_sfp + 1,
						   code);
		break;
	    } /* endof switch */
	}
    }

}

/**********************************************************************
 *
 * Function:    set_sfp_glc_ge_100fx
 *
 * Description:	Configure GLC-GE-100FX to FX mode. Refer to vendor data sheet.
 *
 * Input:	i2c_bus - I2C bus enum.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int set_sfp_glc_ge_100fx(int i2c_bus)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    uint8_t reg_hi, reg_lo;
    char err_buf[ERR_BUF_SIZE * 2];

    /* Setup the I2C struct */
    /* I2C bus number, and device enum */
    switch(i2c_if.i2c_bus_type = i2c_bus) {
    case GEPHY0_I2C:
	/* SFP 1 */
	i2c_if.i2c_dev = MB_I2C_SFP1_X;
	break;
    case GEPHY1_I2C:
	/* SFP 2 */
	i2c_if.i2c_dev = MB_I2C_SFP2_X;
	break;
    default:
	/* Invalid SFP */
	cterr('f', 0, "%s() Invalid SFP I2C bus %#x", __FUNCTION__, i2c_bus);
	return(FAILED);
    } /* endof switch */

    i2c_if.i2c_speed = N2G_I2C_100KHZ;	/* I2C bus speed */

    /* Open the device */
    if ((rc = n2g_i2c_open(&i2c_if)) != PASSED) {
	cterr('f', 0, "%s() Unable to open. rc = %#x", __FUNCTION__, rc);
	return(FAILED);
    }

    /* Write FX100 Enable register to set FX mode */
    i2c_if.offset = SFP_GE_100FX_REG; /* offset */
    reg_lo = SFP_GE_100FX_REG_FX_L;   /* Set the FX mode */
    reg_hi = SFP_GE_100FX_REG_FX_H;

    /* Ready to write the high byte */
    i2c_if.size = sizeof(reg_hi);
    i2c_if.buf = &reg_hi;

    msleep(SFP_I_INIT_TIME);    /* wait for t_init */

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write FX Enable Register "
                         "High byte in GLC-GE-100FX SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } else {
        /* Ready to write the low byte */
        i2c_if.size = sizeof(reg_lo);
        i2c_if.buf = &reg_lo;

        rc = n2g_i2c_write(&i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s %s() Unable to write FX Enable Register "
                             "Low byte in GLC-GE-100FX SFP.\nrc = %#x",
			     __FILE__, __FUNCTION__, rc);
            rc = FAILED;
        } else {
            /* Write the Edge Control register for the loopback plug */
            i2c_if.offset = SFP_GE_100FX_REG18; /* Offset */
            reg_lo = SFP_GE_100FX_REG_EC_L;     /* New Edge Control */
            reg_hi = SFP_GE_100FX_REG_EC_H;

            /* Ready to write the high byte */
            i2c_if.size = sizeof(reg_hi);
            i2c_if.buf = &reg_hi;

            msleep(SFP_I_INIT_TIME);	/* wait for t_init */

            rc = n2g_i2c_write(&i2c_if);
            if (rc != PASSED) {
#ifdef I2C_DEBUG
        	printf("\ni2c bus %#x, dev %#x\n",
                       i2c_if.i2c_bus_type, i2c_if.i2c_dev);
#endif /* I2C_DEBUG */
        	sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                 "Edge Control Register High bytes.\n"
                                 "rc = %#x", __FUNCTION__, rc);
        	rc = FAILED;
            } else {
        	/* Ready to write the low byte */
        	i2c_if.size = sizeof(reg_lo);
        	i2c_if.buf = &reg_lo;
        	rc = n2g_i2c_write(&i2c_if);
        	if (rc != PASSED) {
        	    sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                     "Edge Control Register Low byte.\n"
                                     "rc = %#x", __FUNCTION__, rc);
         	    rc = FAILED;
                } /* endof if write Edge Control low byte */
            } /* endof if write Edge Control high byte */
	} /* endof if write FX Enable low byte */
    } /* endof if write FX Enable high bytes */

    /* Close the device */
    n2g_i2c_close(&i2c_if);

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
    }

    return(rc);

}
#endif


/*------------------------------------------------------------------
$Log: platform_sfp_cookie.c,v $
Revision 1.13  2019/04/18 18:26:55  ptong
Fix bug in at_alter_eeprom() caused by getnum() change in linux_stub.c

Revision 1.12  2017/08/10 10:12:43  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.11  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.10  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.9  2013/12/11 10:12:40  alpeng
remove usb console i2c test due to rommon issue; 30w poe is not supported on sword and dagger; adding temp sensor i2c test

Revision 1.8  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.7  2013/07/18 17:17:05  mcharon
add -Wal and clean up compile warnings

Revision 1.6  2013/07/02 10:57:37  hroni
fix sfp cookie utility

Revision 1.5  2013/06/28 06:53:25  hroni
fix sfp mux utility

Revision 1.4  2013/06/19 09:45:40  hroni
add utilities for I/O side and Bezel side temperature sensors

Revision 1.3  2013/06/14 09:51:06  hroni
1. support to PSU mux
2. temporarily uses #ifdef MUX124 to turn off/on PCA9545 mux support

Revision 1.2  2013/06/13 10:53:03  hroni
add mux_id in set_mux_channel()
uses set_i2c_if_struct() to set i2c_if structure

Revision 1.1  2013/06/13 08:34:56  hroni
add support for SFP mux

Revision 1.5  2012/06/20 07:26:08  alpeng
including i2c scan test for SFP

Revision 1.4  2012/06/06 15:00:37  palin2
Clean up compiler warnings.

Revision 1.3  2012/06/06 09:57:29  iachang
Clean up complier warnings.

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/

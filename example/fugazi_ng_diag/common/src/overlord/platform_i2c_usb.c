/* $Id: platform_i2c_usb.c,v 1.4 2013/10/25 03:36:58 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_i2c_usb.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_i2c_usb.c
 *
 * Description: Informers USB Console I2C device.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include "endians.h"
#include "common.h"
#include "platform_i2c_usb.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "i2c_api.h"
#include "byteswap.h"



/*#define USB_REG_READ  * */

#include "i2c_address.h"

/* Function prototypes */
extern uint32 err_report (dev_object_t *dev, char *err_msg,
			  uint32 err_type); /* in hwic_spidey_ct3.c */
static void dev_obj_create(dev_cy7c64215_object_t *, n2g_i2c_if_t *i2c_if);
static int init_usb(void);
#ifdef USB_REG_READ
static int usb_reg_read(int);
#endif /* USB_REG_READ */
static int usb_iofpga_util(int);
static void show_usb_iofpga_stat(void);
static void en_usb_con_loopback();
static void dis_usb_con_loopback();
static int write_console();
static void *read_console(void *);

uint32_t usb_i2c_read(n2g_i2c_if_t *);
uint32_t usb_i2c_write(n2g_i2c_if_t *);

/* USB console */
int usb_con_int_loopback(void);
int usb_con_ext_loopback(void);
static int usb_con_loopback(int);
static int usb_con_reset();

#define TEST_PATTERN "12345678"
#define TEST_PATTERN2 12345612

#define USB_CON_LPBK_ON  1
#define USB_CON_LPBK_OFF 0
#define SKIP_DISPLAY     3

/* Using cable conntect to test usb console instead of 
 * putting strings in Linux /dev/console, which is not reliable.
 * When the cable is dis/connected, Linux will exercise usb protocol, 
 * and user will see the msg.
 */
#define ENABLE_USB_CONSOLE_TEST 0


/* Global variables */
#include "platform_usb_fw.h"	/* Firmware supplied by Cypress */

/*
 * USB Console I2C Menu
 */
static submenu_xtable_t usb_menu_table[] = {
    {"Download Firmware",                  (PFT)init_usb,             0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Firmware Version",                   (PFT)show_usb_ver,         USB_REV_REG,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Show USB Console Status",            (PFT)usb_iofpga_util,      0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Read from Console",                  (PFT)read_console,         0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Write to Console",                   (PFT)write_console,        0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
#if ENABLE_USB_CONSOLE_TEST
    {"Enable USB Loopback",                (PFT)en_usb_con_loopback,  0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"Disable USB Loopback",               (PFT)dis_usb_con_loopback, 0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"USB Console Internal Loopback Test", (PFT)usb_con_int_loopback, 0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
    {"USB Console External Loopback Test", (PFT)usb_con_ext_loopback, 0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
#endif
};

#define USB_MENU_TABLE_SIZE (sizeof(usb_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t usb_menu_primary_items[USB_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t usb_menu_secondary_items[USB_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo usbdiag = {
    "USB Console I2C Utility Menu", /* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    usb_menu_primary_items,
};

static struct menuinfo *usbdiagp = &usbdiag;


/*******************************************************************************
 *
 * Function   : get_usb_i2c_fw_dl_struct
 * Description: To get USB I2C interface structure for FW dnld.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
get_usb_i2c_fw_dl_struct (n2g_i2c_if_t *i2c_if)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_USB_CONSOLE_FW_DL);

    if (tmp == NULL) {
        printf("%s: Failed to get USB I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : get_usb_i2c_struct
 * Description: To get USB I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
get_usb_i2c_struct (n2g_i2c_if_t *i2c_if)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_USB_CONSOLE);

    if (tmp == NULL) {
        printf("%s: Failed to get USB I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}

/**********************************************************************
 *
 * function:	build_i2c_usb_menu
 *
 * Description:	Build USB Console I2C menu.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void
build_i2c_usb_menu (void)
{
    testname("USB Console I2C");

    build_primary_submenu(usb_menu_table, USB_MENU_TABLE_SIZE,
			  "USB Console I2C Utility Menu", &usbdiagp);
    build_secondary_submenu(usb_menu_table, USB_MENU_TABLE_SIZE,
			    usb_menu_secondary_items);
    menu(&usbdiag, usb_menu_secondary_items, 0);
}

/*********************************************************************
 *
 * Function:	dev_obj_create
 *
 * Description:	Create USB Console I2C device object for
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
dev_obj_create (dev_cy7c64215_object_t *pe, n2g_i2c_if_t *i2c_if)
{
    dev_object_t *dev = (dev_object_t *)pe;

    /* Setup device struct */

    /* Create common device object */
    dev_cy7c64215_create(dev, (dev_error_report_t) err_report);

    /* Setup call-out function vectors */
    pe->callout_fvt->open = n2g_i2c_open;
    pe->callout_fvt->close = n2g_i2c_close;
    pe->callout_fvt->rd = usb_i2c_read;
    pe->callout_fvt->wr = usb_i2c_write;

    /* Setup other struct fields */
    pe->i2c_p = i2c_if;
    i2c_if->size = sizeof(usb_t);	/* Buffer size */
}

/**********************************************************************
 *
 * Function:	init_usb
 *
 * Description:	Initilize USB Console I2C content.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
init_usb(void)
{
    dev_cy7c64215_object_t usb;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&usb;
    int rc;

    rc = get_usb_i2c_fw_dl_struct(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "get_usb_i2c_fw_dl_struct() failed");
	return(FAILED);
    }

    /* Create the device object */
    dev_obj_create(&usb, &i2c_if);

    /* Attach the device object */
    rc = usb.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
	usb.base.dev_object_fvt->dev_destroy(&dev);
	cterr('f', 0, "init_usb() Device attach failed");
	return(FAILED);
    }

    usb.init_p = &usb_console[0];

    /* Call the common device driver to initialize the CY7C64215 */
    rc = usb.base.dev_object_fvt->dev_init(dev);

    /* V1.27 needs delay after downloading. Without the delay, the Informers
     * 1:4 Mux will timeout.
     */
    msleep(CY7C64215_RESET_DELAY);

    /* Calls common device driver destroy instead of detach to free memory */
    usb.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
	cterr('f', 0, "init_usb() write failed");
    }

    return(rc);

}

/*******************************************************************************
 *
 * Function   : read_console
 * Description: read TEST_PATTERN2 (string) from /dev/console
 * Inputs     : ref_pattern - TEST_PATTERN2
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
void *
read_console (void *ref_pattern)
{

    FILE *fd_aux; 
    uint32_t getdata; 

    fd_aux = fopen("/dev/console", "r");
    if (fd_aux == NULL) {
        cterr('f',0,"open /dev/console failed. \n");
        return (void *)FAILED; 
    }
    
     fscanf(fd_aux, "%d", &getdata); 
     
     fclose(fd_aux);
     
     if (getdata != (* (int *)ref_pattern))
         return (void *)FAILED; 
     else 
         return (void *)PASSED; 
}


/*******************************************************************************
 *
 * Function   : write_console
 * Description: Write TEST_PATTERN2 (string) into /dev/console
 * Inputs     : NONE.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
write_console (void)
{

    FILE *fd_aux; 
    uint32_t getdata; 
  
    fd_aux = fopen("/dev/console", "w");
    if (fd_aux == NULL) {
        cterr('f',0,"open /dev/console failed. \n");
        return FAILED; 
    }
    getdata = TEST_PATTERN2;
    
    fprintf(fd_aux, "%d", getdata); 
          
    fclose(fd_aux);	
    
    return PASSED; 
}

/*******************************************************************************
 *
 * Function   : usb_con_ext_loopback
 * Description: To detect usb console cable for ext loopback test.
 * Inputs     : NONE.
 * Outputs    : PASSED/FAILED
 * NOTE       : Need to connect usb console cable first
 *
 *******************************************************************************
 */
int
usb_con_ext_loopback (void)
{
    uint32_t reg, retval;

    reg = get_platform_uart_mux_ctrl_reg();

    testname("USB console external loopback");
    
    if (reg & MUX_REG_USB_CONSOLE_CABLE_DET) {
        prpass(testpass, "pass");
        retval =  PASSED;
    } else {
	cterr('f', 0, "cannot detect USB console");
        retval =  FAILED;
    } 
   
    prcomplete(testpass, errcount, (char *)0);
    return retval;
}


/*******************************************************************************
 *
 * Function   : usb_con_int_loopback
 * Description: w/r /dev/console for console internal loopback test. 
 * Inputs     : NONE.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
usb_con_int_loopback (void)
{
    pthread_t threads;
    int pthr_rv = 0;
    char command[100];
    int test_pattern;
    uint ver;
    
    testname("USB console internal loopback");
    prpass(testpass, "\n%s\n",__FUNCTION__);
    fflush(stdout);
 
    /* Exit if FW is old */
    ver = show_usb_ver(SKIP_DISPLAY);
    if ((ver >> 8) < 2) {
	cterr('f', 0, "USB Console Firmware V%d.%02d. Needs V2.2 or later",
		ver >> 8, ver & 0xFF);
	return FAILED;
    }

    test_pattern = TEST_PATTERN2;
    
    sprintf(command, "echo \"%d\" > /dev/console ", test_pattern);

    if (pthread_create(&threads, NULL, read_console, (void *)&test_pattern)) {
        printf("pthread_create failed \n");
        return FAILED;
    }

    msleep(10); /*wait for thread create */
	
    en_usb_con_loopback();
    system(command);

    pthread_join(threads, (void **)&pthr_rv);

    /* close loopback immediately, to prevent the system noise from system. */
    dis_usb_con_loopback();
    
    msleep(10);  /*this delay is needed, or test getting failed.*/

    fflush(stdout);
        
       
    if (pthr_rv != PASSED) {
        cterr('f', 0, "usb_con_int_loopback() rx compare failed");
        prcomplete(testpass, errcount, (char *)0);
        return FAILED;
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;	

}

#ifdef USB_REG_READ
/**********************************************************************
 *
 * Function:	usb_reg_read
 *
 * Description:	Read USB Console register.
 *
 * Inputs:	index - Register offset.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
usb_reg_read(int index)
{
    dev_cy7c64215_object_t usb;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&usb;
    uint16_t reg;
    int rc;

    /* Create the device object */
    dev_obj_create(&usb, &i2c_if);
    i2c_if.i2c_dev = MB_I2C_USB_R;

    /* Attach the device object */
    rc = usb.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
	usb.base.dev_object_fvt->dev_destroy(&dev);
	cterr('f', 0, "usb_reg_read() Device attach failed");
	return(FAILED);
    }

    /* Read the register */
    i2c_if.offset = index;
    i2c_if.size = sizeof(reg);
    i2c_if.buf = (char *)&reg;

    rc = n2g_i2c_read(&i2c_if);

    /* Calls common device driver destroy instead of detach to free memory */
    usb.base.dev_object_fvt->dev_destroy(&dev);

    if (rc == PASSED) {
	printf("\nRegister @ %#x = %#x\n", index, reg);
    } else {
	cterr('f', 0, "%s: I2C read USB register %#x failed. rc = %#x",
                      __FUNCTION__, index, rc);
    }

    return(rc);

}
#endif /* USB_REG_READ */


/**********************************************************************
 *
 * Function:    usb_i2c_write
 *
 * Description: write usb console address on FPGA via i2c
 *
 * Inputs:      i2c_if - i2c structure.
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
uint32_t usb_i2c_write(n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"usb_i2c_write: buf is null");
    }
    
    rc = n2g_i2c_write(i2c_if);
    
    if (rc != RC_I2C_OP_OK) {
	msleep(CY7C64215_CMD_DELAY);
	//msleep(100);	/* Env MCU I2C cycle time */
        return (FAILED);
    }

    msleep(CY7C64215_CMD_DELAY);

    return(PASSED);
}


/**********************************************************************
 *
 * Function:    usb_i2c_read
 *
 * Description: read usb console address on FPGA via i2c
 *
 * Inputs:      i2c_if - i2c structure.
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
uint32_t usb_i2c_read(n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"USB I2C read: buf is null");
    }
    
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	/* Unable to read data */
	cterr('f', 0, "%s Unable to read. rc = 0x%08x", __FUNCTION__, rc);
	return(FAILED);
    }

    msleep(CY7C64215_CMD_DELAY);

    return(PASSED);
}

/**********************************************************************
 *
 * Function:	show_usb_ver
 *
 * Description:	Display USB Console firmware version.
 *
 * Inputs:	format - Display format as display_format_t in common.h
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int
show_usb_ver (int format)  
{
    n2g_i2c_if_t i2c_if;
    cy7c64215_reg_t reg;
    int rc = FAILED;

    rc = get_usb_i2c_struct(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "get_usb_i2c_struct() failed");
	return(FAILED);
    }

    i2c_if.offset = USB_REV_REG;
    i2c_if.size = sizeof(reg);
    i2c_if.buf = (char *)&reg;

    rc = usb_i2c_read(&i2c_if);

    if (rc == PASSED) {
	if (format == DISPLAY_M2M) {
	    /* Manufacturing display format */
	    printf("USBCNSLVER:%d.%02d\n",
		   reg.fw_version_major, reg.fw_version_minor);
	} else {
	    if (format == SKIP_DISPLAY) {
		return(((reg.fw_version_major & 0xFF) << 8) |
			(reg.fw_version_minor & 0xFF));
	    } else {
		/* Regular Diagmon display format */
		printf("\n USB Console Firmware Version - %d.%02d\n",
			reg.fw_version_major, reg.fw_version_minor);
	    }
#ifdef USB_CONSOLE_DEBUG /* */
	    printf("%#x %#x %#x %#x %#x %#x %#x %#x %#x %#x\n",
		   reg.reg[0], reg.reg[1], reg.reg[2], reg.reg[3], reg.reg[4],
		   reg.reg[5], reg.reg[6], reg.reg[7], reg.reg[8], reg.reg[9]);
#endif /* USB_CONSOLE_DEBUG */
	}
    } else {
	cterr('f', 0, "%s: I2C read USB version failed. rc = %#x",
                      __FUNCTION__, rc);
    }

    
    return(rc);
}

/**********************************************************************
 *
 * Function:	usb_iofpga_util
 *
 * Description:	USB Console IOFPGA utilities
 *
 * Inputs:	menu - Not used.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Note: need usb cable to do it. 
 **********************************************************************
 */
static int
usb_iofpga_util (int menu)  
{
    testname("USB Console IOFPGA");
    printf("\nThis test requires both the term server screen for RJ45, and "
	   "the Hyperterminal for USB console to be ready\n");

    show_usb_iofpga_stat();

    return(PASSED);
}

/**********************************************************************
 *
 * Function:	show_usb_iofpga_stat
 *
 * Description:	Display USB Console IOFPGA register status
 *
 * Inputs:	None.
 *
 * Outputs:	None.
 *
 **********************************************************************
 */
static void
show_usb_iofpga_stat (void)
{
    uint32_t reg;

    reg = get_platform_uart_mux_ctrl_reg();

    printf("\nConsole RJ-45/USB Multiplexer Register reg = 0x%x:\n", reg);

    /* bit 13 */
    printf("	USB Console GPIO - %s\n", (reg & MUX_REG_USB_CONSOLE_GPIO_VAL) ?  
	   "Asserted" : "De-asserted");

    /* bit 9 */
    printf("	USB Console Cable - %s\n", (reg & MUX_REG_USB_CONSOLE_CABLE_DET) ? 
	   "Detected" : "Not installed");

    /* bit 2 */
    printf("	USB Manual Mux Select - %s\n", (reg & MUX_REG_USB_MANUAL_MUX_SEL) ?  
	   "Manual" : "Auto");

    /* bit 1 */
    printf("	USB Mux Select - %s\n", (reg & MUX_REG_USB_MUX_SEL) ?  
	   "USB Console" : "RJ45 Serial Port");
}


/**********************************************************************
 *
 * Function:    en_usb_con_loopback
 *
 * Description: enable usb console and turn on usb looback.
 *
 * Inputs:
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
void
en_usb_con_loopback (void)
{
    switch_console_usb(ENABLE); /* on dash_fpga.c */
    usb_con_loopback(USB_CON_LPBK_ON); /* write to i2c */
}


/**********************************************************************
 *
 * Function:    dis_usb_con_loopback
 *
 * Description: disable usb console and turn off looback on usb.
 *
 * Inputs:
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
void
dis_usb_con_loopback (void){
	
    usb_con_loopback(USB_CON_LPBK_OFF); /* write to i2c */ 
    switch_console_usb(DISABLE); /* on dash_fpga.c */
    usb_con_reset(); 
}


/**********************************************************************
 *
 * Function:	usb_con_loopback
 *
 * Description:	looback usb console
 *
 * Inputs:      toggle - turn on/off usb loopback
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
usb_con_loopback (int toggle)  
{
    n2g_i2c_if_t i2c_if;
    uint32_t reg;
    int rc = FAILED;

    rc = get_usb_i2c_struct(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "get_usb_i2c_struct() failed");
	return(FAILED);
    }

    /* Write */
    if(toggle == 1) {
	reg = 0x20;
    } else {
	reg = 0x21;
    }
    /* Only offset is written. */
    i2c_if.size = 0;
    i2c_if.buf = (char *)&reg;
    i2c_if.offset = reg;

    rc = usb_i2c_write(&i2c_if);

    return(rc);
}


/**********************************************************************
 *
 * Function:	usb_con_reset
 *
 * Description:	restore from loopback mode
 *
 * Inputs:
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
usb_con_reset (void)
{
    n2g_i2c_if_t i2c_if;
    uint8_t reg = 0x0;
    int rc = FAILED;

    rc = get_usb_i2c_struct(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "get_usb_i2c_struct() failed");
	return(FAILED);
    }

    /* Write. Reset USB Console firmware memory pointer. */
    i2c_if.offset = 0x0;
    i2c_if.size = 0;
    i2c_if.buf = (char *)&reg;

    rc = usb_i2c_write(&i2c_if);
    
    return(rc);
}

/*------------------------------------------------------------------
$Log: platform_i2c_usb.c,v $
Revision 1.4  2013/10/25 03:36:58  alpeng
using cable to test usb console instead of putting strings into /dev/console

Revision 1.3  2013/06/28 10:53:53  hroni
fix usb console mux number

Revision 1.2  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.9  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.8  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.7  2012/08/22 20:03:44  mcharon
remove extra menu items not used

Revision 1.6  2012/08/22 19:31:06  mcharon
remove cs_key_buffer.h

Revision 1.5  2012/07/25 19:32:14  mcharon
 move aux loopback test to mb_tst.c

Revision 1.4  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.3  2012/05/05 04:49:55  mcharon
change long long to just long

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/

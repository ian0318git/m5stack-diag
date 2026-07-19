/* $Id: patriot_util.c,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_util.c
 *
 * Description: Patriot Utility
 *
 *
 * Author: Sofian Teja
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "defs.h"
#include "patriot_main.h"
#include "common_utils.h"
#include "ds3170.h"
#include "p1021_immap.h"
#include "p1021_espi.h"
#include "patriot_intr.h"
#include "router_if.h"


/***********************************************************************
 *  Global Variable
 ************************************************************************/
int patriot_framer_debug = 0 ;
extern fe_packet_t *tx_packet_p;
extern unsigned char patriot_fpga_prom[];
extern unsigned long int patriot_fpga_prom_size;
extern const unsigned int patriot_fpga_prom_ver;
extern unsigned char patriot_fpga_hmac[];
extern unsigned long int patriot_fpga_hmac_size;
extern uchar err_msg[];
/***********************************************************************
 *  Functions
 ************************************************************************/
int patriot_fpga_reset(void);
extern unsigned char param_arr[6];
static signed int fpga_fd = -1;

/**********************************************************************
 *
 * Function: patriot_framer_debug_onoff
 *
 * This function for open close the debug flag.
 *
 * Input : debug_onoff
 *
 * Output: none
 *
 **********************************************************************
 */
int patriot_framer_debug_onoff(uint debug_onoff)
{
    if (patriot_framer_debug) {
        printf("Patriot Framer: %s: %s %d\n",
            __FUNCTION__,
            "debug option:",
            debug_onoff);
    }
    if (debug_onoff) {
        patriot_framer_debug = 1;
    } else {
        patriot_framer_debug = 0;
    }
    return 0;
}

/**********************************************************************
 *
 * Function: patriot_conf_ds3170_frmr
 *
 * This function configures the FPGA and the T3/E3 framer chip for the
 * specified mode.
 *
 * Input : lpmode    - loopback mode
 *         intf_mode - T3 / E3 mode
 *         lpbk_mode - FPGA config lpbk mode
 *         subrate   - use / bypass subrate
 *         bypassfpga - skip fpga
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_conf_ds3170_frmr(uchar lpmode, uchar intf_mode, uchar lpbk_mode,
		uchar subrate, uchar bypassfpga)
{
    uchar buf = 0;
    uchar temp = 0;
    /* Assume platform_cpu_i2c_init () is init done at the beginning */
    printf("\npatriot_conf_ds3170_frmr\n");

    /* Reset Framer */
    patriot_ds3170_reset();

    /* Reset FPGA */
    if (!bypassfpga) {
        platform_cpu_i2c_init();
        patriot_fpga_reset();
        REGB->im_gur.pmuxcr &= ~0x60000000;
    }

    /* Debug flag turn off */
    patriot_framer_debug_onoff(0);

    /* Init DS3170 */
    if (ds3170_init_clear_te3(intf_mode)) {
        sprintf(err_msg, "\n%s, [#%d]:ds3170_init_clear_te3(), failed\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    msleep(100);
    /*
     * port_type_sel offset is the same for clear TE3
     */
    if (!bypassfpga) {
    if (intf_mode == MODE_T3) {
        /* Config FPGA for interface mode test T3 */
        if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:config fpga read mode T3 fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
        }

        buf &= (~PORT_SEL_TYPE_E3);
        if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:config fpga write mode T3 fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
    } else {
        /* Config FPGA for interface mode test E3 */
        if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:config fpga read mode E3 fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
        }

        buf |= PORT_SEL_TYPE_E3;
        if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:config fpga write mode E3 fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
    }


    if (patriot_framer_debug) {
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config fpga port type select register fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x", __FUNCTION__,
	       __LINE__, buf);
    }


    /* Config FPGA reg for clock mode select */
    buf &= ~PORT_CLK_SLAVE;
    if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:config fpga write reg for enable"
        		" the CLK Mode fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    msleep(20);

    if (patriot_framer_debug) {
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config fpga port type select register fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x", __FUNCTION__,
	       __LINE__, buf);
    }


    /*****************SET FPGA LOOPBACK***************/
    if (lpbk_mode == FPGA_LPBK) {
	buf = 0;
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga read mode T3 fail\n"
	    		, __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	
	buf |= PORT_FPGA_LPBK; /* bit 5 */
	
	if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga write reg for enable"
	    		" the OSC fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);

	if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
		printf("\npatriot_conf_ds3170_frmr(), "
		       "config fpga read mode T3 fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
	}

    } else {
	buf = 0;
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga read mode T3 fail\n"
	    		, __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	
	buf &= ~PORT_FPGA_LPBK; /* bit 5 */
	
	if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
	    sprintf(err_msg,"\n%s, [#%d]:config fpga write reg for "
	    		"enable the OSC fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);

	if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
		printf("\npatriot_conf_ds3170_frmr(), "
		       "config fpga read mode T3 fail\n");
		return (FAILED);
	    }
	    printf("\n AFTER SETTING FPGA LOOPBACK \n");
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
	}

    }
    /***************END OF SET FPGA LOOPBACK***************/
    /*****************SET FPGA SUBRATE ***************/
    if (subrate == USE_SUB) {
    	/* Set bit 3 to enable FPGA start of frame output enable.
           Set bit 6 of Port Type Selection Reg for enable subrate.
        */
    	buf = 0;
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga read mode subrate "
	    		"mode fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	
	buf |= PORT_TX_SYNC_EN;  /* bit 3 */
	buf &= ~PORT_SUBRATE_BYPASS; /* bit 6 */
	
	if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga write reg for "
	    		"enable the subrate fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);

	if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
		printf("\npatriot_conf_ds3170_frmr(), "
		       "config fpga read mode subrate mode fail\n");
		return (FAILED);
	    }
	    
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
	}

    } else {
    	/* Set bit 3 to disable FPGA start of frame output enable.
    	 * Set bit 6 of Port Type Selection Reg for disable subrate */
    	buf = 0;
	if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga read mode subrate "
	    		"mode fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	
	buf &= ~PORT_TX_SYNC_EN;  /* bit 3 */
	buf |= PORT_SUBRATE_BYPASS; /* bit 6 */
	
	if (p1021_i2c_write_fpga_byte (PORT_TYPE_SEL_REG, buf)) {
	    sprintf(err_msg, "\n%s, [#%d]:config fpga write reg for "
	    		"disable the subrate fail\n", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);

	if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (PORT_TYPE_SEL_REG, &buf)) {
		printf("\npatriot_conf_ds3170_frmr(), "
		       "config fpga read mode subrate mode fail\n");
		return (FAILED);
	    }
	    
	    printf("\n%s, [#%d]:PORT_TYPE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
	}

    }
    /*****************END OF SET FPGA SUBRATE ***************/
    /* Using SPI directly control the Framer
     * enable / disable loopback mode
     */
    } /* end of bypassfpga */
    if (ds3170_read(&temp, CR4_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer read loopback mode"
        		" reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    temp |= lpmode;

    if (ds3170_write(temp, CR4_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer write loopback mode"
        		" reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }


    if (patriot_framer_debug) {
	if (ds3170_read(&temp, CR4_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read loopback mode reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:CR4_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
    }

    
    /* Enable Tx the B3ZS / HDB3 Encoder performs zero suppression and AMI
     * encoding.
     */
    if (ds3170_read(&temp, LINE_TCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer read LINE_TCR_ADDR_L "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    temp &= ~LINE_TCR_TZSD;
    if (ds3170_write(temp, LINE_TCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer write LINE_TCR_ADDR_L "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    if (patriot_framer_debug) {
	if (ds3170_read(&temp, LINE_TCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read LINE_TCR_ADDR_L reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:LINE_TCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
    }
    

    /* Enable Rx the B3ZS / HDB3 Encoder performs zero suppression and AMI
     * encoding.
     */
    if (ds3170_read(&temp, LINE_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer read LINE_RCR_ADDR_L "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    temp &= ~LINE_RCR_RZSD;
    if (ds3170_write(temp, LINE_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer write LINE_RCR_ADDR_L "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }


    if (patriot_framer_debug) {
	if (ds3170_read(&temp, LINE_RCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read LINE_RCR_ADDR_L reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:LINE_RCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
    }


    /* enable port interface mode LIU ON , JA TX.
     * Refer to DS3170 Specs. Table 10-26 Page.136 */
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer read CR2_ADDR_H "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    temp |= CR2_LM(0x2);

    if (ds3170_write(temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:config framer write CR2_ADDR_H "
        		"reg fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    msleep(20);

    if (patriot_framer_debug) {
	if (ds3170_read(&temp, CR2_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read CR2_ADDR_H reg fail\n");
	    return (FAILED);
	}
	printf("\n%s,[#%d]:CR2_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
    }


    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_t3_set_subrate
 *           This function configures the FPGA to the specified subrate
 *           and bandwidth if applicable. If an invalid subrate is
 *           specified, it configures the FPGA to Clear T3 mode.
 *           If the mode is different from the FPGA's current mode,
 *           the FPGA's DSU block will be reset.
 *
 * Input : sr_type  - subrate mode
 *         sr_val   - bandwidth
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_t3_set_subrate(uchar sr_type, ulong sr_val)
{
    uchar temp = 0;
    uchar buf = 0;

    /* DEBUG Flag ON */
    patriot_framer_debug_onoff(0);

    printf("\npatriot_t3_set_subrate");
    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
        sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n", __FUNCTION__,
        		__LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    temp = buf;

    if (patriot_framer_debug) {
	printf("\n%s, [#%d]:T3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
	       __FUNCTION__, __LINE__, buf);
    }


    /* Config FPGA by select subrate mode */
    switch (sr_type) {
    case DIGITAL_LINK:
    case LARSCOM:
    case VERILINK:
        buf = (sr_val & 0xff);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n",
            		__FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_1 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
        }

        break;
    case KENTROX:
        buf = (sr_val & 0xff);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf = ((sr_val & 0xff00) >> 8);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_2, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf = ((sr_val & 0xff0000) >> 16);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_3, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_1 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);

	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_2, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_2 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
	    
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_3, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_3 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
        }

        break;
    case ADTRAN:
        buf = (sr_val & 0xff);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf = ((sr_val & 0x0300) >> 8);
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_BW_SEL_REG_2, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_1, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_1 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
	    
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_BW_SEL_REG_2, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]T3_SUBRATE_BW_SEL_REG_2 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
        }

        break;
    default:
        sr_type = CLEAR;
    }

    /* Subrate type reset */
    buf = sr_type | DSU_RESET;
    if (p1021_i2c_write_fpga_byte (T3_SUBRATE_MODE_SEL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }


    if (patriot_framer_debug) {
	if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
	    printf("\npatriot_t3_set_subrate(), read fpga fail\n");
	    return (FAILED);
	}
	
	printf("\n%s, [#%d]:T3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
	       __FUNCTION__, __LINE__, buf);
    }

    if ((sr_type & SUBRATE_MASK) != (temp & SUBRATE_MASK)) {
        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf &= ~DSU_RESET;
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        msleep(1);
        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf |= DSU_RESET;
        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
		printf("\npatriot_t3_set_subrate(), read fpga fail\n");
		return (FAILED);
	    }
	    
	    printf("\n%s, [#%d]:T3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
		   __FUNCTION__, __LINE__, buf);
        }

    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_e3_set_subrate
 *           This function configures the FPGA to the specified subrate
 *           and bandwidth if applicable. If an invalid subrate is
 *           specified, it configures the FPGA to Clear E3 mode.
 *           If the mode is different from the FPGA's current mode,
 *           the FPGA's DSU block will be reset.
 *
 * Input : sr_type  - subrate mode
 *         sr_val   - bandwidth
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_e3_set_subrate(uchar sr_type, ulong sr_val)
{
    uchar temp = 0;
    uchar buf = 0;

    /* DEBUG Flag ON */
    patriot_framer_debug_onoff(0);

    printf("\npatriot_e3_set_subrate");
    if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
        sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n", __FUNCTION__,
        		__LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    temp = buf;

    if (patriot_framer_debug) {
	printf("\n%s, [#%d]:E3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
	       __FUNCTION__, __LINE__, buf);
    }

    /* Config FPGA by select subrate mode */
    switch (sr_type) {
    case DIGITAL_LINK:
        buf =(sr_val & 0xff);
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_BW_SEL_REG_1, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (E3_SUBRATE_BW_SEL_REG_1, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]E3_SUBRATE_BW_SEL_REG_1 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
        }

        break;
    case KENTROX:
        buf =(sr_val & 0xff);
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_BW_SEL_REG_1, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf =((sr_val & 0xff00) >> 8);
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_BW_SEL_REG_2, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf =((sr_val & 0xff0000) >> 16);
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_BW_SEL_REG_3, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (patriot_framer_debug) {
	    if (p1021_i2c_read_fpga_byte (E3_SUBRATE_BW_SEL_REG_1, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]E3_SUBRATE_BW_SEL_REG_1 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);

	    if (p1021_i2c_read_fpga_byte (E3_SUBRATE_BW_SEL_REG_2, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]E3_SUBRATE_BW_SEL_REG_2 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);

	    if (p1021_i2c_read_fpga_byte (E3_SUBRATE_BW_SEL_REG_3, &buf)) {
		printf("\npatriot_t3_set_subrate(), write fpga fail\n");
		return (FAILED);
	    }
	    printf("\n%s, [#%d]:[%d]E3_SUBRATE_BW_SEL_REG_3 contents = 0x%02x",
		   __FUNCTION__, __LINE__, sr_type,  buf);
        }

        break;
    case UNFRM_E3:
        break;
    default:
        sr_type = CLEAR;
    }
    /* Subrate type Reset */
    buf = sr_type | DSU_RESET;
    if (p1021_i2c_write_fpga_byte (E3_SUBRATE_MODE_SEL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    if (patriot_framer_debug) {
	printf("\n%s, [#%d]:E3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
	       __FUNCTION__, __LINE__, buf);
    }

    if ((sr_type & SUBRATE_MASK) != (temp & SUBRATE_MASK)) {
        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf &= ~DSU_RESET;
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\nwrite fpga fail\n", __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        msleep(1);
        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:read fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        buf |= DSU_RESET;
        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:write fpga fail\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
    }

    if (patriot_framer_debug) {
	printf("\n%s, [#%d]:E3_SUBRATE_MODE_SEL_REG contents = 0x%02x",
	       __FUNCTION__, __LINE__, buf);
    }

    return (PASSED);
}



/**********************************************************************
 *
 * Function: patriot_fpga_reset
 *
 * This function reset FPGA
 *
 * Input : None
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int patriot_fpga_reset(void)
{
    /* Configure pin direction and  function */
    /* PB12 as output */
    printf("\npatriot_fpga_reset\n");
    /* clear direction bits for PB12 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(12));
    /* PB12 as output */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_OUT(12);
    /* clear function bits for PB12 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(12, 0x3));

    /* Reset - Active Low */
    REGB->im_gur.cpddatb &= ~0x00080000;
    msleep(10);
    REGB->im_gur.cpddatb |= 0x00080000;
    msleep(10);
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_ds3170_reset
 *
 * This function reset Maxim DS3170 chip
 *
 * Input : None
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int patriot_ds3170_reset(void)
{
    
    printf("\npatriot_ds3170_reset\n");
    /* Configure pin direction and  function */
    /* PB13 as output */
    /* clear direction bits for PB12 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(13));
    /* PB12 as output */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_OUT(13);
    /* clear function bits for PB13 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(13, 0x3));

    /* Reset - Active Low */
    REGB->im_gur.cpddatb &= ~0x00040000;
    msleep(100);
    REGB->im_gur.cpddatb |= 0x00040000;
    msleep(100);

    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_display_led
 *
 * This function displays the led according to the Framer's status.
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_display_led(void)
{
    uchar buf = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4);

    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
	sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n", __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_LED_DISPLAY_FAIL);
    }

    buf &= ~BIT0;
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf |= BIT0;

    buf &= ~BIT1;
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf |= BIT1;

    buf &= ~BIT2;
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf |= BIT2;

    buf &= ~BIT3;
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf |= BIT3;

    buf &= ~BIT4;
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf |= BIT4;

    buf &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
    
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    msleep(1000);

    buf = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
   
    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_LED_DISPLAY_FAIL);
    }

    return (TO_HOST_LED_DISPLAY_OK);
}


/**********************************************************************
 *
 * Function: patriot_fpga_register_read
 *
 * This function provide for fpga utilites to read fpga register
 *
 * Input : None
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_fpga_register_read(void)
{
    ushort offset = 0;
    uchar temp = 0;
    uchar buf = 0;

    printf("\npatriot_fpga_register_read");
    offset = gethex_answer("\nRegister Offset: ", 0, 0, 0xffff);

    if (p1021_i2c_read_fpga_byte (offset, &buf)) {
        printf("\n Read fail offset @%#x = %#x\n", offset, buf);
        return (FAILED);
    }
    temp = buf;
    printf("\nFPGA Register @%#x = %#x\n", offset, temp);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_fpga_register_write
 *
 * This function provide for fpga utilites to write fpga register
 *
 * Input : None
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int  patriot_fpga_register_write(void)
{
    ushort offset = 0;
    uchar temp = 0;
    uchar buf = 0;

    printf("\npatriot_fpga_register_write");
    offset = gethex_answer("\nRegister Offset: ", 0, 0, 0xffff);
    temp = gethex_answer("Write Register Value: ", temp, 0, 0xff);
    buf = temp;
    if (p1021_i2c_write_fpga_byte (offset, buf)) {
        printf("\n Write fail offset @%#x = %#x\n", offset, buf);
        return (FAILED);
    }

    buf = 0;
    temp = 0;
    /* Read back the value */
    if (p1021_i2c_read_fpga_byte (offset, &buf)) {
        printf("\n Read fail offset @%#x = %#x\n", offset, buf);
        return (FAILED);
    }

    temp = buf;
    printf("\nRead FPGA Register @%#x = %#x\n", offset, temp);
}


/**********************************************************************
 *
 * Function: patriot_fpga_led_test
 *
 * This function provide for fpga utilites to test FPGA LED
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_fpga_led_test(void)
{
    ushort offset = 0, i;
    uchar temp = 0;
    uchar buf = 0xFF;

    printf("\npatriot_fpga_led_test\n");

    /* Turn on one by one LED */
    for (i = 0; i < 5; i++) {
	if (p1021_i2c_write_fpga_byte (offset, buf &= ~(1 << i))) {
	    printf("\n Read fail offset @%#x = %#x\n", offset, buf);
	    return (FAILED);
	}
	msleep(500);
    }

    /* Turn on all LEDs */
    if (p1021_i2c_write_fpga_byte (offset, 0xE0)) {
	printf("\n Read fail offset @%#x = %#x\n", offset, buf);
	return (FAILED);
    }
    msleep(500);

    /* Turn off all LEDs */
    if (p1021_i2c_write_fpga_byte (offset, 0x1F)) {
	printf("\n Read fail offset @%#x = %#x\n", offset, buf);
	return (FAILED);
    }
    
    return (PASSED);
}



/**********************************************************************
 *
 * Function: patriot_spi_prom_submenu
 *
 * This function provide a submenu for utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_spi_prom_submenu(void)
{
    uchar ch, stop = FALSE, spi_status = 0, ret_val;
    uchar mfg_id = 0, dev_id0 = 0, dev_id1 = 0, ex_dev_id0, ex_dev_id1;
    int spi_num;
    
    printf("\nPatriot SPI PROM Submenu\n");
    if (patriot_spi_prom_init()) {
	return (FAILED);
    }

    while (1) {
        printf("1. Read SPI PROM status register\n");
        printf("2. Read SPI PROM ID\n");
        printf("3. Read SPI PROM data\n");
        printf("4. Erase SPI PROM sector\n");
	printf("5. Write SPI PROM data\n");
	printf("6. Exit\n");
        ch = getc_answer("Select an option", "123456", '6');
	switch(ch) {
	case '1':
	    spi_num = gethex_answer("Select SPI PROM:0-Golden, 1-Upgrate, 2-FPGA",
				    0, 0, 2);
	    if (spi_num == 0) {
		spi_status = spi_prom_read_if_status(ESPI_CS0);
	    } else if (spi_num == 1) {
		spi_status = spi_prom_read_if_status(ESPI_CS2);
	    } else {
		spi_status = spi_prom_read_if_status(ESPI_CS3);
	    }
	    if (spi_status != INVALID_DATA) {
		printf("\nSPI PROM status = 0x%02x\n", spi_status);
	    }
	    break;
	case '2':
	    spi_num = gethex_answer("Select SPI PROM:0-Golden, 1-Upgrate, 2-FPGA",
				    0, 0, 2);
	    if (spi_num == 0) {
		ret_val = spi_prom_read_id(ESPI_CS0, &mfg_id, &dev_id0,
					   &dev_id1, &ex_dev_id0, &ex_dev_id1);
	    } else if (spi_num == 1) {
		ret_val = spi_prom_read_id(ESPI_CS2, &mfg_id, &dev_id0,
					   &dev_id1, &ex_dev_id0, &ex_dev_id1);
	    } else {
		ret_val = spi_prom_read_id(ESPI_CS3, &mfg_id, &dev_id0,
					   &dev_id1, &ex_dev_id0, &ex_dev_id1);
	    }
	    
	    if (ret_val != INVALID_DATA) {
		printf("\nMFG ID = 0x%02x, DEVICE ID = 0x%02x 0x%02x\n",
		       mfg_id, dev_id0, dev_id1);
		printf("EXTENDED DEVICE ID = 0x%02x 0x%02x\n", ex_dev_id0,
		       ex_dev_id1);
	    }
	    break;
	case '3':
	    ret_val = patriot_spi_prom_read_util();
	    break;
	case '4':
	    ret_val = patriot_spi_prom_erase_util();
	    break;
	case '5':
	    ret_val = patriot_spi_prom_write_util();
	    break;
	case '6':
	default:    
	    printf("Exit\n");
	    stop = TRUE;
	    break;	    
	}
	if (stop == TRUE)
	    break;
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_cpu_submenu
 *
 * This function provide a submenu for cpu utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_cpu_submenu(void)
{

    uchar ch, stop = FALSE, spi_status, ret_val;
    uchar mfg_id, dev_id0, dev_id1;
    int spi_num;
    
    printf("\nPatriot CPU Submenu\n");
    while (1) {
        printf("1. Display Local Access CCS registers\n");
        printf("2. Display LAW registers\n");
        printf("3. Display e500 Coherency Module (ECM) registers\n");
        printf("4. Display DDR1 controller registers\n");
	printf("5. Display Local Bus controller registers\n");
	printf("6. Display PIC registers\n");
	printf("7. Display I2C 1 registers\n");
	printf("8. Display PCIe port 0 registers\n");
	printf("9. Display PCIe port 1 registers\n");
	printf("A. Display GPIO registers\n");
	printf("B. Display POR registers\n");
	printf("C. Display L2 Cache registers\n");
	printf("D. Display ETSEC registers\n");
	printf("E. Display SPI registers\n");
	printf("F. Display QE submenu registers\n");
	printf("G. Alter CPU Registers\n");
	printf("H. Exit\n");
        ch = getc_answer("Select an option", "123456789ABCDEFGH", 'H');
	switch(ch) {
	case '1':
	    display_laccs_registers();
	    break;
	case '2':
	    display_law_registers();
	    break;
	case '3':
	    display_ecm_registers();
	    break;
	case '4':
	    display_ddr1_registers();
	    break;
	case '5':
	    display_lbus_registers();
	    break;
	case '6':
	    display_pic_registers();
	    break;
	case '7':
	    display_i2c1_registers();
	    break;
	case '8':
	    display_pcie_registers(0);
	    break;
	case '9':
	    display_pcie_registers(1);
	    break;
	case 'A':
	    display_gpio_registers();
	    break;
	case 'B':
	    display_por_registers();
	    break;
	case 'C':
	    display_l2cache_registers();
	    break;
	case 'D':
	    display_etsec_regs ();
	    break;
	case 'E':
	    display_espi_registers();
	    break;
	case 'F':
	    patriot_qe_submenu();
	    break;
	case 'G':
	    modify_cpu_regs();
	    break;
	case 'H':
	default:    
	    printf("Exit\n");
	    stop = TRUE;
	    break;	    
	}
	if (stop == TRUE)
	    break;
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_fpga_submenu
 *
 * This function provide a submenu for utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_submenu(void)
{
    uchar ch, stop = FALSE, ret_val = PASSED, buf;
    printf("\nPatriot FPGA Submenu\n");
    
    while (1) {
        printf("\n1. FPGA Register test\n");
        printf("2. FPGA Register Read\n");
        printf("3. FPGA Register Write\n");
        printf("4. Dump FPGA Register\n");
	printf("5. FPGA LED test\n");
        printf("6. Exit\n");
	printf("7. FPGA Interrupt test\n");
	printf("8. FPGA alter No Margin\n");
	printf("9. FPGA alter Low Margin\n");
	printf("A. FPGA alter High Margin\n");
	printf("B. FPGA dump all info\n");
        ch = getc_answer("Select an option", "123456789AB", '6');
        switch(ch) {
        case '1':
            ret_val = patriot_fpga_reg_test();
            break;
        case '2':
            patriot_fpga_register_read();
            break;
        case '3':
            patriot_fpga_register_write();
            break;
        case '4':
            patriot_dump_fpga_reg();
            break;
	case '5':
	    patriot_fpga_led_test();
	    break;
	case '7':
	    patriot_fpga_intr_test();
	    break;
	case '8':
	    patriot_power_no_margin();
	    break;
	case '9':
	    patriot_power_margin_low();
	    break;
	case 'A':
	    patriot_power_margin_high();
	    break;
	case 'B':
	    patriot_dump_fpga_info_to_host();
	    break;
        case '6':
        default:
            printf("Exit\n");
            stop = TRUE;
            break;
    }

    if (stop == TRUE)
        break;
    }
    return (ret_val);
}


/**********************************************************************
 *
 * Function: patriot_dump_ds3170_reg_submenu
 *
 * This function provide a submenu for ds3170 alter register
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_dump_ds3170_reg_submenu(void)
{
    uchar ch, stop = FALSE, ret_val = PASSED;
    printf("\nPatriot DS3170 Dump Register Submenu\n");
    
    while (1) {
    	printf("\n");
        printf("1. Dump DS3170 Global Register\n");
        printf("2. Dump DS3170 Port Register\n");
        printf("3. Dump DS3170 BERT Register\n");
        printf("4. Dump DS3170 Line Register\n");
        printf("5. Dump DS3170 HDLC Register\n");
        printf("6. Dump DS3170 FEAC Register\n");
        printf("7. Dump DS3170 Trail Trace Register\n");
        printf("8. Dump DS3170 DS3 Register\n");
        printf("9. Dump DS3170 E3 G751 Register\n");
        printf("A. Dump DS3170 E3 G832 Register\n");
        printf("B. Exit\n");
        ch = getc_answer("Select an option", "123456789AB", 'B');
        switch(ch) {
        case '1':
            patriot_ds3170_dump_reg(DS3170_GLOBAL);
            break;
        case '2':
            patriot_ds3170_dump_reg(DS3170_PORT);
            break;
        case '3':
            patriot_ds3170_dump_reg(DS3170_BERT);
            break;
        case '4':
            patriot_ds3170_dump_reg(DS3170_LINE);
            break;
        case '5':
            patriot_ds3170_dump_reg(DS3170_HDLC);
            break;
        case '6':
            patriot_ds3170_dump_reg(DS3170_FEAC);
            break;
        case '7':
            patriot_ds3170_dump_reg(DS3170_TT);
            break;
        case '8':
            patriot_ds3170_dump_reg(DS3170_T3);
            break;
        case '9':
            patriot_ds3170_dump_reg(DS3170_E3G751);
            break;
        case 'A':
            patriot_ds3170_dump_reg(DS3170_E3G832);
            break;
        case 'B':
        default:
            printf("Exit\n");
            stop = TRUE;
            break;
    }

    if (stop == TRUE)
        break;
    }
    return (ret_val);
}


/**********************************************************************
 *
 * Function: patriot_alter_ds3170_reg_submenu
 *
 * This function provide a submenu for ds3170 alter register
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_alter_ds3170_reg_submenu(void)
{
    uchar ch, stop = FALSE, ret_val = PASSED;
    printf("\nPatriot DS3170 Alter Register Submenu\n");

    while (1) {
        printf("1. Alter DS3170 Global Register\n");
        printf("2. Alter DS3170 Port Register\n");
        printf("3. Alter DS3170 BERT Register\n");
        printf("4. Alter DS3170 Line Register\n");
        printf("5. Alter DS3170 HDLC Register\n");
        printf("6. Alter DS3170 FEAC Register\n");
        printf("7. Alter DS3170 Trail Trace Register\n");
        printf("8. Alter DS3170 DS3 Register\n");
        printf("9. Alter DS3170 E3 G751 Register\n");
        printf("A. Alter DS3170 E3 G832 Register\n");
        printf("B. Exit\n");
        ch = getc_answer("Select an option", "123456789AB", 'B');
        switch(ch) {
        case '1':
            patriot_ds3170_alter_reg(DS3170_GLOBAL);
            break;
        case '2':
            patriot_ds3170_alter_reg(DS3170_PORT);
            break;
        case '3':
            patriot_ds3170_alter_reg(DS3170_BERT);
            break;
        case '4':
            patriot_ds3170_alter_reg(DS3170_LINE);
            break;
        case '5':
            patriot_ds3170_alter_reg(DS3170_HDLC);
            break;
        case '6':
            patriot_ds3170_alter_reg(DS3170_FEAC);
            break;
        case '7':
            patriot_ds3170_alter_reg(DS3170_TT);
            break;
        case '8':
            patriot_ds3170_alter_reg(DS3170_T3);
            break;
        case '9':
            patriot_ds3170_alter_reg(DS3170_E3G751);
            break;
        case 'A':
            patriot_ds3170_alter_reg(DS3170_E3G832);
            break;
        case 'B':
        default:
            printf("Exit\n");
            stop = TRUE;
            break;
    }

    if (stop == TRUE)
        break;
    }
    return (ret_val);
}


/**********************************************************************
 *
 * Function: patriot_ds3170_submenu
 *
 * This function provide a submenu for utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ds3170_submenu(void)
{
    int ch;
    uchar stop = FALSE, ret_val = PASSED;
    printf("\nPatriot DS3170 Submenu\n");

    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        printf("\nInitial fail\n");
        return (FAILED);
    }

    /* Reset then take it out of reset */
    patriot_ds3170_reset();
    /* Shutdown PMUXCR bit 2 to zero to become GPIO instead of SDHC_WP */
    REGB->im_gur.pmuxcr &= (~0x20000000);

    /* Init first the i2c controller */
    platform_cpu_i2c_init();

    while (1) {
        printf("1.  DS3170 Register test\n");
        printf("2.  Clear E3 AIS Test\n");
        printf("3.  Clear T3 Interrupt test\n");
        printf("4.  Clear T3 BERT test\n");
        printf("5.  Clear T3 Internal Loopback test\n");
        printf("6.  Clear T3 External Loopback test\n");
        printf("7.  Subrate T3 Internal Loopback Test\n");
        printf("8.  Subrate T3 External Loopback Test\n");
        printf("9.  Subrate T3 Individual Internal Loopback test\n");
        printf("A.  Subrate T3 Individual External Loopback test\n");
        printf("B.  Clear E3 Internal Loopback test\n");
        printf("C.  Clear E3 External Loopback test\n");
        printf("D.  Subrate E3 Internal Loopback Test\n");
        printf("E.  Subrate E3 External Loopback Test\n");
        printf("F.  Subrate E3 Individual Internal Loopback test\n");
        printf("G.  Subrate E3 Individual External Loopback test\n");
        printf("H.  Check / Display LED Test\n");
        printf("I.  DS3170 Register read\n");
        printf("J.  DS3170 Register write\n");
        printf("K.  Dump DS3170 Register submenu\n");
        printf("L.  Alter DS3170 Register submenu\n");
        printf("M.  Turn on / off bit Payload Loopback\n");
        printf("N.  Turn on / off bit Line Loopback\n");
        printf("O.  Exit\n");
        ch = getc_answer("Select an option", "123456789ABCDEFGHIJKLMNO", 'O');

        switch(ch) {
        case '1':
            ret_val = patriot_ds3170_reg_test();
            break;
        case '2':
            ret_val = patriot_clear_e3_ais_test();
            break;
        case '3':
            ret_val = patriot_clear_t3_intr_test();
            break;
        case '4':
            ret_val = patriot_clear_t3_bert_test();
            break;
        case '5':
            ret_val = patriot_clear_t3_int_lpbk_test();
            break;
        case '6':
            ret_val = patriot_clear_t3_ext_lpbk_test();
            break;
        case '7':
            ret_val = patriot_subrate_t3_int_lpbk_test();
            break;
        case '8':
            ret_val = patriot_subrate_t3_ext_lpbk_test();
            break;
        case '9':
            ret_val = patriot_subrate_t3_individual_int_lpbk_test();
            break;
        case 'A':
            ret_val = patriot_subrate_t3_individual_ext_lpbk_test();
            break;
        case 'B':
            ret_val = patriot_clear_e3_int_lpbk_test();
            break;
        case 'C':
            ret_val = patriot_clear_e3_ext_lpbk_test();
            break;
        case 'D':
            ret_val = patriot_subrate_e3_int_lpbk_test();
            break;
        case 'E':
            ret_val = patriot_subrate_e3_ext_lpbk_test();
            break;
        case 'F':
            ret_val = patriot_subrate_e3_individual_int_lpbk_test();
            break;
        case 'G':
            ret_val = patriot_subrate_e3_individual_ext_lpbk_test();
            break;
        case 'H':
            patriot_display_led();
            break;
        case 'I':
            patriot_ds3170_reg_read();
            break;
        case 'J':
            patriot_ds3170_reg_write();
            break;
        case 'K':
            patriot_dump_ds3170_reg_submenu();
            break;
        case 'L':
            patriot_alter_ds3170_reg_submenu();
            break;
        case 'M':
        	patriot_alter_ds3170_payload_lpbk();
        	break;
        case 'N':
        	patriot_alter_ds3170_line_lpbk();
        	break;
        case 'O':
        default:
            printf("Exit\n");
            stop = TRUE;
            break;
    }

    if (stop == TRUE)
        break;
    }
    return (ret_val);
}


/**********************************************************************
 *
 * Function: patriot_mem_submenu
 *
 * This function provide a submenu for memory
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_mem_submenu(void)
{
    uchar ch, stop = FALSE, ret_val;
    
    printf("\nPatriot Memory Submenu\n");
    while (1) {
        printf("1. Display Memory\n");
        printf("2. Alter Memory\n");
	printf("3. Memory Test\n");
	printf("4. Exit\n");
        ch = getc_answer("Select an option", "1234", '4');
	switch(ch) {
	case '1':
	    display_mem();
	    break;
	case '2':
	    modify_mem();
	    break;
	case '3':
	    patriot_memory_test();
	    break;
	case '4':
	default:    
	    printf("Exit\n");
	    stop = TRUE;
	    break;	    
	}
	if (stop == TRUE)
	    break;
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_submenu
 *
 * This function provide a submenu for utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_submenu(void)
{

    uchar ch = 0, stop = FALSE, spi_status, ret_val;
    uchar mfg_id, dev_id0, dev_id1;
    int spi_num, i;
    
    printf("\nPatriot Module Submenu\n");
    while (1) {
	printf("\n1. SPI PROM Submenu\n");
        printf("2. FPGA Submenu\n");
        printf("3. DS3170 Submenu\n");
        printf("4. Freescale CPU Submenu\n");
	printf("5. Memory Submenu\n");
	printf("6. Exit\n");
	printf("7. Reboot\n");
	printf("8. Internal Loopback\n");
	printf("9. FPGA Loopback\n");
	printf("A. Framer Loopback\n");
	printf("B. External Loopback\n");
	printf("C. MAC Loopback\n");
	printf("D. Test FPGA GPIO Framer GPIO\n");
	printf("E. Test GPIO 1 module\n");
	printf("F. Test GPIO 4 module\n");
	printf("G. Test GPIO 3 module\n");
	printf("H. ECC Memory Test\n");
	
        ch = getc_answer("Select an option", "123456789ABCDEFGH", '6');

	switch(ch) {
	case '1':
	    ret_val = patriot_spi_prom_submenu();
	    break;
	case '2':
	    ret_val = patriot_fpga_submenu();
	    break;
	case '3':
	    ret_val = patriot_ds3170_submenu();
	    break;
	case '4':
	    ret_val = patriot_cpu_submenu();
	    break;
	case '5':
	    ret_val = patriot_mem_submenu();
	    break;
	case '7':
	    REGB->im_gur.rstcr = 0x00000002;	    
	    break;
	case '8':
	    patriot_fs_ucc_lpbk_test();
	    break;
	case '9':
	    patriot_fpga_lpbk_test();
	    break;
	case 'A':
	    patriot_clear_t3_int_lpbk_test();
	    break;
	case 'B':
	    patriot_clear_t3_ext_lpbk_test();
	    break;
	case 'C':    
	    patriot_mac_lpbk_test();
	    break;
	case 'D':
    	ret_val = patriot_test_fpga_gpio_framer();
		break;
	case 'E':
		patriot_host_to_module_gpio1_wr1_test();
		patriot_host_to_module_gpio1_wr0_test();
	    break;
	case 'F':
		patriot_host_to_module_gpio4_wr1_test();
		patriot_host_to_module_gpio4_wr0_test();
		break;
	case 'G':
		patriot_module_to_host_gpio3_rd1_test();
		patriot_module_to_host_gpio3_rd0_test();
		break;
	case 'H':
	    patriot_ddr_ecc_single_bit_err_test ();
	    break;
	case '6':
	default:
	    printf("Exit\n");
	    stop = TRUE;
	    break;
	}
	if (stop == TRUE)
	    break;
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: patriot_switch_console
 *
 * This function provides the menu
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_switch_console(void)
{
    uchar ch;

    
    while (1) {
	printf("\nHit any key to start, '#' to quit");
	ch = getchar();
	if (ch == '#') {
	    break;
	}
	patriot_submenu();
	
    }

    return (PASSED); 
}


/**********************************************************************
 *
 * Function: patriot_qe_submenu
 *
 * This function provide a submenu for qe
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_qe_submenu(void)
{
    uchar ch, stop = FALSE, ret_val;

    printf("\nPatriot QE Submenu\n");
    while (1) {
        printf("1. Display QE RAM registers\n");
        printf("2. Display QE Interrupt Controller registers\n");
        printf("3. Display QE Communication Processor registers\n");
        printf("4. Display QE Multiplexer registers\n");
        printf("5. Display QE Timers registers\n");
        printf("6. Display QE SPI 1 registers\n");
        printf("7. Display QE Baud Rate Generator registers\n");
        printf("8. Display QE SI registers\n");
        printf("9. Display QE SI Routing Table registers\n");
        printf("A. Display QE UCC1 registers\n");
        printf("B. Display QE UCC3 registers\n");
        printf("C. Display QE UCC5 registers\n");
        printf("D. Display QE UCC7 registers\n");
        printf("E. Display QE Multi-PHY Controller registers\n");
        printf("F. Display QE Serial DMA registers\n");
        printf("G. Display QE Multi User RAM registers\n");
        printf("H. Reboot\n");
        printf("I. Exit\n");
        ch = getc_answer("Select an option", "123456789ABCDEFGHI", 'I');
    switch(ch) {
    case '1':
        display_qe_iram_registers();
        break;
    case '2':
        display_qe_irq_registers();
        break;
    case '3':
        display_qe_cp_registers();
        break;
    case '4':
        display_qe_mux_registers();
        break;
    case '5':
        display_qe_timer_registers();
        break;
    case '6':
        display_qe_spi1_registers();
        break;
    case '7':
        display_qe_brg_registers();
        break;
    case '8':
        display_qe_si_registers();
        break;
    case '9':
        display_qe_sirt_registers();
        break;
    case 'A':
        display_qe_ucc1_registers();
        break;
    case 'B':
        display_qe_ucc3_registers();
        break;
    case 'C':
        display_qe_ucc5_registers();
        break;
    case 'D':
        display_qe_ucc7_registers();
        break;
    case 'E':
        display_qe_utopia_registers();
        break;
    case 'F':
        display_qe_sdma_registers();
        break;
    case 'G':
        display_qe_muram_registers();
        break;
    case 'H':
        REGB->im_gur.rstcr = 0x00000002;
        break;
    case 'I':
    default:
        printf("Exit\n");
        stop = TRUE;
        break;
    }
    if (stop == TRUE)
        break;
    }
    return (PASSED);
}



/**********************************************************************
 *
 * Function: patriot_alter_ds3170_payload_lpbk
 *
 * This function provide a payload loopback utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_alter_ds3170_payload_lpbk(void)
{
    uchar temp = 0;

	printf("\npatriot_alter_ds3170_payload_lpbk\n");

    /* Using SPI directly control the Framer
     * enable / disable Payload loopback mode
     */
    if (ds3170_read(&temp, CR4_ADDR_H)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer read loopback mode reg fail\n");
        return (FAILED);
    }

    if (getc_answer("Payload loopback Turn ON / OFF (O/F)", "O/F", 'F') == 'O') {
	printf("\nPayload Loopback ON \n");
        temp |= CR4_LBM(0x3);
    } else {
	printf("\nPayload Loopback OFF \n");
        temp &= ~CR4_LBM(0x3);
    }
    
    if (ds3170_write(temp, CR4_ADDR_H)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer write loopback mode reg fail\n");
        return (FAILED);
    }
    
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_alter_ds3170_line_lpbk
 *
 * This function provide a line loopback utilities
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_alter_ds3170_line_lpbk(void)
{
    uchar temp = 0;

	printf("\npatriot_alter_ds3170_line_lpbk\n");

    /* Using SPI directly control the Framer
     * enable / disable Line loopback mode
     */
    if (ds3170_read(&temp, CR4_ADDR_H)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer read loopback mode reg fail\n");
        return (FAILED);
    }

    if (getc_answer("Line Loopback Turn ON / OFF (O/F)", "O/F", 'F') == 'O') {
	printf("\nLine Loopback ON \n");
        temp |= CR4_LBM(0x2);
    } else {
	printf("\nLine Loopback OFF \n");
        temp &= ~CR4_LBM(0x2);
    }
    
    if (ds3170_write(temp, CR4_ADDR_H)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer write loopback mode reg fail\n");
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_write_mac_addr
 *
 * This function write the board MAC address sending from host to
 * the SPI EEPROM
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_write_mac_addr(void)
{
    int addr_offset, i;
    unsigned char read_buf[MAC_ADDR_SIZE];

    printf("MAC addr: %02x %02x %02x %02x %02x %02x\n", param_arr[0],
	   param_arr[1], param_arr[2], param_arr[3], param_arr[4], param_arr[5]);

    /* Need to erease a sector of 64K */
    if (spi_prom_erase_if (0xFE0000,  /* Temporarily write to this sector */
        ERASE_64K_BLOCK, ESPI_CS0)) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to erase spi prom interface",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_WRITE_MAC_ADDR_FAIL);
    }

    addr_offset = 0xFE0000;
    for (i = 0; i < MAC_ADDR_SIZE; i++) {
        if (spi_prom_write_if(addr_offset, param_arr[i], ESPI_CS0)) {
            sprintf(err_msg, "\n%s, [#%d]:Fail to write spi prom interface",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }
        addr_offset++;
    }

    memset(&read_buf[0], 0, MAC_ADDR_SIZE);

    addr_offset = 0xFE0000;

    for (i = 0; i < MAC_ADDR_SIZE; i++) {
        if (spi_prom_read_if(addr_offset, &read_buf[i], ESPI_CS0)) {
            sprintf(err_msg, "\n%s, [#%d]:Fail to read spi prom interface",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }
        if (read_buf[i] != param_arr[i]) {
            sprintf(err_msg, "\n%s, [#%d]:Fail to write MAC addr, "
            		"write = 0x%02x, read = 0x%02x",
            		__FUNCTION__, __LINE__, param_arr[i], read_buf[i]);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }
        addr_offset++;
    }

#ifdef OTP_REGION
    addr_offset = 0x114;  /* OTP 1 */
    for (i = 0; i < MAC_ADDR_SIZE; i++) {
        if (spi_prom_otpp_if(addr_offset, param_arr[i], ESPI_CS0)) {
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }
        addr_offset++;
    }
    memset(&read_buf[0], 0, MAC_ADDR_SIZE);
    addr_offset = 0x114;

    for (i = 0; i < MAC_ADDR_SIZE; i++) {
        if (spi_prom_otpr_if(addr_offset, &read_buf[i], ESPI_CS0)) {
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }

        if (read_buf[i] != param_arr[i]) {
            printf("\nFail to write MAC addr, write = 0x%02x, read = 0x%02x",
                   param_arr[i], read_buf[i]);
            return (TO_HOST_WRITE_MAC_ADDR_FAIL);
        }
        addr_offset++;
    }    
    
#endif
    
    return (TO_HOST_WRITE_MAC_ADDR_OK);    
}


/**********************************************************************
 *
 * Function: patriot_fpga_get_version
 *
 * This function reads the FPGA version
 *
 * Input : None
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_fpga_get_version(uchar *version)
{
    ushort offset = 0xe;
    uchar buf = 0;

    if (p1021_i2c_read_fpga_byte (offset, &buf)) {
        sprintf(err_msg, "\n%s, [#%d]:Read fail offset @%#x = %#x\n",
        		__FUNCTION__, __LINE__, offset, buf);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    *version = buf;
    printf("\nFPGA Version Register @%#x = %#x\n", offset, buf);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_power_no_margin
 *
 * This function alter the power margin in patriot module to no margin
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_power_no_margin(void)
{
    uchar buf = (BIT6 | BIT7);


    if (p1021_i2c_read_fpga_byte (LED_CTL_REG, &buf)) {
	sprintf(err_msg, "\n%s, [#%d]:read power margin value, i2c_wr_failed !\n"
			, __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_POWER_NO_MARGIN_FAIL);
    }

    buf &= ~(BIT6 | BIT7);

    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:write power margin value, i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_POWER_NO_MARGIN_FAIL);
    }

    printf("\nFPGA Margin value @%#x = %#x\n", LED_CTL_REG, buf);
    printf("\nNo power margin \n");

    return (TO_HOST_POWER_NO_MARGIN_OK);
}


/**********************************************************************
 *
 * Function: patriot_power_margin_low
 *
 * This function alter the power margin in patriot module to low margin
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_power_margin_low(void)
{
    uchar buf = (BIT6 | BIT7);


    if (p1021_i2c_read_fpga_byte (LED_CTL_REG, &buf)) {
	sprintf(err_msg, "\n%s, [#%d]:read power margin value, i2c_wr_failed !\n"
			, __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_POWER_LOW_MARGIN_FAIL);
    }

    buf &= ~BIT7;
    buf |= BIT6;

    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:write power margin value, i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_POWER_LOW_MARGIN_FAIL);
    }

    printf("\nFPGA Margin value @%#x = %#x\n", LED_CTL_REG, buf);
    printf("\nLow power margin \n");

    return (TO_HOST_POWER_LOW_MARGIN_OK);
}


/**********************************************************************
 *
 * Function: patriot_power_margin_high
 *
 * This function alter the power margin in patriot module to high margin
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_power_margin_high(void)
{
    uchar buf = (BIT6 | BIT7);


    if (p1021_i2c_read_fpga_byte (LED_CTL_REG, &buf)) {
	sprintf(err_msg, "\n%s, [#%d]:read power margin value, i2c_wr_failed !\n"
			, __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_POWER_HIGH_MARGIN_FAIL);
    }

    buf &= ~BIT6;
    buf |= BIT7;

    if (p1021_i2c_write_fpga_byte (LED_CTL_REG, buf)) {
        sprintf(err_msg, "\n%s, [#%d]:write power margin value, i2c_wr_failed !\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_POWER_HIGH_MARGIN_FAIL);
    }

    printf("\nFPGA Margin value @%#x = %#x\n", LED_CTL_REG, buf);
    printf("\nHigh power margin \n");

    return (TO_HOST_POWER_HIGH_MARGIN_OK);
}

/**********************************************************************
 *
 * Function: patriot_upgrade_fpga_download_to_spi_prom
 *
 * This function download upgrade FPGA to the FPGA SPI PROM
 *
 * Input : None
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_upgrade_fpga_download_to_spi_prom(void)
{
    int i, size = 0, block_size = 0, num_pages;
    uchar *read_buf, buf;
    uchar fpga_header[FPGA_HEADER_SIZE];
    uchar header_read[FPGA_HEADER_SIZE];
    uchar *hmac_hdr_read_buf;
    printf("\npatriot_upgrade_fpga_download_to_spi_prom\n");

    patriot_fpga_reset();
    platform_cpu_i2c_init();
    
    /* Need to erease the sector for FPGA header*/ 
    if (spi_prom_erase_if (FPGA_HEADER_DATA_SECTOR,
			   ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase the sector for FPGA header fail\n",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    /* Need to erease the sector for FPGA HMAC header*/ 
    if (spi_prom_erase_if (FPGA_UPGRADE_HMAC_HEADER_ADDR,
			   ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase the sector for FPGA HMAC header fail\n",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }    
        
    /* Need to erease 8 sectors for FPGA */    
    for (i = 0; i < 8; i++) {
	if (spi_prom_erase_if (UPGRADE_FPGA_SECTOR_ADDR + i * SECTOR_SIZE,
			       ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase the 8 sector for FPGA fail\n",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }

    size = patriot_fpga_prom_size;
    block_size = SPI_PROM_PAGE_SIZE;
    if (patriot_fpga_prom_size % SPI_PROM_PAGE_SIZE) {
	num_pages = patriot_fpga_prom_size/SPI_PROM_PAGE_SIZE + 1;
    } else {
	num_pages = patriot_fpga_prom_size/SPI_PROM_PAGE_SIZE;
    }
#ifdef DEBUG
    printf("\nnum_pages = %d", num_pages);fflush(0);
#endif    
    for (i = 0; i < num_pages; i++) {
	if (size >= SPI_PROM_PAGE_SIZE) {
	    size = size - SPI_PROM_PAGE_SIZE;
	    block_size = SPI_PROM_PAGE_SIZE;
	} else {
	    block_size = size;
	}
	if (spi_prom_write_multi_bytes (UPGRADE_FPGA_SECTOR_ADDR +
					i*SPI_PROM_PAGE_SIZE,
				    &patriot_fpga_prom[i*SPI_PROM_PAGE_SIZE],
					 ESPI_CS3, block_size)) {
		sprintf(err_msg, "\n%s, [#%d]:Fail to write the FPGA",
				__FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
	if (i%100 == 0) {
	    printf("\n i = %d", i);fflush(0);
	}
    }

    /* Read back to verify */
    read_buf = (uchar *)malloc(patriot_fpga_prom_size);
    if (!read_buf) {
	sprintf(err_msg, "\n%s, [#%d]:Malloc memory for reading FPGA SPI PROM failed\n",
			__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
    	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    memset(read_buf, 0, patriot_fpga_prom_size);
    
    if (spi_prom_read_multi_bytes (UPGRADE_FPGA_SECTOR_ADDR, read_buf, ESPI_CS3,
				   patriot_fpga_prom_size)) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to read back the FPGA from FPGA SPI PROM",
			__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	free(read_buf);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }
    
    /* Compare the FPGA */
    for (i = 0; i < patriot_fpga_prom_size; i++) {
	if (read_buf[i] != patriot_fpga_prom[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Fail to verify FPGA at patriot_fpga_prom[%d]"
                         " expect 0x%02x, get 0x%02x",__FUNCTION__, __LINE__
	    		, i, patriot_fpga_prom[i],read_buf[i]);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    free(read_buf);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }
    free(read_buf);

    /* Write the image header */
    for (i = 0; i < 4; i++) {
	fpga_header[i] = (patriot_fpga_prom_ver >> (i * 8)) & 0xFF;
	fpga_header[i + 4] = fpga_header[i];
    }
    
    fpga_header[8] = 0xA2;
    fpga_header[9] = 0xFF;
    fpga_header[10] = 0xFF;
    fpga_header[11] = 0xFF;
    fpga_header[12] = 0x06;
    fpga_header[13] = 0x5D;
    fpga_header[14] = 0x4F;
    fpga_header[15] = 0x7E;

#ifdef DEBUG    
    for (i = 0; i < FPGA_HEADER_SIZE; i++) {
	printf("\nfpga_header[%d] = 0x%02x", i, fpga_header[i]); fflush(0);
    }
#endif
    
    if (spi_prom_write_multi_bytes (FPGA_HEADER_DATA_ADDR,
				    &fpga_header[0],
				    ESPI_CS3, FPGA_HEADER_SIZE)) {
	sprintf(err_msg, "\n%s, [#%d]:Failed to write the FPGA header\n",
			__FUNCTION__, __LINE__);fflush(0);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    /* Read back to verify */
    if (spi_prom_read_multi_bytes (FPGA_HEADER_DATA_ADDR, &header_read[0],
				   ESPI_CS3,
				   FPGA_HEADER_SIZE)) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to read back the header from FPGA SPI PROM",
			__FUNCTION__, __LINE__);fflush(0);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    for (i = 0; i < FPGA_HEADER_SIZE; i++) {
	if (header_read[i] != fpga_header[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Fail to verify header at fpga_header[%d]"
                " expect 0x%02x, get 0x%02x",
	    		__FUNCTION__, __LINE__, i, fpga_header[i],
		   header_read[i]);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }

    /* Start writing the HMAC HDR */
    size = patriot_fpga_hmac_size;
    block_size = SPI_PROM_PAGE_SIZE;
    if (patriot_fpga_hmac_size % SPI_PROM_PAGE_SIZE) {
	num_pages = patriot_fpga_hmac_size/SPI_PROM_PAGE_SIZE + 1;
    } else {
	num_pages = patriot_fpga_hmac_size/SPI_PROM_PAGE_SIZE;
    }
    for (i = 0; i < num_pages; i++) {
	if (size >= SPI_PROM_PAGE_SIZE) {
	    size = size - SPI_PROM_PAGE_SIZE;
	    block_size = SPI_PROM_PAGE_SIZE;
	} else {
	    block_size = size;
	}
	if (spi_prom_write_multi_bytes (FPGA_UPGRADE_HMAC_HEADER_ADDR +
					i*SPI_PROM_PAGE_SIZE,
					&patriot_fpga_hmac[i*SPI_PROM_PAGE_SIZE],
					ESPI_CS3, block_size)) {
	    sprintf(err_msg, "\n%s, [#%d]:Failed to write the FPGA HMAC header\n",
	    		__FUNCTION__, __LINE__);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }
    /* Read back to verify */
    hmac_hdr_read_buf = (uchar *)malloc(patriot_fpga_hmac_size);

    if (!hmac_hdr_read_buf) {
        sprintf(err_msg, "\n%s, [#%d]:Malloc memory for reading FPGA HMAC"
        		" header failed\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }
    memset(hmac_hdr_read_buf, 0, patriot_fpga_hmac_size);
    if (spi_prom_read_multi_bytes (FPGA_UPGRADE_HMAC_HEADER_ADDR,
				   &hmac_hdr_read_buf[0],
				   ESPI_CS3,
				   patriot_fpga_hmac_size)) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to read back the HMAC header from"
			" FPGA SPI PROM", __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	free(hmac_hdr_read_buf);
	return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    for (i = 0; i < patriot_fpga_hmac_size; i++) {
	if (hmac_hdr_read_buf[i] != patriot_fpga_hmac[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Fail to verify HMAC header at "
	    		"patriot_fpga_hmac[%d] expect 0x%02x, get 0x%02x"
	    		, __FUNCTION__, __LINE__, i, patriot_fpga_hmac[i],
		   hmac_hdr_read_buf[i]);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    free(hmac_hdr_read_buf);
	    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }
    free(hmac_hdr_read_buf);

    printf("\nProgramming upgrade FPGA completed\n");fflush(0);
    return (TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_OK);
    
}


/**********************************************************************
 *
 * Function: patriot_golden_fpga_download_to_spi_prom
 *
 * This function download golden FPGA to the FPGA SPI PROM
 *
 * Input : None
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int patriot_golden_fpga_download_to_spi_prom(void)
{
    int i, size = 0, block_size = 0, num_pages;
    uchar *read_buf, buf;
    uchar fpga_header[FPGA_HEADER_SIZE];
    uchar header_read[FPGA_HEADER_SIZE];
    uchar *hmac_hdr_read_buf;
    printf("\npatriot_golden_fpga_download_to_spi_prom\n");

    patriot_fpga_reset();
    platform_cpu_i2c_init();
    
    /* Need to erease the sector for FPGA header */ 
    if (spi_prom_erase_if (FPGA_HEADER_DATA_SECTOR,
			   ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase the sector for FPGA header fail\n",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    /* Need to erease the sector for FPGA HMAC header*/ 
    if (spi_prom_erase_if (FPGA_GOLDEN_HMAC_HEADER_ADDR,
			   ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase the sector for FPGA HMAC header "
    			"fail\n", __FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }    

    /* Need to erease 8 sectors for FPGA */    
    for (i = 0; i < 8; i++) {
	if (spi_prom_erase_if (GOLDEN_FPGA_SECTOR_ADDR + i * SECTOR_SIZE,
			       ERASE_64K_BLOCK, ESPI_CS3)) {
    	sprintf(err_msg, "\n%s, [#%d]:Erase to 8 sector for FPGA fail\n",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }

    size = patriot_fpga_prom_size;
    block_size = SPI_PROM_PAGE_SIZE;
    if (patriot_fpga_prom_size % SPI_PROM_PAGE_SIZE) {
	num_pages = patriot_fpga_prom_size/SPI_PROM_PAGE_SIZE + 1;
    } else {
	num_pages = patriot_fpga_prom_size/SPI_PROM_PAGE_SIZE;
    }
#ifdef DEBUG
    printf("\nnum_pages = %d", num_pages);fflush(0);
#endif    
    for (i = 0; i < num_pages; i++) {
	if (size >= SPI_PROM_PAGE_SIZE) {
	    size = size - SPI_PROM_PAGE_SIZE;
	    block_size = SPI_PROM_PAGE_SIZE;
	} else {
	    block_size = size;
	}
	if (spi_prom_write_multi_bytes (GOLDEN_FPGA_SECTOR_ADDR +
					i*SPI_PROM_PAGE_SIZE,
				    &patriot_fpga_prom[i*SPI_PROM_PAGE_SIZE],
					 ESPI_CS3, block_size)) {
    	sprintf(err_msg, "\n%s, [#%d]:Fail to write the FPGA from FPGA SPI PROM",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
	if (i%100 == 0) {
	    printf("\n i = %d", i);fflush(0);
	}
    }
    /* Read back to verify */
    read_buf = (uchar *)malloc(patriot_fpga_prom_size);
    if (!read_buf) {
	sprintf(err_msg, "\n%s, [#%d]:Malloc memory for reading FPGA SPI PROM failed",
			__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
    	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    memset(read_buf, 0, patriot_fpga_prom_size);
    
    if (spi_prom_read_multi_bytes (GOLDEN_FPGA_SECTOR_ADDR, read_buf, ESPI_CS3,
				   patriot_fpga_prom_size)) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to read back the FPGA from FPGA SPI PROM",
			__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	free(read_buf);
	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }
    
    /* Compare the FPGA */
    for (i = 0; i < patriot_fpga_prom_size; i++) {
	if (read_buf[i] != patriot_fpga_prom[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Fail to verify FPGA at patriot_fpga_prom[%d]"
                " expect 0x%02x, get 0x%02x",__FUNCTION__, __LINE__
                , i, patriot_fpga_prom[i], read_buf[i]);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    free(read_buf);
	    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }
    
    free(read_buf);
    /* Start writing the HMAC HDR */
    size = patriot_fpga_hmac_size;
    block_size = SPI_PROM_PAGE_SIZE;
    if (patriot_fpga_hmac_size % SPI_PROM_PAGE_SIZE) {
	num_pages = patriot_fpga_hmac_size/SPI_PROM_PAGE_SIZE + 1;
    } else {
	num_pages = patriot_fpga_hmac_size/SPI_PROM_PAGE_SIZE;
    }
    for (i = 0; i < num_pages; i++) {
	if (size >= SPI_PROM_PAGE_SIZE) {
	    size = size - SPI_PROM_PAGE_SIZE;
	    block_size = SPI_PROM_PAGE_SIZE;
	} else {
	    block_size = size;
	}
	if (spi_prom_write_multi_bytes (FPGA_GOLDEN_HMAC_HEADER_ADDR +
					i*SPI_PROM_PAGE_SIZE,
					&patriot_fpga_hmac[i*SPI_PROM_PAGE_SIZE],
					ESPI_CS3, block_size)) {
	    sprintf(err_msg, "\n%s, [#%d]:Failed to write the FPGA HMAC header\n",
	    		__FUNCTION__, __LINE__);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }
    /* Read back to verify */
    hmac_hdr_read_buf = (uchar *)malloc(patriot_fpga_hmac_size);
    
    if (!hmac_hdr_read_buf) {
	sprintf(err_msg, "\n%s, [#%d]:Malloc memory for reading FPGA HMAC header"
			" failed\n", __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }
    
    if (spi_prom_read_multi_bytes (FPGA_GOLDEN_HMAC_HEADER_ADDR,
				   &hmac_hdr_read_buf[0],
				   ESPI_CS3,
				   patriot_fpga_hmac_size)) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to read back the HMAC header from FPGA"
			" SPI PROM", __FUNCTION__, __LINE__);
	fflush(0);
	print_err(TRUE, err_msg, LVL_0);
	free(hmac_hdr_read_buf);
	return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
    }

    for (i = 0; i < patriot_fpga_hmac_size; i++) {
	if (hmac_hdr_read_buf[i] != patriot_fpga_hmac[i]) {
	    sprintf(err_msg, "\n%s, [#%d]:Fail to verify HMAC header at "
	    		"patriot_fpga_hmac[%d] expect 0x%02x, get 0x%02x"
	    		, __FUNCTION__, __LINE__, i, patriot_fpga_hmac[i],
		   hmac_hdr_read_buf[i]);fflush(0);
	    print_err(TRUE, err_msg, LVL_0);
	    free(hmac_hdr_read_buf);
	    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL);
	}
    }

    free(hmac_hdr_read_buf);

    printf("\nProgramming golden FPGA completed\n");fflush(0);
 
    return (TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_OK);
    
}


/**********************************************************************
 *
 * Function: patriot_dump_fpga_info_to_host
 *
 * This function read all FPGA info and send them to the host
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_dump_fpga_info_to_host(void)
{
    int i;
    uchar buf_0[BLOCK_256];  /* Maximum I2C addressing range is 256 */
    uchar buf_1[BLOCK_256];
    uchar buf_2[BLOCK_256];
    uchar buf_3[BLOCK_256];
    uchar dev_addr, dev_offset, data;

    
#ifdef DEBUG    
    printf("\nBLOCK 0:\n");
#endif    
    for (i = 0; i < BLOCK_256; i++) {
	if (p1021_i2c_read_bytes (MB_I2C_ADDR_FPGA, i, 1, &buf_0[i],
			      CPU_I2C0)) {
	    return (TO_HOST_FPGA_READ_INFO_FAIL);
	}
#ifdef DEBUG	
	if (i % 16 == 0) {
	    printf("\n 0x%02x: ", i);
	}
	printf("0x%02x ", buf_0[i]);
#endif	
    }
#ifdef DEBUG    
    printf("\nBLOCK 1:\n");
#endif    
    for (i = 0; i < BLOCK_256; i++) {
	if (p1021_i2c_read_bytes (MB_I2C_ADDR_FPGA1, i, 1, &buf_1[i],
			      CPU_I2C0)) {
        return (TO_HOST_FPGA_READ_INFO_FAIL);
	}
#ifdef DEBUG	
	if (i % 16 == 0) {
	    printf("\n 0x%02x: ", i);
	}
	printf("0x%02x ", buf_1[i]);
#endif	
    }
#ifdef DEBUG    
    printf("\nBLOCK 2:\n");
#endif    
    for (i = 0; i < BLOCK_256; i++) {
	if (p1021_i2c_read_bytes (MB_I2C_ADDR_FPGA2, i, 1, &buf_2[i],
			      CPU_I2C0)) {
        return (TO_HOST_FPGA_READ_INFO_FAIL);
	}
#ifdef DEBUG	
	if (i % 16 == 0) {
	    printf("\n 0x%02x: ", i);
	}
	printf("0x%02x ", buf_2[i]);
#endif	
    }
#ifdef DEBUG    
    printf("\nBLOCK 3:\n");
#endif    
    for (i = 0; i < BLOCK_256; i++) {
	if (p1021_i2c_read_bytes (MB_I2C_ADDR_FPGA3, i, 1, &buf_3[i],
			      CPU_I2C0)) {
        return (TO_HOST_FPGA_READ_INFO_FAIL);
	}
#ifdef DEBUG	
	if (i % 16 == 0) {
	    printf("\n 0x%02x: ", i);
	}
	printf("0x%02x ", buf_3[i]);
#endif	
    }
    

#ifdef DEBUG
    dev_addr = gethex_answer("\nI2C Device Address: ", 0, 0, 0xff);
    dev_addr = dev_addr >> 1;

    dev_offset = gethex_answer("\nI2C Device Offset: ", 0, 0, 0xff);

    p1021_i2c_read_bytes (dev_addr, dev_offset, 1, &data,
			  CPU_I2C0);
	
    printf("\ndata at offset 0x%02x = 0x%02x", dev_offset, data);
#endif    

    /* Clean up the tx packet */
    memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
    /* Copy the data over, leave the first 12 bytes for test results
     and parameters */
    
    memcpy((char *)&(tx_packet_p->data[12]), (char *)&(buf_0[0]),
    	   BLOCK_256);
    memcpy((char *)&(tx_packet_p->data[12 + BLOCK_256]), (char *)&(buf_1[0]),
    	   BLOCK_256);    
    memcpy((char *)&(tx_packet_p->data[12 + 2 * BLOCK_256]), (char *)&(buf_2[0]),
    	   BLOCK_256);
    memcpy((char *)&(tx_packet_p->data[12 + 3 * BLOCK_256]), (char *)&(buf_3[0]),
    	   BLOCK_256);

    return (TO_HOST_FPGA_READ_INFO_OK);
}


/*------------------------------------------------------------------------------
 * $Log: patriot_util.c,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.11  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.10  2012/10/25 07:24:10  steja
 * Remove "Clear" from the subrate test
 *
 * Revision 1.9  2012/10/16 07:42:40  steja
 * Improve the GPIO test
 *
 * Revision 1.8  2012/09/14 23:41:56  huanngo
 * Adding the utility to display FPGA secure boot registers and multiboot info table
 *
 * Revision 1.7  2012/08/07 07:06:19  steja
 * 1. Update the patriot_conf_ds3170_frmr function by add parameter bypass FPGA
 * 2. adjust msleep for ds3170 reset
 *
 * Revision 1.6  2012/07/31 00:13:36  huanngo
 * Remove the modification on reconfiguration control register on FPGA
 *
 * Revision 1.5  2012/07/18 23:54:23  huanngo
 * Add functions to support FPGA programming to SPI PROM
 *
 * Revision 1.4  2012/06/11 07:43:28  steja
 * Add Power Margin Utilities
 *
 * Revision 1.3  2012/06/08 23:35:39  huanngo
 * Adding constant definitions for ECC memory,UART and GE 0 loopback tests
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.22  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.21  2012/03/21 09:33:24  steja
 * Update code
 *
 * Revision 1.1.4.20  2012/03/21 08:38:20  steja
 * Include  the verilink subrate on the loopback test
 *
 * Revision 1.1.4.19  2012/03/21 07:31:09  steja
 * 1. adjust subrate test time slots
 * 2. add debug flag
 *
 * Revision 1.1.4.18  2012/03/13 13:43:51  steja
 * Add Debug output flag
 *
 * Revision 1.1.4.17  2012/02/28 02:23:38  huanngo
 * Fix the LED test on module side
 *
 * Revision 1.1.4.16  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.15  2011/12/22 12:11:50  steja
 * Fix the GPIO test for Rev 1B board.
 *
 * Revision 1.1.4.14  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.13  2011/12/08 15:07:11  steja
 * Update IO Test function
 *
 * Revision 1.1.4.12  2011/12/01 18:51:49  huanngo
 * Adding new function to write MAC address to EEPROM
 *
 * Revision 1.1.4.11  2011/11/24 09:33:34  steja
 * Update Patriot code
 *
 * Revision 1.1.4.10  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.9  2011/11/17 15:18:08  steja
 * Add Test Function for FPGA GPIO and GPIO on Framer
 *
 * Revision 1.1.4.8  2011/10/27 09:35:08  steja
 * Update DS3170 BERT test
 *
 * Revision 1.1.4.7  2011/10/12 15:22:56  steja
 * Update DS3170 Submenu for :
 * add Payload loopback and Line loopback utilities test.
 *
 * Revision 1.1.4.6  2011/10/11 01:51:29  steja
 * Update DS3170 Register test code
 *
 * Revision 1.1.4.5  2011/10/07 01:11:46  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.4  2011/08/30 10:23:03  steja
 * Update P1021 display QE register code
 *
 * Revision 1.1.4.3  2011/08/26 14:44:56  steja
 * Update p1021 code to display SPI registers
 *
 * Revision 1.1.4.2  2011/08/18 19:43:25  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.29  2011/08/16 17:57:59  huanngo
 * Fix bugs for SPI EEPROM
 *
 * Revision 1.1.2.28  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.27  2011/07/26 14:35:51  steja
 * Update DS3170 code
 *
 * Revision 1.1.2.26  2011/07/21 12:14:15  steja
 * Update DS3170 functionality
 *
 * Revision 1.1.2.25  2011/07/19 06:11:35  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.24  2011/07/11 07:43:10  steja
 * Update DS3170 function patriot_clear_e3_ais_test(void)
 *
 * Revision 1.1.2.23  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.22  2011/07/07 16:21:54  steja
 * 1. Clean up code
 * 2. Add check statur register after loopback test for DS3170.
 *
 * Revision 1.1.2.21  2011/07/05 09:57:36  steja
 * Update Loopback pass for DS3170 code
 *
 * Revision 1.1.2.20  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.19  2011/07/01 17:27:21  steja
 * Update the DS3170 utility for dump and alter register function
 *
 * Revision 1.1.2.18  2011/07/01 15:39:07  steja
 * 1. Update DS3170 utility test code
 * 2. Update Internal and External loopback test for DS3170
 *
 * Revision 1.1.2.17  2011/06/30 16:31:42  steja
 * 1. Update DS3170 Register table
 * 2. Update DS3170 patriot_clear_t3_intr_test
 *
 * Revision 1.1.2.16  2011/06/29 16:24:55  steja
 * Update DS3170 code.
 *
 * Revision 1.1.2.15  2011/06/28 16:59:50  steja
 * 1. Update FPGA register read and write function
 * 2. Update DS3170 register test function
 * 3. Update Common register test, reg alter, reg display
 *
 * Revision 1.1.2.14  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.13  2011/06/27 14:14:07  steja
 * 1. Update FPGA register test function
 * 2. Add FPGA dump register function
 * 3. Add FPGA register read / write utility function
 * 4. Add FPGA initialization function
 *
 * Revision 1.1.2.12  2011/06/22 02:37:18  steja
 * Update DS3170 code Interrupt Handler function
 *
 * Revision 1.1.2.11  2011/06/17 07:03:54  steja
 * 1. Move Patriot_fpga_test to patriot_main.c
 * 2. Remove fpga loopback test item
 *
 * Revision 1.1.2.10  2011/06/16 11:35:53  steja
 * Add Display LED for DS3170 Utility
 *
 * Revision 1.1.2.9  2011/06/14 10:13:42  steja
 * Update DS3170 code and FPGA Register test
 *
 * Revision 1.1.2.8  2011/06/13 12:21:27  steja
 * Add submenu utilites for DS3170 and FPGA
 *
 * Revision 1.1.2.7  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.6  2011/06/09 07:03:37  steja
 * Update the code for DS3170 and FPGA's Patriot
 *
 * Revision 1.1.2.5  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.4  2011/05/26 00:38:11  huanngo
 * Update with SPI PROM access and FPGA, DS3170 reset functions
 * Change the SPI read/write to uchar access
 *
 * Revision 1.1.2.3  2011/05/25 16:05:06  steja
 * Update the DS3170 testing function based on specs
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

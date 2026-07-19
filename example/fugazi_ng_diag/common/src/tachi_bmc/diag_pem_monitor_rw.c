/* $Id: diag_pem_monitor_rw.c,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_pem_monitor_rw.c,v $
 *------------------------------------------------------------------
 *
 * diag_pem_monitor_wr.c: 
 * porting from o2 platform_pem_monitor_read.c: 
 *             Utility to read and display Power Supply monitor,
 *             information, PSU_MODEL, HOUR_ELAPSE, 12V_I, 12V_V,
 *             OUT_TEMP, and INPUT_V.
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
 */
#include "diag_pem_lib.h"
#include <stdio.h>
#include "common.h"
#include "types.h"
#include "queryflags.h"

#define  ERR_CHECKER_CLEAR_ERRORS 0   /* Just Clear -- no errors */
#define  ERR_CHECKER_LEAVE_ERRORS 1   /* Just Check -- don't explicitly clear */
#define  ERR_CHECKER_CHECK_THEN_CLEAR_ERRORS 2 /* Check and then explicitly clear */

#define PEM_CONTROL_OFFSET      0x63 
#define PEM_FAULT_ENABLE_BIT    0x40
#define PEM_WARN_ENABLE_BIT     0x80
#define PEM_TEST_OFFSET         0xD0 
#define PEM_STATUS_OFFSET       0x62 

typedef struct rp1ruve_pem_reg_st {
    char          *name;
    uint32_t       offset;
} rp1ruve_pem_reg_t;


/* Function prototype */
static int pem_register_read(int, uint32_t, uint16_t *);
int pem_monitor_write(int);
int pem_monitor_dump(int);
int pem_get_sw_ver(int, unsigned char *, unsigned char *);
int pem_get_type(int, uchar *);
int pem_force_error_check_ver(int);
static int pem_register_read(int, uint32_t, uint16_t *);
static int pem_register_mask(int, uint32_t, uint16_t, int);
int pem_create_error(int, uint32_t);
static int16_t pem_force_error(uint32_t);
int16_t pem_force_error_f(void);
int pem_alert_enable(uint32_t, uint32_t);
int16_t pem_alert_enable_f(void);
static int pem_check_data(int, rp1ruve_pem_reg_t *, int, char*, uint32_t, int);
static int pem_check_core(uint32_t, int); 
int pem_check(void);
int pem_clear(void);


/**********************************************************************
 *
 * Function: pem_monitor_write
 *
 * Description: write the data to PEM register manully. 
 *
 * Inputs:  ps_num - PSU number
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_monitor_write (int ps_num) {
    
    uint offset, value;
  
    printf("PSU%d ", ps_num);
    /* the only writable registers are 0x03 (clear fault) 
     * and 0x3B (fan command 1) */
    offset = gethex_answer("Offset [0-0xff] ", 0, 0, 0xff); 
    value = gethex_answer("Reg Value [0x0000-0xffff] ", 0, 0, 0xffff);

    /* Read PEM Monitor Data Packet into local buffer */
    if (rp1ruve_pem_write(ps_num, offset, value) != 0) {
      cterr('f', 0,"PS%d monitor write failure!!!", ps_num);
      return FAILED;
    } 
  
  return (PASSED);
  
} /* pem_monitor_write() */

/**********************************************************************
 *
 * Function: pem_monitor_dump
 *
 * Description: dump the data on each psu.
 *
 * Inputs:  PSU number
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_monitor_dump (int ps_num) {
    
    /* Read PEM Monitor Data Packet into local buffer */
    if (rp1ruve_pem_data_read_all(ps_num) != 0) {
	cterr('f', 0,"PS%d monitor read failure!!!", ps_num);
        return FAILED;
    } 

    rp1ruve_pem_display(ps_num);
    
    return PASSED;
  
} /* pem_monitor_dump() */


/**********************************************************************
 *
 * Function: pem_get_sw_ver
 *
 * Description: read firmware rev. 
 *
 * Inputs:  ps_num - PSU num.
 *          major - major num
 *          minor - minor num
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_get_sw_ver( int ps_num, unsigned char *major, unsigned char *minor ) {

    unsigned short int data;

    *major = 0;
    *minor = 0;

    if ( rp1ruve_pem_read( ps_num, FIRMWARE_REVISION, &data )) 
	return TEST_FAILED;

    *minor = data & 0xFF;
    *major = data >> 8;

    return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: pem_get_type
 *
 * Description: get current PSU type 
 *
 * Inputs:  ps_num  - psu num
 *          type    - ACDC/DCDC
 *
 * Outputs: PASSED / FAILED
 *
 * The 0xC0 / 0xC1 combination for the AC unit should read "ACDC"
 *  (0x4143 / 0x4443) without the byte swapping
 * The 0xC0 / 0xC1 combination for the DC unit should read "DCDC"
 *  (0x4443 / 0x4443) without the byte swapping
 **********************************************************************
 */
int pem_get_type( int ps_num, uchar *type ) {

    unsigned short int data_c0, data_c1;

    if ( pem_register_read( ps_num, 0xC0, &data_c0 )) 
	return TEST_FAILED;
    
    if ( pem_register_read( ps_num, 0xC1, &data_c1 )) 
	return TEST_FAILED;

    if (( data_c0 == 0x4341 ) && ( data_c1 == 0x4344 ))
	*type = RP1RUVE_PEM_TYPE_ACDC;
    else if (( data_c0 == 0x4344 ) && ( data_c1 == 0x4344 )) 
	*type = RP1RUVE_PEM_TYPE_DCDC;
    else
	*type = RP1RUVE_PEM_TYPE_UNKNOWN;

    return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: pem_force_error_check_ver
 *
 * Description: check the firmware rev and PSU type
 *
 * Inputs:  ps_num  - psu num
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_force_error_check_ver( int ps_num ) {

    u_char major, minor, type;
    
    if ( pem_get_sw_ver( ps_num, &major, &minor ))
	return TEST_FAILED;

    if ( pem_get_type( ps_num, &type ))
	return TEST_FAILED;

    /* version at least 5.2 to do force error for AC/DC */
    if ( type == RP1RUVE_PEM_TYPE_ACDC ) {
	if (( 5 <= major ) && ( 2 <= minor )) 
	    return TEST_PASSED;
	else {
	    printf(" PEM#%d AC/DC Firmware version %d.%d does not support fault insertion. \n\r",
		      ps_num, major, minor );
	    return TEST_FAILED;
	}
    }
    /* version at least 3.1 to do force error for DC/DC */
    else if ( type == RP1RUVE_PEM_TYPE_DCDC ) {
	if (( 3 <= major ) && ( 1 <= minor )) 
	    return TEST_PASSED;
	else {
	    printf(" PEM#%d DC/DC Firmware version %d.%d does not support fault insertion. \n\r",
		      ps_num, major, minor );
	    return TEST_FAILED;
	}
    }
    else {
	printf(" Unknown PEM type! \n\r" );
	return TEST_FAILED;
    }
}

/**********************************************************************
 *
 * Function: pem_register_read
 *
 * Description: read PEM register. 
 *
 * Inputs:  ps_num  - psu num
 *          addr - offset
 *          data - read data
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
static int pem_register_read( int ps_num, uint32_t addr, uint16_t *data ) {
    
    if ( rp1ruve_pem_read( ps_num, addr, data ))
	return TEST_FAILED;	
    
    return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: pem_register_mask
 *
 * Description: using a mask to clear/set the register
 *
 * Inputs:  ps_num  - psu num
 *          addr - offset
 *          mask - data mask
 *          mode - mask/unmask
 *  
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
static int pem_register_mask( int ps_num, uint32_t addr, uint16_t mask, int mode ) {
    
    uint16_t data;

    if ( pem_register_read( ps_num, addr, &data ))
	return TEST_FAILED;

    if ( mode == 0 )
	data |= mask;
    else
	data &= ~mask;

    if ( rp1ruve_pem_write( ps_num, addr, data ))
	return TEST_FAILED;
    
    return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: pem_create_error
 *
 * Description: force error test. 
 *
 * Inputs:  ps_num  - psu num
 *          force_error - force error
 * 
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_create_error( int ps_num, uint32_t force_error ) {

    int     set_mode;
    uint16_t mask;
    uint16_t data;

    /* set the mode if force error */
    set_mode = ( force_error == 1 ) ? 0 : 1;

    mask = PEM_FAULT_ENABLE_BIT | PEM_WARN_ENABLE_BIT;

    if ( pem_register_mask( ps_num, PEM_TEST_OFFSET, mask, set_mode ))
	return TEST_FAILED;

    if ( pem_register_mask( ps_num, PEM_CONTROL_OFFSET, mask, set_mode ))
	return TEST_FAILED;
		
    if ( pem_register_read( ps_num, PEM_STATUS_OFFSET, &data ))
	return TEST_FAILED;

    if ( ((   force_error )  && ( ! (data & 0x40)) ) ||
	 (( ! force_error )  && (   (data & 0x40)) )) {
	cterr('f', 0,"PS%d register [0x%0x]=0x%x BIT 6 is not %s!!!", ps_num, PEM_STATUS_OFFSET, data,
		   (force_error)?"set":"cleared");

	return TEST_FAILED;
    }

    return TEST_PASSED;
}

/**********************************************************************
 *
 * Function: pem_force_error
 *
 * Description: force error test.
 *
 * Inputs:  force_error - force error
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
static int16_t pem_force_error( uint32_t force_error ) {

    int ps_num = PSU_ONE;

    if ( pem_force_error_check_ver( ps_num )) {
        printf(" Returning PASSED for PEM#%d \n\r", ps_num );
        return TEST_PASSED;
    }

    return pem_create_error( ps_num, force_error );

}

/**********************************************************************
 *
 * Function: pem_force_error_f
 *
 * Description: wrapper for force error test.
 *
 * Inputs:  NONE
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int16_t pem_force_error_f (void) {
    uint32_t force_error_param = 0;
    force_error_param = getdec_answer("Error insertion (0-clear 1-force) ", 0, 0, 1);
    return pem_force_error( force_error_param );
}

/**********************************************************************
 *
 * Function: pem_alert_enable
 *
 * Description: enable PEM alert
 *
 * Inputs:  shutdown - enable shutdown
 *          warning - enable warning
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_alert_enable (uint32_t shutdown, uint32_t warning) {

    uint16_t data;
    int ps_num = PSU_ONE;

    data  = 0;
    data |= ( shutdown == 1 ) ? PEM_FAULT_ENABLE_BIT : 0;
    data |= ( warning  == 1 ) ? PEM_WARN_ENABLE_BIT  : 0;

    if ( pem_register_mask( ps_num, PEM_CONTROL_OFFSET, data, 1 )) {
        return TEST_FAILED;
    }

    if ( pem_register_mask( ps_num, PEM_CONTROL_OFFSET, data, 0 )) {
        return TEST_FAILED;
    }

    return TEST_PASSED;    
}

/**********************************************************************
 *
 * Function: pem_alert_enable_f
 *
 * Description: wrapper for enable PEM alert
 *
 * Inputs:  NONE
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int16_t pem_alert_enable_f( void ) {
    uint32_t pem_shutdown_param = 0;
    uint32_t pem_warning_param = 0;

    pem_shutdown_param = getdec_answer("Enable Shutdown event (0-no 1-yes) ", 0, 0, 1);
    pem_warning_param = getdec_answer("Enable Warning event (0-no 1-yes) ", 0, 0, 1);
    return pem_alert_enable( pem_shutdown_param, pem_warning_param );
}

/**********************************************************************
 *
 * Function: pem_check_data
 *
 * Description: check PEM data
 *  
 * Inputs:  ps_num - psu number 
 *          list - psu reg list
 *          mode  - the mode for check psu register
 *          reg_name - register name 
 *          reg_offset - register offset
 *          debug - en/disable debug message.
 *  
 * Outputs: PASSED / FAILED
 *  
 **********************************************************************
 */
static int pem_check_data( int ps_num, rp1ruve_pem_reg_t *list, int mode, char* reg_name, uint32_t reg_offset, int debug ) {
    
    int     rv = TEST_PASSED;
    uint16_t data, mask;
    rp1ruve_pem_reg_t *reg_list;
    
    // Get the mask
    mask     = 0;
    reg_list = list;
    while ( reg_list->name ) {
	mask |= ( 1 << reg_list->offset );
	reg_list ++;
    }

    if ( pem_register_read( ps_num, reg_offset, &data ))
	return TEST_FAILED;
        
    if ( debug ) {
	printf(" [0x%08X] = 0x%08X  check_mask = 0x%08X - %s ", 
		 reg_offset, data, mask, reg_name );
    }
    
    if (( mode == ERR_CHECKER_CHECK_THEN_CLEAR_ERRORS ) ||
	( mode == ERR_CHECKER_LEAVE_ERRORS )) {
	if ( data & mask ) {
	    rv = TEST_FAILED;
	    reg_list = list;
	    cterr('f', 0, " PEM#%d [0x%02X] = 0x%08X - %s ", ps_num, reg_offset, data, reg_name );
	    cterr('f', 0, "   check mask = 0x%08X ", mask );
	    while ( reg_list->name ) {
		if ( data & mask & ( 1 << reg_list->offset ))
		    cterr('f', 0,"      BIT(%02d) - %s ", reg_list->offset, reg_list->name );
		reg_list ++;
	    }
    	}
    }
    
    if (( mode == ERR_CHECKER_CHECK_THEN_CLEAR_ERRORS ) || 
	( mode == ERR_CHECKER_CLEAR_ERRORS )) {
	/* just write back to clear */
	if ( rp1ruve_pem_write( ps_num, reg_offset, data ))
	    return TEST_FAILED;
    }
    
    return rv;
}

/**********************************************************************
 *
 * Function: pem_check_core
 *
 * Description: check PEM data
 *
 * Inputs:  ps_num - psu number
 *          mode  - the mode for check psu register
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
static int pem_check_core( uint32_t ps_num, int mode ) {
    
    u_char major, minor;
    int rv = TEST_PASSED;
    int debug = 0;
    //                               name          offset (bit position)
    rp1ruve_pem_reg_t reg_0x5e[] = {{ "Over-current Fault",     1 },
				   { "Over-temperature Fault", 2 },
				   { "Fan Fault",              4 },
				   { 0, 0 }};
    rp1ruve_pem_reg_t reg_0x5f[] = {{ "Heatsink Temp Warning",  2 },
				   { 0, 0 }};
    rp1ruve_pem_reg_t reg_0x60[] = {{ "12V Over-current",     0 },
				   { 0, 0 }};
    rp1ruve_pem_reg_t reg_0x61[] = {{ "Input Over-current",   0 },
				   { "Input Under-voltage",  3 },
				   { 0, 0 }};
    rp1ruve_pem_reg_t reg_0x63[] = {{ "Shutdown events",      6 },
				   { "Warning events",       7 },
				   { 0, 0 }};
    
    if ( pem_get_sw_ver( ps_num, &major, &minor ))
	return TEST_FAILED;

    if (( 5 <= major ) && ( 1 <= minor )) {
	rv |= pem_check_data( ps_num, &reg_0x5e[0], mode, "Reg 0x5E", 0x5e, debug );
	rv |= pem_check_data( ps_num, &reg_0x5f[0], mode, "Reg 0x5F", 0x5f, debug );
	rv |= pem_check_data( ps_num, &reg_0x60[0], mode, "Reg 0x60", 0x60, debug );
	rv |= pem_check_data( ps_num, &reg_0x61[0], mode, "Reg 0x61", 0x61, debug );
	rv |= pem_check_data( ps_num, &reg_0x63[0], mode, "Reg 0x63", 0x63, debug );
    }

    return rv;
}

/**********************************************************************
 *
 * Function: pem_check
 *
 * Description: entry point for check PEM data
 *
 * Inputs:  NONE
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_check( void ) {

    int rv = TEST_PASSED;

    rv |=  pem_check_core( PSU_ONE, ERR_CHECKER_LEAVE_ERRORS );

    return rv;
}

/**********************************************************************
 *
 * Function: pem_clear
 *
 * Description: Clear up error check setting.
 *
 * Inputs: NONE
 *
 * Outputs: PASSED / FAILED
 *
 **********************************************************************
 */
int pem_clear( void ) {

    int rv = TEST_PASSED;    

    rv |=  pem_check_core( PSU_ONE, ERR_CHECKER_CLEAR_ERRORS );

    return rv;
}

/*
 *------------------------------------------------------------------
 * $Log: diag_pem_monitor_rw.c,v $
 * Revision 1.2  2016/04/20 11:25:31  benchen2
 * add tachi fru portion
 *
 * Revision 1.1.2.1  2015/12/23 11:16:13  alpeng
 * support PEM(PSU) utility and its fan utils
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

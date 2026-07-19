/* $Id: platform_pem_monitor_read.c,v 1.1 2020/01/09 01:02:03 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_pem_monitor_read.c,v $
 *------------------------------------------------------------------
 *
 * platform_pem_monitor_read.c: 
 *             Utility to read and display Power Supply monitor,
 *             information, PSU_MODEL, HOUR_ELAPSE, 12V_I, 12V_V,
 *             OUT_TEMP, and INPUT_V.
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
 *------------------------------------------------------------------
 */
#include "platform_pem_utils.h"
#include <stdio.h>
#include "common.h"
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
int pem_monitor_write(int);
int pem_monitor_dump(int);


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

/*
 *-----------------------------------------------------------------------------
$Log: platform_pem_monitor_read.c,v $
Revision 1.1  2020/01/09 01:02:03  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

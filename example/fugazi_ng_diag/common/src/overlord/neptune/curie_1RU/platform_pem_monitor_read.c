/* $Id: platform_pem_monitor_read.c,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_pem_monitor_read.c,v $
 *------------------------------------------------------------------
 *
 * platform_pem_monitor_read.c: 
 *             Utility to read and display Power Supply monitor,
 *             information, PSU_MODEL, HOUR_ELAPSE, 12V_I, 12V_V,
 *             OUT_TEMP, and INPUT_V.
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
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
 *------------------------------------------------------------------
 * $Log: platform_pem_monitor_read.c,v $
 * Revision 1.2  2019/08/06 06:56:14  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.1.2.2  2018/11/14 01:40:29  alpeng
 * remove redundant and useless utilities from power entry module
 *
 * Revision 1.1.2.1  2018/06/22 08:05:19  alpeng
 * move curie diag to neptune/curie_1RU directory
 *
 * Revision 1.1.2.1  2018/05/30 02:39:37  alpeng
 * porting neptune x86 to curie
 *
 * Revision 1.2  2018/05/18 09:25:00  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/02 22:04:02  jskow
 * Move Overlord/x86 specific files to Neptune/x86.
 *
 * Revision 1.3  2013/08/14 06:01:35  alpeng
 * support PSU ucontroller write
 *
 * Revision 1.2  2013/08/13 07:19:30  alpeng
 * support i2c scan on PEM ucontroller, update the code for new PSU eeprom w/r and ucontroller read
 *
 * Revision 1.1  2013/05/31 12:43:15  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

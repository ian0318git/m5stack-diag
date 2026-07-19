/* $Id: o2_dash_fpga_reg_test.c,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_dash_fpga_reg_test.c,v $
 *------------------------------------------------------------------
 * Description: DASH FPGA register test sample with failure case to call cterr
 *
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "python_error.h"
#include "diag_flag_create_from_py.h"
#include "platform_fru.h"

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of executable
 * Inputs     : argc, number of argument
 *              argv, arguments in o2.pcfg and python menu script
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    if (plat_init(argc, argv) == FAILED) {
        return FAILED;
    }

    if (dash_fpga_register_test_fn(argc, argv) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function   : reg_dump()
 * Description: prinf hard-coded register dump value
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void
reg_dump(void)
{
    cterr_db_print("[REG_ADDR1] FPGA id register = 0xabcd\n");
    cterr_db_print("[REG_ADDR2] Board id register = 0xabcd\n");
    cterr_db_print("[REG_ADDR3] Reset register = 0xabcd\n");
    cterr_db_print("[REG_ADDR4] Global Device Reset register = 0xabcd\n");
    cterr_db_print("[REG_ADDR5] LED control register = 0xabcd\n");
    cterr_db_print("[REG_ADDR6] LED Override control register = 0xabcd\n");
}

/*****************************************************************************
 *
 * Function   : env_dump()
 * Description: prinf hard-coded environment dump value
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void
env_dump(void)
{
    cterr_db_print("IN1TEMP:[TEMP1]\n");
    cterr_db_print("IN2TEMP:[TEMP2]\n");
    cterr_db_print("OUT1TEMP:[TEMP3]\n");
    cterr_db_print("OUT2TEMP:[TEMP4]\n");
    cterr_db_print("FAN1STAT:[TEMP5]\n");
    cterr_db_print("FAN1SPD:[FANSPD1]\n");
    cterr_db_print("FAN2STAT:[Enabled]\n");
    cterr_db_print("FAN2SPD:[FANSPD2]\n");
    cterr_db_print("FAN3STAT:[Enabled]\n");
    cterr_db_print("FAN3SPD:[FANSPD3]\n");
    cterr_db_print("FAN4STAT:[Enabled]\n");
    cterr_db_print("FAN4SPD:[FANSPD4]\n");
    cterr_db_print("CPU die temperature is [TEMP6]\n");
    cterr_db_print("PSU1TEMP:[TEMP7]\n");
}

/*****************************************************************************
 *
 * Function   : dash_fpga_register_test_fn
 * Description: fail case example calls cterr
 * Inputs     : argc, number of argument
 *              argv, arguments in o2.pcfg and python menu script
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int dash_fpga_register_test_fn(int argc,char *argv[])
{
    int test_result;
    int last_arg = 0;
    last_arg = atoi(argv[argc-1]);
    uchar mb_get_pid[FRU_SIZE] = {0};
    uchar mb_get_loc[FRU_SIZE] = {0};

    fru_table_offset = MB;
    strcpy(mb_get_pid, "CISCO2800");
    strcpy(mb_get_loc, "MB-DASH_FPGA");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    cterr_add_component("Component B","Component C");
    cterr_add_reg_dump((PFV)reg_dump);
    cterr_add_env_dump((PFV)env_dump);
    cterr_add_debug("Action D","Action E", "Action F");
    /* flag usage example */
    if (warning == 1) {
        printf("\n Warning flag ON \n");
        printf("\n Warning process in o2_dash_fpga_reg_test \n");
    } else {
        printf("\n Warning flag OFF \n");
        printf("\n non Warning process in o2_dash_fpga_reg_test \n");
    }
    testname("DASH_FPGA");
    prpass(testpass, "Sub-test 1 - Hardcoded, ");
    /* ran_num_one_ten() generates a random num from 1 to 10 */
    test_result = ran_num_one_ten();

    if (test_result != PASSED) {
        //this example function will call cterr w/ fatal here!
        cterr('f',0,"Assume FATAL message in DASH_FPGA_register_test\n");
    }

    fru_table_offset = FRU_INFO_INVALID;
    prpass(testpass, "Sub-test 2 - Hardcoded, ");
    test_result = ran_num_one_ten();
    if (test_result != PASSED) {
        //this example function will call cterr w/ warning here!
        cterr('w',0,"Assume WARNING message in FPGA, ");
    }

    prcomplete(testpass, errcount, (char *)0);
    return PASSED;
}

/******** History ********
$Log: o2_dash_fpga_reg_test.c,v $
Revision 1.2  2014/06/03 10:53:31  erwu2
python menu collapsed to main trunk

Revision 1.1.2.4  2014/04/29 11:40:37  erwu2
update python file structure

Revision 1.1.2.3  2014/04/24 08:53:52  erwu2
merge makefile and add flag example to test

Revision 1.1.2.2  2014/04/19 07:14:35  erwu2
modularized makefile updated

Revision 1.1.2.1  2014/04/10 06:24:05  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/

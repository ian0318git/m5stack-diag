/* $Id: o2_vm_test.c,v 1.2 2014/06/03 10:53:32 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_vm_test.c,v $
 *------------------------------------------------------------------
 * Description: hard-coded printing for simulating vm test
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
#include "python_util.h"
#include "o2_util.h"
#include "diag_flag_create_from_py.h"
#include "platform_fru.h"

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
 * Function    : main
 *
 * Description : entry point of executable
 *
 * Inputs      : argc, number of argument
 *               argv, arguments in o2.pcfg and python menu script
 *
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    int ret_val = 0;
    int last_arg = 0;
    last_arg = atoi(argv[argc-1]);
    uchar mb_get_pid[FRU_SIZE] = {0};
    uchar mb_get_loc[FRU_SIZE] = {0};

    fru_table_offset = MB;
    strcpy(mb_get_pid, "CISCO1300");
    strcpy(mb_get_loc, "VM0");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    cterr_add_component("Component A","Component B");
    cterr_add_reg_dump((PFV)reg_dump);
    cterr_add_env_dump((PFV)env_dump);
    cterr_add_debug("Action C","Action D", "Action E");
    if (plat_init(argc, argv) == FAILED) {
        return FAILED;
    }

    if (argc == ARGS_3) {
        /* if vm_test without any args in o2.pcfg, at least 3 args here */
        /* first arg       : executable */
        /* second-last arg : testpass */
        /* last arg        : diag flag value */
        /* last and second-last which came from python script. */

        /* if vm_test has no arg as slot number in o2.pcfg, */
        /* remove slot_id.tmp if it existed */
        rm_slot_tmp();

        cterr('f',0,"FATAL error when getting slot id from VM slot\n");
        return FAILED;

    } else if (argc > ARGS_3){
        /* if vm_test has args in o2.pcfg, take first one as slot number, */
        /* and save it to slot_id.tmp */
        save_slot_num(argc, argv);

        /* pass first arg of vm_test in o2.pcfg as slot number */
        ret_val = vm_test_fn(atoi(argv[1]));

    } else {
        printf("number of arguments came from python are fail!\n");
        return FAILED;
    }

    if (ret_val != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }

}

/*****************************************************************************
 *
 * Function    : read_vm_cookie
 *
 * Description : print hard-coded moduld id to module_id.tmp for python menu
 *               script to load module side menu
 *
 * Inputs      : slot_num - slot number passed from slot_id.tmp
 *
 * Outputs     : vm module cookie id
 *
 *****************************************************************************/
int read_vm_cookie(int slot_num)
{
    printf("reading vm slot %d cookie\n",slot_num);
    printf("(slot vacant)\n");
    return FAILED;
}

/*****************************************************************************
 *
 * Function    : vm_test_fn
 *
 * Description : print hard-coded moduld id to module_id.tmp for python menu
 *               script to load module side menu
 *
 * Inputs      : slot_num - slot number passed from slot_id.tmp
 *
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int
vm_test_fn(int slot_num)
{
    int vm_cookie_val = 0;

    printf("init system to discovery vm slot %d\n",slot_num);

    /* flag usage example */
    if (verbose_mode == 1) {
        printf("\n Verbose mode flag ON \n");
        printf("\n verbose_mode in o2_vm_test \n");
    } else {
        printf("\n Verbose mode flag OFF \n");
        printf("\n non verbose_mode in o2_vm_test \n");
    }

    vm_cookie_val = read_vm_cookie(slot_num);

    if (vm_cookie_val != FAILED ) {
        save_mod_id(vm_cookie_val);
    }
    return PASSED;
}

/******** History ********
$Log: o2_vm_test.c,v $
Revision 1.2  2014/06/03 10:53:32  erwu2
python menu collapsed to main trunk

Revision 1.1.2.3  2014/04/29 11:40:37  erwu2
update python file structure

Revision 1.1.2.2  2014/04/24 08:53:52  erwu2
merge makefile and add flag example to test

Revision 1.1.2.1  2014/04/10 06:24:06  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/

/* $Id: leb_mod_util.c,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_util.c,v $
 *------------------------------------------------------------------
 * Filename:    leb_mod_util.c
 *
 * Description: lebowski module's side common utilities
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"
#include "leb_mod_util.h"
#include "python_util.h"
#include "python_error.h"

/******************************************************************************
 *
 *  Function: mod_init
 *
 *  Description: lebowski module's side initialize common part, it will 
 *               register signal() and call py_parse() to get diag flag value.
 *
 *  Input: argc and argv[] of each test executable
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int mod_init(int argc,char *argv[])
{
    /* register handler for catching ctrl c */
    signal(SIGINT, inthdlr);

    /* call py_parse() in python_error.c */
    if (argc >= ARGS_3) {
        /* at least 3 argc here, first one is executable */
        /* last argument is diag flag value, second-last is testpass */
        /* came from python. */
        if (py_parse(argc, argv) == FAILED) {
            return FAILED;
        }
        return PASSED;
    } else {
        printf("number of arguments came from python are fail!\n");
        return FAILED;
    }
}

/*****************************************************************************
 *
 *  Function: rm_slot_tmp
 *
 *  Description: This function remove slot_id.tmp file if there is no slot
 *               number in $product.pcfg file
 *
 *  Input: None
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int
rm_slot_tmp(void)
{
    FILE *fp;
    fp = fopen(SLOT_ID_PATH, "r");
    if (fp) {
        /* if slot test without arg as slot number in o2.pcfg, */
        /* remove slot_id.tmp if it existed */
        if (remove(SLOT_ID_PATH) != 0) {
            printf("Unable to delete %s\n",SLOT_ID_PATH);
            return FAILED;
        }
    }
    fclose(fp);
    return PASSED;
}

 /*****************************************************************************
 *
 *  Function: save_slot_num
 *
 *  Description: This function save first arg followed the slot test item
 *               as slot number to slot_id.tmp
 *
 *  Input: argc and argv[] of each test executable
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int
save_slot_num(int argc,char *argv[])
{
    FILE *fp;
    fp = fopen(SLOT_ID_PATH, "w");
    if (fp) {
        fprintf(fp,"%s",argv[1]);
    } else {
        printf("open %s fail!\n",SLOT_ID_PATH);
        return FAILED;
    }
    fclose(fp);
    return PASSED;
}

 /*****************************************************************************
 *
 *  Function: save_mod_id
 *
 *  Description: This function save module id to module_id.tmp
 *
 *  Input: module id
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int
save_mod_id(int mod_id)
{
    FILE *fp;
    fp = fopen(MOD_ID_PATH, "w");
    if (fp) {
        fprintf(fp,"0x%x",mod_id);
    } else {
        printf("open %s file fail!\n",MOD_ID_PATH);
        return FAILED;
    }
    fclose(fp);
    return PASSED;
}


/******** History ********
$Log: leb_mod_util.c,v $
Revision 1.2  2014/06/03 10:53:30  erwu2
python menu collapsed to main trunk

Revision 1.1.2.1  2014/04/29 11:40:37  erwu2
update python file structure


$Endlog$
*/

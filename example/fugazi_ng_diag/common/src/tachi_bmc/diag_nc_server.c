/* $Id: diag_nc_server.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_server.c,v $
 *------------------------------------------------------------------
 *
 * diag_nc_server.c
 * CSX-Tachi nc server entry  
 *
 * Nov 2015, Alan Peng
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "diag_nc_common.h"
#include "ngio.h"
#include "types.h"
#include "diag_fpga_lib.h"
#include "types.h"
#include "slot.h"

static int diag_cmdline_uname_a(struct nc_args *);
static int diag_nc_is_nim_present(struct nc_args *);
static int diag_nc_get_nim_ctype(struct nc_args *);
static int diag_nc_read_fpga_reg(struct nc_args *);
static int diag_nc_set_fpga_reg(struct nc_args *);
static int diag_nc_reset_device(struct nc_args *);
static int diag_nc_get_board_type(struct nc_args *);

struct nc_command nc_cmd_items[] = {
    {DIAG_COMMAND_UNAME_DISPLAY, diag_cmdline_uname_a},
    {DIAG_COMMAND_UNAME_DISPLAY, diag_cmdline_uname_a},
    {DIAG_COMMAND_IS_NIM_PRESENT, diag_nc_is_nim_present},
    {DIAG_COMMAND_GET_NIM_CTYPE, diag_nc_get_nim_ctype},
    {DIAG_COMMAND_READ_FPGA_REG, diag_nc_read_fpga_reg},
    {DIAG_COMMAND_SET_FPGA_REG, diag_nc_set_fpga_reg},
    {DIAG_COMMAND_RESET_DEVICE, diag_nc_reset_device},
    {DIAG_COMMAND_GET_BOARD_TYPE, diag_nc_get_board_type},
    {DIAG_COMMAND_MAX_ITEM,      NULL},
};

#define DIAG_NC_MAX_ITEM sizeof(nc_cmd_items)/sizeof(struct nc_command)


/**********************************************************************
 *
 * Function: diag_cmdline_uname_a
 *
 * Description: sample function 
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_cmdline_uname_a (struct nc_args *args) {

   struct nc_args *tmp; 
   char cmd[32];
   sprintf(cmd, "%s", "uname -a ");

   /* example of get arguments */
   tmp = args;
   while (tmp->next != NULL) {

      printf("%s\n", tmp->arg);
      tmp = tmp->next; 
   }
    
   return (system(cmd));
}

/*******************************************************************************
 *
 * Function   : diag_nc_is_nim_present
 *
 * Description: This function provides information whether NIM is detected or not.
 *
 * Inputs     : slot_no - Slot number (0, 1)
 *
 * Outputs    : 1 - Present, 0 - Not present
 *
 *******************************************************************************
 */
static int diag_nc_is_nim_present(struct nc_args *args)
{
    struct nc_args *tmp;
    struct ngio_intf_t *ngio;

    int slot_num;
    tmp = args;
    int ix;
    for (ix=0;ix<1;ix++)
    {
        slot_num = (int)tmp->arg;
        printf("%s\n", tmp->arg);

        tmp = tmp->next;
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot_num);
    sprintf(args->arg, "%d", ngio->is_present((void *)ngio));
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_nc_get_nim_ctype
 *
 * Description: This function returns contoller type of NIM
 *
 * Inputs     : slot_no - Slot number (0, 1)
 *
 * Outputs    : FFFF - Controller Type not available otherwise returns controller type
 *
 *******************************************************************************
 */
static int diag_nc_get_nim_ctype(struct nc_args *args)
{
    struct ngio_intf_t *ngio;
    int status = PASSED;
    char err[80];

    struct nc_args *tmp;
    tmp = args;
    int ix, slot_num;
    for (ix=0;ix<1;ix++)
    {
        slot_num = (int)tmp->arg;
        printf("%s\n", tmp->arg);
        tmp = tmp->next;
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot_num);
    if (!ngio->is_present((void *)ngio)) {
        sprintf(args->arg, "%x", 0xffff);
    }

    if ((status = ngio->get_id((void *)ngio, err))==FAILED) {
        printf("can't get the controller type!\n");
        return (FAILED);
    }

    sprintf(args->arg, "%x", ngio->id);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_nc_read_fpga_reg
 *
 * Description: Read FPGA Register function
 *
 * Inputs     : offset - FPGA register offset (hexadecimal, e.g. A5A5)
 *
 * Outputs    : FPGA Register value (hexadecimal, e.g. 5F5FAFAF)
 *
 *******************************************************************************
 */
static int diag_nc_read_fpga_reg(struct nc_args *args)
{
    int retval = FAILED;
    int value;

    struct nc_args *tmp;
    tmp = args;
    int ix, offset;
    for (ix=0;ix<1;ix++)
    {
        offset = (int)tmp->arg;
        printf("%s\n", tmp->arg);
        tmp = tmp->next;
    }

    if (diag_fpga_reg_read(offset, &value) == PASSED) {
        retval = PASSED;
        sprintf(args->arg, "%x", value);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : diag_nc_set_fpga_reg
 *
 * Description: Write FPGA Register function
 *
 * Inputs     : offset - FPGA register offset (hexadecimal, e.g. A5A5)
 *
 * Outputs    : 0 - PASS, non-zero - FAIL
 *
 *******************************************************************************
 */
static int diag_nc_set_fpga_reg(struct nc_args *args)
{
    int retval = FAILED;
    struct nc_args *tmp;
    tmp = args;
    int ix, offset,value;
    for (ix=0;ix<2;ix++)
    {
        printf("%s\n", tmp->arg);
        if(ix ==0) {
            offset = (int)tmp->arg;
        } else {
            value = (int)tmp->arg;
        }
        tmp = tmp->next;
    }

    if (diag_fpga_reg_write(offset, value) == PASSED) {
       retval = PASSED;
    }
    return (retval);
}


/*******************************************************************************
 *
 * Function   : diag_nc_reset_device
 *
 * Description: Reset and unreset device
 *
 * Inputs     : device - 0=Lewis
 *
 * Outputs    : 0 - PASS, non-zero - FAIL
 *
 *******************************************************************************
 */
static int diag_nc_reset_device(struct nc_args *args)
{
    int retval = FAILED;

    struct nc_args *tmp;
    tmp = args;
    int ix, device;
    for (ix=0;ix<1;ix++)
    {
        device = (int)tmp->arg;
        printf("%s\n", tmp->arg);
        tmp = tmp->next;
    }

    if (device == 0) {
        if(diag_fpga_reg_write(FPGA_EXT_RESET_REG, FPGA_CETUS_RESET) == PASSED) {
            retval = PASSED;
        }
    }

    return (retval);
}


/*******************************************************************************
 *
 * Function   : diag_nc_get_board_type
 *
 * Description: return BMC board type, in case we need to distinguish Tachi-L or Tachi-H
 *                   such as Victory platforms.
 *
 * Inputs     : NONE
 *
 * Outputs    : 0 - Tachi Low, 1 - Tachi High, 2 - Tachi Ultra High
 *
 *******************************************************************************
 */

static int diag_nc_get_board_type(struct nc_args *args)
{
    int retval;
    FILE *stream;
    int num;

    if(system("cat /tmp/sku > sku.txt")) {
        printf("Please program FRU first !\n");
    }
    stream = fopen("sku.txt", "r");
    if (stream == NULL) {
        printf("open file failed!");
        fclose(stream);
    } else {
        fscanf(stream, "%d", &num);
        /*5 represent tachi-l product*/
        if( num == 5) {
            printf("Tachi-l product!\n");
            retval =0;
            sprintf(args->arg, "%d", retval);
        }
        fclose(stream);
    }

    return (PASSED);
}



/**********************************************************************
 *
 * Function: diag_nc_server_dispatch_comm
 *
 * Description: nc server dispatch command 
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int diag_nc_server_dispatch_comm (void) {

    int ix, rc; 
    unsigned cli_sub; 
    struct nc_args *arg = (struct nc_args*) malloc(sizeof(struct nc_args));
    char cli_cmd[32]; 

    cli_sub = diag_nc_get_dispatch_comm(ALL_SUB);
    arg = diag_get_parms_frm_host();
    strcpy(cli_cmd, arg->arg);

    for (ix = 0; ix < DIAG_NC_MAX_ITEM; ix++) {
        if (!strcmp(nc_cmd_items[ix].cmd_str, cli_cmd)) {
            rc = nc_cmd_items[ix].func(arg);
            break; 
        }
    }

    if (ix == DIAG_NC_MAX_ITEM) {
        printf("Did not hit command-%s on table\n", arg->arg); 
        rc = FAILED; 
    }

    /* arg->arg is return information or cmd name */
    if (rc == PASSED) { 
        diag_return_parms_to_host(cli_sub, "PASS", arg->arg); 
    } else {
        diag_return_parms_to_host(cli_sub, "FAIL", arg->arg); 
    }

#if 0  /* sub-system will keep server listen port available */   
    /* listen for next request */
    nc_init_listen_port();
#endif 
    return (rc);

}

/*---------------------------------------------------------------
$Log: diag_nc_server.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.3  2015/12/01 02:04:36  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.2  2015/11/25 06:12:12  benchen2
add bmc nc comm portion

Revision 1.1.2.1  2015/11/24 12:14:30  alpeng
add nc infrastructure


$Endlog$
*/


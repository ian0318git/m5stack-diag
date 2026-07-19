/* $Id: cli_cmd.c,v 1.30 2018/02/09 09:11:18 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/cli_cmd.c,v $
 ***********************************************************************
 *
 * cli_cmd.c - new CLI command for manufacturing.
 *
 * Mar 2008 - steja
 *
 * Copyright (c) 2009-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Scott Tsai 
 ***********************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "setjmps.h"
#include "monitor.h"
#include "nvsysvars.h"
#include "nmc93c46.h"
#include "cross_platform.h"
#include "cookie_plat.h"
#include "mon_plat_defs.h"
#include "error.h"
#include "dev_print.h"
#include "proto.h"
#include "pcmap.h"
#include "pm_utils.h"
#include "platform_smi.h"
#include "dev_at24c0n.h"
#include "dev_object.h"
#include "cli_cmd.h"
#include "smart_cookie.h"
#include "cookie_4.h"
#include "slot.h"
#include "ngio.h"
#include "act2_utils.h" /* act2_init_cont() */
#include "platform_slot.h"
#include "platform_cookie.h"
#include "platform_dimm.h"
#include "platform_i2c.h"
#include "platform_margin_utils.h"

#define CLIDBG 0

/* Extern functions */
extern int netflashbooted;
extern int async_select;
extern cookie_4_table cookie_4_info[];
extern COOKIE_4 *buffer_search_x (boolean, int);
extern COOKIE_4 *get_new_buf (void);
extern void fetch_user_input_data (char *, COOKIE_4 *);
extern void cookie_4_enque (COOKIE_4 *, COOKIE_4 *);
extern COOKIE_4 *cookie_root;
//extern uchar get_max_sm_slots (void);
extern boolean is_ism_present (void);
extern int smart_cookie_program_dig_sign_submenu_x(sc_context *, boolean);
extern submenu_xtable_t main_menu_table[];
extern ushort get_grwic_cookie_id(int, void *, uchar);
extern int linux_set_rtc(int, cli_time *);
extern int utility_get_rtc(int);
extern char atoh(char c);
extern void act2_init_cont(void *);
extern int is_act2(void);
extern int act2_prog(boolean);
extern int is_usd_machines(void);

static int cli_set_time(int,  cli_time *);
static int checkdate(cli_time *);
static int checktime(cli_time *);
static int cli_cookie_wicdclevel (cli_cookie_cmd *);
static int cli_cookie_smdclevel (cli_cookie_cmd *);
static dev_if_info_t dev_if;
static void cli_discovery_mb(void);
static int do_cli_discovery_mb(cli_cookie_cmd *);
static void cli_discovery_mb_wic(void);
static int do_cli_discovery_mb_wic(cli_cookie_cmd *);
static void cli_discovery_mb_vm(void);
static int do_cli_discovery_mb_vm(cli_cookie_cmd *);
static void cli_discovery_sm(void);
static int do_cli_discovery_sm(cli_cookie_cmd *);

static int support_cp_memsize(void);
static int support_dp_memsize(void);
static int cli_support_vm(void);
static int cli_support_cf(void) ;
static int cli_support_psu(void);
static int cli_support_sm(void);
static int cli_support_mb_wic(void);
#ifdef NOT_USED
static int do_nm_nlevel(cli_cookie_cmd *);
static int do_sm_nlevel(cli_cookie_cmd *);
#endif
static void cli_discovery_psu(void);
static void cli_discovery_mem(void);
static int cli_execgrp_diags (int);
static void cli_discovery_cf(void);
static void cli_cookie_no_ism_msg(void);
extern uint ge_phy_test_flags[MB_SMI_INVALID];
int do_cli_cookie_switches(cli_cookie_cmd *);
int cli_cookie_strncmp (char *, cli_cookie_cmd *);

/*************************************************************************
 Function: cli_power_of_digit
 *
 * This function is to power the digit depend type (HEX or DEC)
 *
 * Input: int len
 *        int type
 *       
 * Output: digit
 **************************************************************************
*/
static int 
cli_power_of_digit(int len, int type)
{
    int digit = 1;

    while (--len) {
        digit *= type;
    }

    return digit;
}

/*************************************************************************
 Function: cli_str2num
 *
 * This function is to convert str to number(HEX,DEC)
 *
 * Input: char *str
 *        int type
 *       
 * Output: rc
 **************************************************************************
*/ 
static int 
cli_str2num(char *str, int type)
{
    int rc = 0;
    int len = 0;
    char *ptr = NULL;

    len = strlen(str);
  
    for (ptr = str; *ptr != '\0'; ptr++)
    {
        rc += (atoh(*ptr) * cli_power_of_digit(len, type));
        len--;
    }

    return rc;
}

/*************************************************************************
 Function: cli_get_keyword
 *
 * This function is to get index by compare the str with keyword
 *
 * Input: char *str
 *        char *keyword
 *       
 * Output: index / -1
 **************************************************************************
*/  
static int 
cli_get_keyword(char *str, char **keyword)
{
    int index = 0;

    if (str == NULL) {
        return CLI_UNKNOWN_INDEX;
    }

    while (*keyword != NULL) {
        if (strncmp(str, *keyword, strlen(*keyword)) == 0) {
            if ((strlen(str) == strlen(*keyword)) || 
                (**keyword == '-')) {
                return index;
            }
        }

        index++;
        keyword++;
    }

    return CLI_UNKNOWN_INDEX;
}

/*************************************************************************
 Function: cli_print_node
 *
 * This function is debug by print out node of link list
 *
 * Input: cli_token head
 *        
 * Output: printf
 **************************************************************************
*/ 
#ifdef CLI_DEBUG 
static void 
cli_print_node(cli_token head)
{
    cli_value *ptr;
    int count = 0;

    for (ptr = head.vlist; ptr != NULL; ptr = ptr->next, count++) {
        printf("node %d = %s\n", count, ptr->value);
    }
}
#endif
/*************************************************************************
 Function: cli_insert_node
 *
 * This function is created linked list 
 *
 * Input: cli_token *head
 *        char *value
 *        
 * Output: NONE
 **************************************************************************
*/  
static void 
cli_insert_node(cli_token *head, char *value)
{
    cli_value *new;
    cli_value *ptr;

    for (ptr = head->vlist; ptr != NULL; ptr = ptr->next) {
        if (ptr->next == NULL)
            break;
    }

    new = (cli_value *)malloc(sizeof(cli_value));

    memset(new->value, 0, sizeof(new->value));
    
    strcpy(new->value, value);

    if (head->vlist != NULL) {
        new->next = ptr->next;
        ptr->next = new;
        head->count++;
    } else {
        head->vlist = new;
        new->next = NULL;
        head->count = 1;
    }
}

/*************************************************************************
 Function: cli_del_node
 *
 * This function is to set the node back to the head of the list
 *
 * Input: cli_token *head
 *        
 * Output: NONE
 **************************************************************************
*/    
static void 
cli_del_node(cli_token *head)
{
    cli_value *ptr;
    cli_value *next_ptr;
    
    ptr = head->vlist;
    while (ptr != NULL) {
        next_ptr = ptr->next;
        free(ptr);
        ptr = next_ptr;
    }

}

/*************************************************************************
 Function: cli_parse_token
 *
 * This function is to parse token and add it in the node linked list
 *
 * Input: char **str
 *        cli_token *tlist
 *        
 * Output: NONE
 **************************************************************************
*/    
static void 
cli_parse_token(char **str, cli_token *tlist)
{
    char *ptr = NULL;

    for (ptr = *str; *ptr != '\0'; ptr++) {
        if (*ptr == tlist->token) {
            *ptr = '\0';
            cli_insert_node(tlist, *str);
            *str = ++ptr;
        }
    }

    cli_insert_node(tlist, *str);
}

/*************************************************************************
 Function: cli_check_option
 *
 * This function is check option for setflag
 *
 * Input: char *str
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static int 
cli_check_option (char *str)
{   
    /*
    * str[0]
    * for example : *str = "A\0" or *str = "ACDE\0"
    */
    while (*str != '\0') {
        switch (*str) {
            case 'A':
            case 'C':
            case 'D':
            case 'E':
            case 'L':
            case 'M':
            case 'O':
            case 'S':
            case 'T':
            case 'V':
            case 'W':
            case 'X':
            case 'U':
            case '-':
            case 'H':
            case 'N':
                break;
            default:
                return (FAILED);
        }
        str++;
    }

    return (PASSED);
}

/*************************************************************************
 Function: cli_change_cookie
 *
 * This function is to search the cookie field and change field value
 * 
 * Input: uchar *str
 *        int field
 *        
 * Output: PASSED / FAILED 
 **************************************************************************
*/
int 
cli_change_cookie(int field, char *str, cli_cookie_cmd *cmd)
{
    COOKIE_4 *buf;
    cookie_4_table *cp = cookie_4_info;

     /* make sure the type been supported */
    if ((cp = search_type_4_table(field)) == (cookie_4_table *)NULL)
        return (FAILED);

    if ((buf = buffer_search_x(TRUE, field)) == (COOKIE_4 *)NULL){
    /* Add Cookie Field if it doesn't exist in the buf pool*/
    /* accept new input type afterward */
        if ((buf = get_new_buf()) == (COOKIE_4 *)NULL){
            printf("Index out of COOKIE_4 buffer pool!!\n");
            return (FAILED);
        }

        buf->p_info = cp;       /* pointer in cookie_4_table */
        buf->type = field;
        cmd->type = CLI_COOKIE_ADD;
        /* Don't need user interaction */
        /*prompt_for_display_format(buf);*/

    }

    fetch_user_input_data(str, buf);

    if (cmd->type == CLI_COOKIE_ADD ){
        cookie_4_enque(cookie_root, buf);
    }

    return (PASSED);
}

/*-----------------------------------------------------------------------
 *
 * CLI_set_time()
 *
 * This function to check RTC type present and set the time.
 *
 * Input : set_type 1 = Time
 *         set_type 0 = Date
 *         struct time value
 *
 * Output: none
 *
 *-------------------------------------------------------------------------
 */
static int 
cli_set_time (int set_type, cli_time *timeval)
{
	  int rc;
#ifndef OVERLORD
    
    ds1337_time_t tv1337;
    /* Get rtc value before changes*/
    get_rtc(&tv1337);
    
    /* Changed the rtc value depends on set type */
    if(set_type == CLI_SET_DATE){
        tv1337.year = timeval->year;
        tv1337.month = timeval->month;
        tv1337.date = timeval->date;
    }else if (set_type == CLI_SET_TIME) {
        tv1337.hour = timeval->hour;
        tv1337.minute = timeval->minute;
        tv1337.second = timeval->second; 
    }
    /* Set the rtc value to the ds1337 through platform_rtc.c */
    rc = set_rtc(&tv1337);
        
    
#else
    
    rc = linux_set_rtc(set_type, timeval);
    
#endif
    return (rc);
}

/*************************************************************************
 Function: cli_free_mem
 *
 * This function is to free mem that CLI Allocate 
 *
 * Input: cli_cookie_cmd *cmd
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static void 
cli_free_mem(cli_cookie_cmd *cmd)
{
    /* free memory */
    if (cmd->contents != NULL) {
        free(cmd->contents);
        cmd->contents = NULL;
    }
}

/*************************************************************************
 Function: cli_init_cookie_vars
 *
 * This function is to initialize cookie variables.
 *
 * Input: int index
 *        char *str
 *        cli_cookie_cmd *cmd
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static int 
cli_init_cookie_vars(int index, char *str, cli_cookie_cmd *cmd)
{   
    cli_token temp;
    cli_value *ptr;
    int slot = 0;
    int max_slot = 0;
    int min_slot = 0;
    int max_token = 0;
    int i, value = 0;
    cmd->dc_type = 0;

    /* index is keyword of MB / ISM / VM etc
       init the cli cookie cmd structure
     */
    switch (index) {
        case CLI_COOKIE_MB: /* MB */
            cmd->board_type = MOTHER_BOARD;
            cmd->size = COOKIE_SIZE_512;
            return (PASSED);
        case CLI_COOKIE_BP: /* BP */
            cmd->board_type = MOTHER_BOARD;
            cmd->slot = 0;
            cmd->size = COOKIE_SIZE_512;
            return (PASSED);
        case CLI_COOKIE_VM: /* VM */
            cmd->board_type = VM_MODULE;
            cmd->size = COOKIE_SIZE_512;
            min_slot = FIRST_SLOT;
            max_slot = get_max_num_vm();   
             /*zzzz  without set token the test 
             will failed right after cli_parse_token() function,  
             hard code to 2, ref NMPVDM not sure it's right or not */
            max_token = 2;
            break;
        case CLI_COOKIE_SM: /* SM */
            cmd->board_type = SM_MODULE;
            cmd->size = COOKIE_SIZE_512;
            max_token = 1;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_SMDC: /* SM daughtercard */
            cmd->board_type = SM_MODULE;
            cmd->size = COOKIE_SIZE_512;
            cmd->dc_type = CLI_COOKIE_SMDC;
            max_token = 2;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_WIC: /* WIC */
            cmd->board_type = WIC_MODULE;
            cmd->size = COOKIE_SIZE_512;
            max_token = 1;
            min_slot = FIRST_SLOT;
            max_slot = get_max_wic_slots();
            break;
        case CLI_COOKIE_NM: /* NM */
            cmd->board_type = NETWK_MODULE;
            cmd->size = COOKIE_SIZE_512;
            max_token = 1;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_NMEHWIC: /* NM 2nd level */
        case CLI_COOKIE_NMDC:
        case CLI_COOKIE_NMDCGE:
        case CLI_COOKIE_NMDCPWR:
        case CLI_COOKIE_NMEM:
            cmd->board_type = NETWK_MODULE;
            cmd->size = COOKIE_SIZE_512;
            switch (index){
                case CLI_COOKIE_NMEHWIC: /* NM 2nd level */
                    cmd->dc_type = CLI_COOKIE_NMEHWIC;
                    break;
                case CLI_COOKIE_NMDC:
                    cmd->dc_type = CLI_COOKIE_NMDC;
                    break;
                case CLI_COOKIE_NMDCGE:
                    cmd->dc_type = CLI_COOKIE_NMDCGE;
                    break;
                case CLI_COOKIE_NMDCPWR:
                    cmd->dc_type = CLI_COOKIE_NMDCPWR;
                    break;
                case CLI_COOKIE_NMEM:
                    cmd->dc_type = CLI_COOKIE_NMEM;
                    break;
                default:
                    break;
            }
            max_token = 2;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_NMEHWICECAN: /* NM 3rd level */
            cmd->board_type = NETWK_MODULE;
            cmd->size = COOKIE_SIZE_512;
            cmd->dc_type = CLI_COOKIE_NMEHWICECAN;
            max_token = 3;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_NMPVDM: /* NM-4th*/
            cmd->board_type = NETWK_MODULE;
            cmd->size = COOKIE_SIZE_512;
            cmd->dc_type = CLI_COOKIE_NMPVDM; /*NM-4th */
            max_token = 2;
            min_slot = FIRST_SLOT;
            max_slot = get_max_sm_slots();
            break;
        case CLI_COOKIE_PSU: /* PSU */
            cmd->board_type = PSU_MODULE;
            cmd->size = AT24C02_MAX + 1; // 256 bytes
            max_token = 1;
            min_slot = 1;
            max_slot = 2;
            break;
        case CLI_COOKIE_MP: /* Mid Plane */
            /* Platform those support Mid Plane */
            printf("This Platform doesn't support Midplane\n");
            return (FAILED);

        case CLI_COOKIE_ISM: /* ISM */
            printf("\nPlatform doesn't support ISM\n");
            return (FAILED);

        case CLI_COOKIE_WICDC: /* WIC daughtercard */
        case CLI_COOKIE_EHWICECAN: /* EHWIC ECAN */
            cmd->board_type = WIC_MODULE;
            cmd->size = COOKIE_SIZE_512;
            switch (index) {
                case CLI_COOKIE_WICDC:
                    cmd->dc_type = CLI_COOKIE_WICDC;
                    break;
                case CLI_COOKIE_EHWICECAN:
                    cmd->dc_type = CLI_COOKIE_EHWICECAN;
                    break;
                default:
                    break;
            }
            max_token = 2;
            min_slot = FIRST_SLOT;
            max_slot = get_max_wic_slots();
            break;
        default:
            return (FAILED);
    }

    /* initialize */
    temp.vlist = NULL;
    temp.token = ':';
    /* ie. cookie NM:2ND 2:1 (max_token = 2)
           cookie NM:2ND:3RD 2:1:0 (max_token = 3)
    */

    cli_parse_token(&str, &temp);
    if (temp.count > max_token) {
        return (FAILED);
    }

    /* ie. NM:2ND <1ST level slot>:<2ND level slot>
           NM:2ND:3RD <1ST level slot>:<2ND level slot>:<3RD level slot>
    */
    
    for(ptr = temp.vlist, i = 0; ptr != NULL; ptr = ptr->next, i++) {
        value = cli_str2num(ptr->value, CLI_DEC);
        switch (i) {
            case firstlevel: /* 1ST level */
                slot = value;
                if ((slot > max_slot) || (slot < min_slot)) {
                    printf("\nMax_slot = %x, Min_slot = %x \n",max_slot, 
                        min_slot );
                    cli_del_node(&temp);
                    return (FAILED);
                }
                break;
            case secondlevel:  /* 2ND level */
                cmd->dc_slot = value;
                break;
            case thirdlevel:  /* 3RD level */
                cmd->dcdc_slot = value;
            default:
                break;
        }
    }
    
    /* To Trace CLI  */
    if (diagflag_xram & D_TRACE) {
    	  printf("\n%s,%s,%d\n",__FILE__, __func__, __LINE__);
        printf("1st_level slot = %x\n", slot); /* 1st level */
        printf("2nd_level slot = %x\n", cmd->dc_slot); /* 2nd level */
        printf("3rd_level slot = %x\n", cmd->dcdc_slot); /* 3rd level */
    }
    /* init the cli cookie cmd slot after parser the slot value */
    switch (cmd->board_type) {
        case VM_MODULE: /* VM */
            cmd->slot = slot;
            break;
       case SM_MODULE: /* SM */
            cmd->slot = slot;
            break;
        case NETWK_MODULE: /* NM */ 
            cmd->slot = slot;
            break;
        case WIC_MODULE: /* WIC */
            cmd->slot = slot;
            break;
        case PSU_MODULE: /* PSU */
            /* check the psu present */
            if(has_ps2() && (slot == 2)){
                cmd->slot = TWOPSU;  /* have two psu */
            }else 
            if(has_ps1() && (slot == 1)) {
                cmd->slot = ONEPSU;  /* has one psu */
            }else {
                printf("\nNo PSU cookie supported in this platform\n");
                return(FAILED);
            }
            break;
        case ISM_MODULE: /* ISM */
            break;
        default:
            return (FAILED);
    }
    /* delete the node */
    cli_del_node(&temp);
    return (PASSED);
}

/*************************************************************************
 Function: cli_get_auth_target
 *
 * This function is to initialize variable target for auth and prog
 *
 * Input: int index
 *        char *str
 *        cli_cookie_cmd *cmd
 *        sc_context *con 
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static int 
cli_get_auth_target(int index, char *str, cli_cookie_cmd *cmd,
                        sc_context *con)
{   
    cli_token temp;
    cli_value *ptr;
    /* int slot = 0, main_slot = 0; */
    int slot = 0;
    int max_token = 0;
    /* int value = 0; */
    struct ngio_intf_t *ngio;
    
    switch (index) {
        case CLI_AUTH_MB: /* MB */
            cmd->board_type = MOTHER_BOARD;
            cmd->slot = 0;
            return (PASSED);
        case CLI_AUTH_VM: /* VM */
            cmd->board_type = VM_MODULE;
             max_token = 1;
            break;
        case CLI_AUTH_SM: /* SM */
            cmd->board_type = SM_MODULE;
            max_token = 1;
            break;
        case CLI_AUTH_SMDC: /* SM DC */
            cmd->board_type = SM_DAUGHTER_CARD;
            max_token = 2;
            break;
        case CLI_AUTH_WIC: /* WIC */
            cmd->board_type = WIC_MODULE;
            max_token = 1;
            break;
        case CLI_AUTH_WICDC: /* WIC DC */
            cmd->board_type = WIC_DAUGHTER_CARD;
            max_token = 2;
            break;
        case CLI_AUTH_NM: /* NM */
            cmd->board_type = NETWK_MODULE;
            max_token = 1;
            break;
        case CLI_AUTH_ISM: /* ISM */
            cmd->board_type = ISM_MODULE;
            max_token = 1;
            break;
        default:
            return (FAILED);
    }
    
    /* initialize */
    temp.vlist = NULL;
    temp.token = ':';

    /* ie. cookie NM:2ND 2:1 (max_token = 2)
           cookie NM:2ND:3RD 2:1:0 (max_token = 3)
    */
    cli_parse_token(&str, &temp);
    if (temp.count > max_token) {
        return (FAILED);
    }

    /* get NGIO slot */
    ptr = temp.vlist;
    /* main_slot = cli_str2num(ptr->value, CLI_DEC); */
    slot = cli_str2num(ptr->value, CLI_DEC);

    /* To Trace CLI  */
    if (diagflag_xram & D_TRACE) {
    	  printf("\n%s,%s,%d\n",__FILE__, __func__, __LINE__);
        printf("slot = %x\n", slot);
    }
    
    switch (cmd->board_type) {
    case VM_MODULE: /* VM */
        assert(slot);
        ngio = (struct ngio_intf_t *)slot_get_ngiovm(slot);
        assert(ngio);
        if (!ngiovm_present(ngio)) {
            cterr('f', 0, "No VM in slot.");
            return FAILED;
        }
        if ((ngiovm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset VM module.\n");
            return FAILED;
        }

        cmd->slot = slot;
        break;

    case SM_MODULE: /* SM */
    case SM_DAUGHTER_CARD: /* SM DC */
        assert(slot);
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
        assert(ngio);
        if (!ngiosm_present((void *)ngio)) {
            cterr('f', 0, "No Service Module in slot %d.", slot);
            return FAILED;
        }
        if ((ngiosm_enable((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to power SM module slot %d", slot);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to unreset SM module slot %d", slot);
            return FAILED;
        }
        
        cmd->slot = slot;
        break;

    case WIC_MODULE: /* WIC */
    case WIC_DAUGHTER_CARD: /* WIC DC */
        assert(slot);
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        assert(ngio);
        if (!ngiowic_present(ngio)) {
            printf("\nNo WIC in slot %d.\n", slot);
            return FAILED;
        }
        if ((ngiowic_enable(ngio)) < 0) {
            printf("Unable to power WIC module slot %d\n", slot);
            return FAILED;
        }
        if ((ngiowic_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset WIC  module slot %d\n", slot);
            return FAILED;
        }

        cmd->slot = slot;
        break;

    case NETWK_MODULE: /* NM */
    case ISM_MODULE: /* ISM */
    default:
        assert(!"NETWK_MODULE/ISM_MODULE not supported");
        return (FAILED);
    }
    
    cli_del_node(&temp);
    return (PASSED);
}

/*************************************************************************
 Function: cli_get_field
 *
 * This function is to get cookie field or 3rd keyword for cookie size
 *
 * Input: int index
 *        char *str
 *        cli_cookie_cmd *cmd
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static int 
cli_get_field(int index, char *str, cli_cookie_cmd *cmd)
{
    cli_token temp;
    cli_value *ptr;
    int value, mode;
    uchar *vptr;
    cookie_4_table *cookie_ptr = cookie_4_info;
    char *keyword_3rd[] = {"-RAW", "-FMT", NULL};

    /* to avoid index is unknown index if it is then return Failed */
    if (index == CLI_UNKNOWN_INDEX){
        return (FAILED);
    }
#if CLIDBG
    printf("\nDBG::Current Source line number: %d", __LINE__);
    printf("\nSource File: %s, Function Name: %s", __FILE__, __func__);
    printf("\nindex: %d",index);
#endif
    /* if the type is cookie change then init the cli cmd field 
       then return 
    */
    if(cmd->type == CLI_COOKIE_CHANGE){
        cmd->field = cookie_ptr[index].type;
        return (PASSED);
    }
    /* to avoid if the cli cmd type cookie is change by field not RAW 
       or display RAW  or display FMT
     */
    if (cmd->type == CLI_COOKIE_CHANGE) {
        return (FAILED);
    } else {
        /* get 3rd level keyword, 
        display content of cookie in RAW or FMT */
#if CLIDBG
        /* cookie size 128, 256, 512*/
        printf("\ncmd->size:%d", cmd->size);
#endif
        mode = cli_get_keyword(str, keyword_3rd);
        /* if the mode is RAW or FMT display cookie, 
           or default is write RAW cookie 
        */
        switch (mode) {
            case CLI_RAW: /* RAW */
                cmd->type = CLI_COOKIE_DISPLAY_RAW;
                break;
            case CLI_FMT: /* English */
                cmd->type = CLI_COOKIE_DISPLAY_FMT;
                break;
            default: /* <value> */
                /* initialize */
                temp.vlist = NULL;
                temp.token = ',';
                cli_parse_token(&str, &temp);
   
            if (temp.count > cmd->size) {
                printf("the number of bytes to fill in is more "
                   "than the size specified\n");
                cli_del_node(&temp);
                return (FAILED);
            }

            /* allocate memory to store cookie contents */
            /* double check if it is already allocate the memory */
            if (cmd->contents == NULL) {
                printf("Memory is not enough.\n");
                return (FAILED);
            }
#if CLIDBG
            printf("\n%s(),Line %d", __func__, __LINE__);
            dismem((uchar *)cmd->contents,cmd->size, (ulong)cmd->contents,1);
#endif
            /* set with 0xFF as default */
            memset(cmd->contents, 0xFF, cmd->size);
#if CLIDBG
            printf("\n%s(),Line %d", __func__, __LINE__);
            dismem((uchar *)cmd->contents,cmd->size, (ulong)cmd->contents,1);
#endif
            /* fill the the contents based on the input data.
               ie. cookie MB -S128 "04,ff,01,45" 
               "04,ff,01,45" is input data
             */
            for (ptr = temp.vlist, vptr = cmd->contents; 
                ptr != NULL; ptr = ptr->next, vptr++) {
                    value = cli_str2num(ptr->value, CLI_HEX);
                    if (value < 0 || value > 0xFF) {
                        cli_del_node(&temp);
                        return (FAILED);
                    }
                    *vptr = value;
            }
#if CLIDBG
                printf("\n%s(),Line %d", __func__, __LINE__);
                dismem((uchar *)cmd->contents,cmd->size, (ulong)cmd->contents,1);
#endif
            /* delete the node */
            cli_del_node(&temp);
            /* change the cmd type to fill only the RAW data */
            cmd->type = CLI_COOKIE_FILL;
            break;
        }
    }

    return (PASSED);
}

/*************************************************************************
 Function: disflag
 *
 * This function is to show current flag
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
disflag (int argc, char *argv[])
{
    testname("Disflag");
    if (argc > 1) {
        goto error;
    } else {
        menu_show_all_dflags();
        printf("\n");
        return 0;
    }

error:
    printf("usage: disflag\n");
    return 1;
}

/*************************************************************************
 Function: setflag
 *
 * This function is to set flag environment test and also restore the default flag
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
setflag (int argc, char *argv[])
{   
    testname("Setflag");
    int index;
    char *keyword_1st[] = {"default", "-", NULL};
    char *str = NULL;

    if (argc != 2) {
        goto error;
    } else {
        /* get 1st level keyword */
        index = cli_get_keyword(argv[1], keyword_1st);

        switch (index) {
            case 0: /* default */
                (NVRAM)->diagflag &= 0;
                diagflag_xram &= 0;
#ifdef AUTHENTICATION_TEST_Y
                diagflag_yram &= 0;
#endif /* AUTHENTICATION_TEST_Y */

                break;
            case 1: /* - */
                str = argv[argc - 1];
                if (cli_check_option(str))
                {
                    goto error;
                }
                /* skip '-' */
                str++;
                while (str[0] != '\0') {
                    menu_flags(str[0]);
                    str++;
                }
                
                break;
            default:
                goto error;
        }

        menu_show_all_dflags();
        printf("\n");
        return 0;
    }

error:
    printf("usage: setflag [default | -{A | C | D | E | L | M | O |\n"
           "                            S | T | V | W | X | U}]\n");
    return 1;
}

/*************************************************************************
 Function: settime
 *
 * This function is to parser the input time and compare every value
 * to match the date rules and if is correct then it will set to the RTC.
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
settime(int argc, char *argv[])
{   
    testname("Settime");
    cli_token tlist;
    cli_value *ptr;
    char *str;
    int value = 0;
    cli_time timeval;
    int count = 0;
    int ret = 0;
    int i;
    
    tlist.vlist = NULL;
    tlist.token = ':';
    tlist.count = 0;
    str = argv[1];
    
    if (argc != 2) {
         goto error;
    } else { 
        cli_parse_token(&str, &tlist);
        count = tlist.count;
        if (count != 3) {
            goto error;
        }

        for (ptr = tlist.vlist, i = 1; ptr != NULL; ptr = ptr->next, i++) 
        {
            value = cli_str2num(ptr->value, CLI_DEC);
            switch (i) {
                case hour :
                    timeval.hour = value;
                    break;
                case minute :
                    timeval.minute = value;
                    break;
                case second :
                    timeval.second = value;
                    break;
                default :
                      break;
            }
        }

        ret = checktime(&timeval);

        if (ret) {
            goto error;
        }
    }

    ret = cli_set_time (CLI_SET_TIME, &timeval);
   
    if (ret) {
        goto error;
    }

    cli_del_node(&tlist);
    return ret;

error:
    printf("usage: settime HH:MM:SS\n");
    printf("       Range HH = 00~23\n");
    printf("       Range MM = 00~59\n");
    printf("       Range SS = 00~59\n");
    
    cli_del_node(&tlist);

    return (FALSE);
}

static int 
checktime(cli_time *timeval)
{
    if (((timeval->hour >= 0) && (timeval->hour <= 23)) && 
        ((timeval->minute >= 0) && (timeval->minute <= 59)) &&
        ((timeval->second >= 0) && (timeval->second <= 59))) {
       return (PASSED);
    }

    return (FAILED);
}

/*************************************************************************
 Function: setdate
 *
 * This function is to parser the input date and compare every value
 * to match the date rules and if is correct then it will set to the RTC.
 *       
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
setdate(int argc, char *argv[])
{   
    testname("Setdate");
    cli_token tlist;
    cli_value *ptr;
    char *str;
    int value = 0;
    cli_time timeval;
    int count = 0;
    int ret = 0;
    int i;

    tlist.vlist = NULL;
    tlist.token = '/';
    tlist.count = 0;
    str = argv[1];
    
    /* ie. setdate 02/28/2007 
       this argc = 2 
    */
    if (argc != 2) {
        goto error;
    } else {
        /* ie. 02/28/2007 --> remove the "/" --> 02 28 2007
           count = 3
         */
        cli_parse_token(&str, &tlist);
        count = tlist.count;
        if (count != 3) {
            goto error;
        }
        /* ie. month = 02
               date  = 28
               year  = 2007
         */
        for (ptr = tlist.vlist, i = 1; ptr != NULL; ptr = ptr->next, i++) {
            value = cli_str2num(ptr->value, CLI_DEC);
            switch (i) {
                case month :
                    timeval.month = value;
                    break;
                case date :
                    timeval.date = value;
                    break;
                case year :
                    timeval.year = value;
                    break;
                default :
                    break;
            }
        }
        /* check is it correct date rules */ 
        ret = checkdate(&timeval);

        if (ret) {
            goto error;
        }
    }
    /* set the date value to RTC structure */
    ret = cli_set_time (CLI_SET_DATE, &timeval);
    
    if (ret) {
        goto error;
    } 
    /* delete the node */
    cli_del_node(&tlist);

    return ret;

error:
    printf("usage: setdate MM/DD/YYYY  \n");
    printf("       Range MM = 01~12    \n");
    printf("       Range DD = 01~31    \n");
    printf("       Range YYYY = 2000~2050\n");
    
    cli_del_node(&tlist);

    return (FALSE);
}

/****************************************************************************
 Function : checkdate
 *    This Function are to check date value
 *    Rules : 1. Months that have 31 days
 *            2. Months that have 30 days
 *            3. Months of February that has 29 days on Lunar Year 
 *               otherwise on February has only 28 days.
 *
 * Input  : time_val_t
 * Output : return TRUE / FALSE
 *   
 ****************************************************************************
*/
static int 
checkdate(cli_time *timeval)
{
    /* Date 01~31 */
    if((timeval->date < 1) || (timeval->date > 31)) {
        printf("The date range is 01~31.\n");
        return (FAILED);
    }
    
    /* Month 01~12 */
    if((timeval->month < 1) || (timeval->month > 12)) {
        printf("The month range is 01~12.\n");
        return (FAILED);
    }

    /* Year 2000~2050 */
    if((timeval->year < 2000) || (timeval->year > 2050)) {
        printf("The year range is 2000~2050.\n");
        return (FAILED);
    }

    /* Lunar Year on February is 29 days */   
    if ((timeval->date == 29) && (timeval->month == 2)) {
        if(timeval->year % 4) {
            printf("February 29th day is not Lunar Year.\n");
            return (FAILED);
        }
    }
    
    /* Feb has no 30 and 31 days */
    if((timeval->date >= 30) && (timeval->month == 2)) { 
        printf("February has no 30th and 31th days.\n"); 
        return (FAILED);
    }

    /* Month has no 31 days */
    if((timeval->month == 4) || (timeval->month == 6) || (timeval->month == 9) 
        || (timeval->month == 11)) { 
        if (timeval->date >= 31) {
            printf("This month has no 31th day.\n");
            return (FAILED);
        }
    }

    return (PASSED);
}

/*************************************************************************
 Function: display RTC value
 *
 * This function is to display RTC value.
 *       
 * Input: argc, argv[]
 *
 * Output: printf RTC value. 
 **************************************************************************
*/
int 
disrtc(int argc, char *argv[])
{
#ifndef OVERLORD
    testname("Disrtc");
    ds1337_time_t tv;
	  
    if (argc > 1) {
        goto error;
    } else {
        /* Get rtc value */
        get_rtc(&tv);
        /* Printf rtc value */
        printf("\n Date: %.2d/%.2d/%.4d  Time: %.2d:%.2d:%.2d\n", tv.month, 
            tv.date, tv.year, tv.hour, tv.minute, tv.second ); 
        printf("\n");
        return 0;
    }

error:
    printf("usage: disrtc\n");

#else
    utility_get_rtc(1);
#endif
    return 1;
}

/*************************************************************************
 Function: basic_util
 *
 * This function is to show basic utility menu.
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
basic_util (int argc, char *argv[])
{  
    testname("Basic_util");
    if (argc > 1) {
        goto error;
    } else {
        menu(utilmenup, '\0', '\0' );
        printf("\n");
        return (PASSED);
    }

error:
    printf("usage: basic_util\n");
    return (FAILED);
}

/*************************************************************************
 Function: discovery
 *
 * This function is to show information like cookies, fan , power, temp status.
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
discovery (int argc, char *argv[])
{   
    int ret = FAILED;
    testname("Discovery");
    envflag = INDIAG; /* set the environment flag */
    if (argc > 1) {
        printf("\nDiscovery has no Paramater\n");
        goto error;
    }
#ifndef LINUX_APP
    /* Show Memory Size status */
    printf("\nMEM_SIZE:%dMB", get_platform_memsize());
#endif

    /* Show DP/CP mem size */
    cli_discovery_mem();

    /* Show MB */
    cli_discovery_mb();
    
    if(cli_support_vm()){
        /* Show PVDM */
        cli_discovery_mb_vm();
    }
    if(cli_support_mb_wic()){
        /* Show MB-HWIC */
        cli_discovery_mb_wic();
    }
    if(cli_support_sm()){
        /* Show SM, NM, NM-VIC, NM-PVDM */
        cli_discovery_sm();
    }
    if(cli_support_psu()){
        /* Show PSU*n */
        cli_discovery_psu();
    }
    if(cli_support_cf()){
        /* Show Compact Flash Info */
        cli_discovery_cf();
    }
    /* Show Environment status */
    printf("\n");
#ifndef LINUX_APP
    ret = show_temp(FALSE, DISPLAY_M2M);
#else
    printf("%s not supported\n", __FUNCTION__);
#endif
    /* Show Platform voltage and frequency */
    ret = show_margins_x(FALSE, CLI_MODE);

    printf("\n\n");

    return ret;
    
error:
    printf("usage: discovery\n");
    
    return ret;
}


/*************************************************************************
 Function: support_cp_memsize
 *
 * This function is to check which platform support CP mem size.
 * 
 * Input: void
 *
 * Output: TRUE/FALSE
 **************************************************************************
*/
static int 
support_cp_memsize (void)
{   
    return TRUE; 
}


/*************************************************************************
 Function: support_dp_memsize
 *
 * This function is to check which platform support DP mem size.
 * 
 * Input: void
 *
 * Output: void
 **************************************************************************
*/
static int 
support_dp_memsize (void)
{   
    return FALSE;
}


/*************************************************************************
 Function: cli_discovery_mem
 *
 * This function wrapper to show memsize of DP/CP
 * 
 * Input: void
 *
 * Output: void
 **************************************************************************
*/
static void 
cli_discovery_mem (void) 
{  
    /* for CP */
    if(support_cp_memsize()) {
        get_cp_memsize(MB_I2C_DIMM0);
        get_cp_memsize(MB_I2C_DIMM1);
    }

    /* for DP */
    if(support_dp_memsize()) {
      
    }

    return;
}

/*************************************************************************
 Function: cli_discovery_mb
 *
 * This function wrapper to show information MB cookies
 * 
 * Input: void
 *
 * Output: void
 **************************************************************************
*/
static void 
cli_discovery_mb(void) 
{   
    cli_cookie_cmd cmd;

    /* init */
    cmd.type = CLI_COOKIE_DISPLAY_M2M; /* display in machine to machine */
    memset(cmd.buf, 0, sizeof(cmd.buf));
    strcpy(cmd.buf, "MB:");
    cmd.cli_mode = CLI_DISCOVERY;
    /* do MB discovery */
    if(do_cli_discovery_mb(&cmd)){
        prpass(testpass, "MB");
        cterr('f',0,"MB:Discovery Failed");
    }

    return;
}

/*************************************************************************
 Function: do_cli_discovery_mb
 *
 * This function is to show information MB cookies
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: PASSED /FAILED
 **************************************************************************
*/
static int 
do_cli_discovery_mb(cli_cookie_cmd *cmd)
{
    /* functions in platform_cookie.c */
    return (alter_mb_cookie_x(CLI_MODE, cmd));
}

/*************************************************************************
 Function: cli_discovery_mb_wic
 *
 * This function is to show information like mb_wic cookies.
 * 
 * Input: void
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_discovery_mb_wic(void)
{   
    cli_cookie_cmd cmd;
    unsigned int slot, max_slot, min_slot, len;
    unsigned int rc;

    /* WIC slot */
    cmd.type = CLI_COOKIE_DISPLAY_M2M; /* display in machine to machine */
    cmd.cli_mode = CLI_DISCOVERY;
    min_slot = FIRST_SLOT;
    max_slot = get_max_wic_slots();

    for(slot = min_slot; slot < (max_slot + FIRST_SLOT); slot++) {

        memset(cmd.buf, 0, sizeof(cmd.buf));
        strcpy(cmd.buf, "WIC");
        len = strlen(cmd.buf);
        cmd.slot = slot;
        sprintf(&cmd.buf[len], "%d:", cmd.slot);

        /* do GR/E/H/WIC Discovery */
        rc = do_cli_discovery_mb_wic(&cmd); 
        if (rc == CLI_DEVICE_IS_VACANT) {
            continue;

        } else if (rc == FAILED) {
            prpass(testpass, "%s", cmd.buf);
            cterr('f',0,"%sDiscovery Failed", cmd.buf);
            continue;
        } else { 
            /* do_cli_discovery_mb_wic() is passed */
        }
        
	/*
	 * Discover the daughter card cookies.
	 * do_cli_discovery_mb_wic() must set the values of
	 * cmd.present and cmd.hwic_cookie_id.
	 */
        if((cmd.present != FALSE) &&
	   (cmd.hwic_cookie_id != INVALID_ID)) {

            strcpy(cmd.buf, "DC-EHWIC:");
           
            rc = alter_wic_dc_cookie_cli(CLI_MODE, &cmd);
            if (rc == CLI_DEVICE_IS_VACANT) {
                continue;

            } else if (rc == FAILED) {
                prpass(testpass, "%s", cmd.buf);
                cterr('f',0,"%sDiscovery Failed", cmd.buf);
                continue;
            } else { 
                /* alter_wic_dc_cookie_cli() is passed */
            }
        }
    }
    return;
}

/*************************************************************************
 Function: do_cli_discovery_mb_wic
 *
 * This function is to show information like mb_wic cookies.
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: PASS /FAIL . 
 **************************************************************************
*/
static int 
do_cli_discovery_mb_wic(cli_cookie_cmd *cmd)
{   
    /* functions in platform_cookie.c */
    return (alter_wic_cookie_cli(CLI_MODE, cmd));
}


/*************************************************************************
 Function: cli_discovery_mb_vm
 *
 * This function is to show information like mb_vm cookies
 * 
 * Input: none
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_discovery_mb_vm(void)
{   
    cli_cookie_cmd cmd;
    int slot, max_slot, min_slot, len, rc;
    /* PVDM Slot */
    min_slot = 0;
    max_slot = get_max_num_vm();
    memset(cmd.buf, 0, sizeof(cmd.buf));
    cmd.type = CLI_COOKIE_DISPLAY_M2M; /* display in machine to machine */
    cmd.cli_mode = CLI_DISCOVERY;
    /* PVDM */
    strcpy(cmd.buf, "VM");
    len = strlen(cmd.buf);
    for (slot = min_slot; slot < max_slot; slot++) {
        cmd.slot = slot;             /* VM slot */
        sprintf(&cmd.buf[len], "%d:", cmd.slot);
        /* do mb vm discovery */
        rc = do_cli_discovery_mb_vm(&cmd); 
        if (rc == CLI_DEVICE_IS_VACANT) {
            continue;

        } else if (rc == FAILED) {
            prpass(testpass, "%s", cmd.buf);
            cterr('f',0,"%sDiscovery Failed", cmd.buf);
            continue;
        } else { 
            /* do_cli_discovery_mb_vm() is passed */
        }
    }

    return;
}

/*************************************************************************
 Function: do_cli_discovery_mb_vm
 *
 * This function is to show information like mb_vm cookies
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: PASS / FAIL 
 **************************************************************************
*/
static int 
do_cli_discovery_mb_vm(cli_cookie_cmd *cmd)
{
    /* function in platform_cookie.c */
    return (alter_vm_cookie_cli(CLI_MODE, cmd));
}

/*************************************************************************
 Function: cli_discovery_sm
 *
 * This function is to show information like SM and NM adapter cookies.
 * 
 * Input: none
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_discovery_sm(void) 
{   
    cli_cookie_cmd cmd;
    int slot, max_slot, min_slot, len, rc;

    /* SM Slot */
    cmd.type = CLI_COOKIE_DISPLAY_M2M; /* display in machine to machine */
    cmd.cli_mode = CLI_DISCOVERY;
    min_slot = slot_start_with();
    max_slot = get_max_sm_slots();

    for (slot = min_slot; slot <= max_slot; slot++) {

        /* do sm discovery */
        memset(cmd.buf, 0, sizeof(cmd.buf));
        strcpy(cmd.buf, "SM");
        len = strlen(cmd.buf);
        cmd.slot = slot;
        sprintf(&cmd.buf[len], "%d:", cmd.slot);

        rc = do_cli_discovery_sm(&cmd); 
        if (rc == CLI_DEVICE_IS_VACANT) {
            continue;

        } else if (rc == FAILED) {
            prpass(testpass, "%s", cmd.buf);
            cterr('f',0,"%sDiscovery Failed", cmd.buf);
            continue;
        } else { 
            /* do_cli_discovery_sm() is passed */
        }


        if((cmd.present != FALSE) &&
           (cmd.sm_cookie_id != INVALID_ID)) {

            strcpy(cmd.buf, "DC-SM:");

            rc = alter_sm_dc_cookie_cli(CLI_MODE, &cmd);
            if (rc == CLI_DEVICE_IS_VACANT) {
                continue;

            } else if (rc == FAILED) {
                prpass(testpass, "%s", cmd.buf);
                cterr('f',0,"%sDiscovery Failed", cmd.buf);
                continue;
            } else {
                /* alter_sm_dc_cookie_cli() is passed */
            }
        }

#ifdef NOT_USED
        /* if SM present */
        if(cmd.present != FALSE) {
            
            /* do scan SM table */
            if(do_sm_nlevel(&cmd)) {
                prpass(testpass, "%s", cmd.buf);
                cterr('f',0,"%sDiscovery Failed", cmd.buf);
                continue;
            }
            /* if SM is SM ADAPTER CARD */
            if ((cmd.sm_cookie_id == SM_ADAPTER_CARD) || (cmd.sm_cookie_id == SM2PA_ADAPTER_CARD)) {
                /* do NM discovery */
                strcpy(cmd.buf, "NM");
                len = strlen(cmd.buf);
                sprintf(&cmd.buf[len], "%d:", cmd.slot);
                if(do_cli_discovery_nm(&cmd)) {
                    prpass(testpass, "%s", cmd.buf);
                    cterr('f',0,"%sDiscovery Failed", cmd.buf);
                    continue;
                }
        
                /* if NM not present then continue to the next */
                if(cmd.present != FALSE) {
                    /* do scan NM table */
                    if(do_nm_nlevel(&cmd)) {
                        prpass(testpass, "%s", cmd.buf);
                        cterr('f',0,"%sDiscovery Failed", cmd.buf);
                        continue;
                    }
                } else {
                    /* else NM not present then continue to the next NM */
                    continue;
                }
            }
        } else {
            /* else SM not present then continue to the next SM */
            continue;
        }
#endif /* NOT_USED */
        
    }
    return;
}
    
/*************************************************************************
 Function: do_cli_discovery_sm
 *
 * This function is to show information like SM and NM adapter cookies.
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: Pass / Failed . 
 **************************************************************************
*/
static int 
do_cli_discovery_sm(cli_cookie_cmd *cmd)
{   
    /* function in platform_cookie.c */
    return (alter_sm_cookie_cli(CLI_MODE, cmd));

}

#ifdef NOT_USED
/*************************************************************************
 Function: do_cli_discovery_nm
 *
 * This function is to show information like SM and NM adapter cookies.
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: Pass / Failed . 
 **************************************************************************
*/
static int 
do_cli_discovery_nm(cli_cookie_cmd *cmd)
{   
    /* function in platform_cookie.c */
    return (alter_sm_dc_cookie_cli(CLI_MODE, cmd));
}
#endif

#ifdef NOT_USED
/*************************************************************************
 Function: do_sm_nlevel
 *
 * This function is to scan sm cookie id to match one on sm n level table 
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: Pass / Failed . 
 **************************************************************************
*/
static int 
do_sm_nlevel(cli_cookie_cmd *cmd)
{   
    int ret = PASSED;
    unsigned int i = 0;

    /* display sm n level discovery if exist on cli_sm_level_tbl */
    if((cmd->sm_cookie_id != SM_ADAPTER_CARD) && (cmd->sm_cookie_id != SM2PA_ADAPTER_CARD)) {
        for(i = 0; i < CLI_SMLEVEL_MAX; i++) {
            if(cmd->sm_cookie_id == cli_sm_level_tbl[i].id ) {
                /* 2nd level routine */
                if(cli_sm_level_tbl[i].secondlvl != NULL) {  
                    if((*cli_sm_level_tbl[i].secondlvl)(CLI_MODE, cmd)){
                        ret = FAILED;
                        continue;
                    }
                    ret = PASSED;
                }
            }
        }
    }
    if(ret == FAILED) {
        return (ret);
    }

    return (ret);
}
#endif 

#ifdef NOT_USED

/*************************************************************************
 Function: do_nm_nlevel
 *
 * This function is to scan nm cookie id to match one on nm n level table 
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: Pass / Failed . 
 **************************************************************************
*/    
static int 
do_nm_nlevel(cli_cookie_cmd *cmd)
{   
    int ret = PASSED;
    unsigned int i = 0;
    unsigned int len = 0;

    /* display nm n level discovery if exist on cli_nm_level_tbl */
    if(cmd->nm_cookie_id != INVALID_ID) {
        for(i = 0; i < CLI_NMLEVEL_MAX ; i++) {
            if(cmd->nm_cookie_id == cli_nm_level_tbl[i].id ) {
                /* 2nd level routine */
                if(cli_nm_level_tbl[i].secondlvl != NULL) {  
                    if((*cli_nm_level_tbl[i].secondlvl)(CLI_MODE, cmd)){
                        ret = FAILED;
                        continue;
                    }
                    ret = PASSED;
                }
                /* 3rd level routine */
                if(cli_nm_level_tbl[i].thirdlvl != NULL) { 
                    cmd->max_vs = 1;  /* min level slot */
                    for (cmd->dc_slot = 0; cmd->dc_slot < cmd->max_vs; 
                         cmd->dc_slot++){
                        /* do nm 3rd level discovery */
                        strcpy(cmd->buf, "SM");
                        len = strlen(cmd->buf);
                        sprintf(&cmd->buf[len], "%d:WIC%d:ECAN%d:", cmd->slot, 
                                cmd->dc_slot, 0);
                        if((*cli_nm_level_tbl[i].thirdlvl)(CLI_MODE, cmd)){
                            ret = FAILED;
                            continue;
                        }
                        ret = PASSED;
                    }
                }
                /* 4th level routine */
                if(cli_nm_level_tbl[i].forthlvl != NULL) { 
                    cmd->max_vs = 1;  /* min level slot */
                    for (cmd->dc_slot = 0; cmd->dc_slot < cmd->max_vs; 
                             cmd->dc_slot++){
                        /* do nm 4th/vm on nm level discovery */
                        strcpy(cmd->buf, "SM");
                        len = strlen(cmd->buf);
                        sprintf(&cmd->buf[len], "%d:PVDM%d:", cmd->slot, 
                                  cmd->dc_slot);
                        if((*cli_nm_level_tbl[i].forthlvl)(CLI_MODE, cmd)){
                            ret = FAILED;
                            continue;
                        }
                        ret = PASSED;
                    }
                }
            }
        }
    }
    if(ret == FAILED) {
        return (ret);
    }
    
    return (ret);
}
#endif 

/*************************************************************************
  Function: cli_discovery_psu
 *
 * This function is to show information PSU*n cookies
 * 
 * Input: none
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_discovery_psu(void)
{   
    cli_cookie_cmd cmd;
    int len;
    uint psu_count, count, rc;
    	
    /* init */
    /* check if platfrom support psu */
    if(has_ps1() && has_ps2()){
        psu_count = TWOPSU;  /* have two psu */
    }else 
    if (has_ps1()) {
        psu_count = ONEPSU;  /* has one psu */
    }else {
        printf("\nthere is no psu for this platform");
        return;
    }
     
    memset(cmd.buf, 0, sizeof(cmd.buf));
    strcpy(cmd.buf, "PSU");
    len = strlen(cmd.buf);
    cmd.type = CLI_COOKIE_DISPLAY_M2M; /* display in machine to machine */
    cmd.cli_mode = CLI_DISCOVERY;
    for (count = ONEPSU; count <= psu_count; count++ ){
        sprintf(&cmd.buf[len], "%d:", count);
        cmd.slot = count;
        /* do PSU discovery */
        /* function in platform_psu.c */
        rc = psu_show_cookie_x(CLI_MODE,&cmd);
        if (rc == CLI_DEVICE_IS_VACANT) {
            continue;

        } else if (rc == FAILED) {
            prpass(testpass, "%s", cmd.buf);
            cterr('f',0,"%sDiscovery Failed", cmd.buf);
            continue;
        } else { 
            /* do_cli_discovery_ism() is passed */
        }
    }
    return;
}

/*************************************************************************
 Function: cli_discovery_cf
 *
 * This function is to show information of Compact Flash
 * 
 * Input: void
 *
 * Output: void 
 **************************************************************************
*/
static void 
cli_discovery_cf(void)
{
#ifndef LINUX_APP
    cli_cookie_cmd cmd;
    int slot = 0;
    int len = 0;
    
    memset(cmd.buf, 0, sizeof(cmd.buf));
    strcpy(cmd.buf, "CF");
    len = strlen(cmd.buf);
    /* Function in platform_cf_test.c */
    for (slot = 0; slot < 2; slot++) {
        sprintf(&cmd.buf[len], "%d", slot);
        cf_info_x(slot, CLI_MODE, &cmd);
    }
#else
    printf("%s not supported\n", __FUNCTION__);
#endif
    return;
}

/*************************************************************************
 Function: cookie
 *
 * This function is to modify, saves, and show cookie 4 information
 * 
 * Notes: if the cookie size got extended to any size please check the
 *        files in the include folder if the size of command line is has
 *        correct limitation of size. Files need to review are monitor.h, sh.h, 
 *        mon_boot.h, nvsysvars.h.
 * 
 * Input: argc, argv[]
 *
 * Output: PASSED / FAILED 
 **************************************************************************
*/
int 
cookie (int argc, char *argv[])
{   
    testname("Cookie");
    envflag = INDIAG; /* set the environment flag */
    cli_cookie_cmd cmd;
    int count = 1, index;
    /* ie. cookie MB -S128 -FMT , keyword 1st is MB */
    char *keyword_1st[] = {"MB", "BP", "VM", "SM", "WIC", "NM", "PSU", "MP",
                           "ISM", "SM:DC", "NM:EHWIC", "NM:DC", "NM:DC_GE", "NM:DC_PWR",
                           "NM:EM", "NM:EHWIC:ECAN", 
                           "NM:PVDM", "WIC:DC", "EHWIC:ECAN",
                           NULL};
    /* init cookie cmd variables */
    cmd.type = CLI_COOKIE_BEGIN;
    cmd.contents = NULL;
    cmd.cli_mode = CLI_COOKIE;
    /* get 1st level keyword, the target */
    index = cli_get_keyword(argv[count], keyword_1st);
    /* ie. cookie MB -S128 -FMT , index is MB */

    switch (index) {
        case CLI_COOKIE_VM: /* VM */
        case CLI_COOKIE_SM: /* SM */
        case CLI_COOKIE_SMDC: /* SM:DC */
        case CLI_COOKIE_WIC: /* WIC */
        case CLI_COOKIE_WICDC: /* WIC:DC */
        case CLI_COOKIE_EHWICECAN:  /* EHWIC:ECAN*/
        case CLI_COOKIE_NM: /* NM */
        case CLI_COOKIE_NMEHWIC: /* NM:EHWIC */
        case CLI_COOKIE_NMDC:    /* NM:DC */
        case CLI_COOKIE_NMDCGE:  /* NM:DC_GE */
        case CLI_COOKIE_NMDCPWR:  /* NM:DC_PWR */
        case CLI_COOKIE_NMEM:     /* NM:EM */
        case CLI_COOKIE_NMEHWICECAN: /* NM:EHWIC:ECAN */
        case CLI_COOKIE_NMPVDM: /* NM:PVDM */
        case CLI_COOKIE_PSU: /* PSU */
        case CLI_COOKIE_ISM: /* ISM */
            if (++count >= argc) {
                goto error;
            }
        case CLI_COOKIE_MB: /* MB */
        case CLI_COOKIE_BP: /* BP */
        case CLI_COOKIE_MP: /* MP */
            /* ie. cookie MB then init the MB variables to cmd */
            if (cli_init_cookie_vars(index, argv[count], &cmd)) {
                return (FAILED);
            }
            break;
        default:
            goto error;
    }
    
    /* format should be <keyword> <field> <value> */
    if (++count >= argc) {
        goto error;
    }
    
    
    /* get 2nd level keyword, field of cookie */
    while(count < argc) {
        if ((cmd.type == CLI_COOKIE_FILL) || 
            (cmd.type == CLI_COOKIE_DISPLAY_RAW) || 
            (cmd.type == CLI_COOKIE_DISPLAY_FMT)) {
            goto error;
        }
        
        /* compare the cookie size and cookie 4 field */
        index = cli_cookie_strncmp (argv[count], &cmd);

        /* format should be <field> <value> */
        if (++count >= argc) {
            goto error;
        }

        /* Malloc */
        if (cmd.contents == NULL) {
            cmd.contents = (uchar *)malloc(cmd.size);
        }
        /* cli get field 
         * decide the cli cookie action for :
         * - Read FMT or RAW
         * - Write RAW          
         */
        if (cli_get_field(index, argv[count], &cmd)) {
            /* free memory */
            cli_free_mem(&cmd);
            goto error;
        }
        
        /* cmd.str is should be <value> */
        cmd.str = argv[count];

        /* do select which board type cookie */
        if(do_cli_cookie_switches(&cmd)) {
            printf("\n");
            /* free memory */
            cli_free_mem(&cmd);
            goto error;
        }
        printf("\n");
        /* free memory */
        cli_free_mem(&cmd);
        count++;
    }        
    return (PASSED);

error:
   cli_cookie_no_ism_msg();
    
    return (FAILED);
}

/*************************************************************************
 Function: cli_cookie_no_ism_msg
 *
 * This function is to show help messg for cookie command
 *  
 * Input: 
 *
 * Output: 
 **************************************************************************
*/
static void
cli_cookie_no_ism_msg(void)
{
    printf("usage: cookie [MB | MP | VM <slot 1> | \n"
    "              PSU <units 1-2> | \n"
    "              WIC <slot 1-%d> | \n"
    "              EHWIC:ECAN <slot EHWIC>:<slot ECAN> | \n" 
    "              WIC:DC <slot WIC>:<slot DC> | \n" 
    "              SM <slot 1-%d> | \n"
    "              SM:DC <slot SM>:<slot DC> | \n" 
    "              NM:EHWIC <slot NM>:<slot EHWIC> | \n"
    "              NM:DC <slot NM>:<slot DC> | \n"
    "              NM:DC_GE <slot NM>:<slot DC_GE> | \n"
    "              NM:DC_PWR <slot NM>:<slot DC_PWR> | \n"
    "              NM:EM <slot NM>:<slot EM> | \n"
    "              NM:EHWIC:ECAN <slot NM>:<slot EHWIC>:<slot ECAN> ]\n"           
    "              NM:PVDM <slot NM>:<slot PVDM> | \n"
    "              {FABREV <value> | RMATST <value> |\n"
    "              RMAHIS <value> | CONTYP <value> | EHSAPREFMSTR <value>|\n"
    "              VENDID <value> | PROTYP <value> | TDM <value> |\n" 
    "              PWRSPLYTYP <value> | COMPABYTE <value> | CNTL <value> |\n"
    "              HWREV <value> | PCBREV <value> | MACADDRBLKSIZ <value> |\n"
    "              CAPCODEL <value> | SLFTSTRES <value> | BOOTTIMEOUT <value> |\n"
    "              MBCHID <value> | GROUPTYP <value> | CLIWRTEN <value> |\n"
    "              RADIOCOUNTRYCODE <value> | DEVINUM <value> | RMANUM <value> |\n" 
    "              PARTNUM <value> | HWDATECODE <value> | MFGENGINEER <value> |\n"
    "              28NUM <value> | CALIBDATA <value> | TOPASSYPRTNUM <value> |\n"
    "              NEWDEVNO <value> | VID <value> | PCBREV2 <value> |\n"
    "              800 <value> | PCBSER <value> | CHASSN <value> |\n"
    "              CHMAC <value> | MFGTSTDAT <value> | FDIAGDATA <value> |\n"
    "              CLEI <value> | ENVMON <value> | IBR-MC <value> |\n"
    "              DEVVALUES <value> | HDINFO <value> | PRODID <value> |\n"
    "              ASSETID <value> | IBR-MC2 <value> | PCBSERNUM <value> |\n"
    "              BAMAC <value> | CARDNAME <value> | FILENAME <value> |\n"
    "              ENCRPYT <value> | LONGICALIB <value> | ASSETALIAS <value>|\n"
    "              PROLEVEL <value> | SYSCLKFREQ <value> | DIGSIG <value> |\n"
    "              RFIDPCA <value>  |\n"
    "              MC520 <value> | LASERINFO <value> | ENVMONEXTINFO <value> |\n"
    "              -S128 [<value> | -RAW | -FMT] |\n"
    "              -S256 [<value> | -RAW | -FMT] |\n"
    "              -S512 [<value> | -RAW | -FMT] }\n",
           (get_max_wic_slots()) , get_max_sm_slots());
}

/*************************************************************************
 Function: do_cli_cookie_switches
 *
 * This function is to execute alter cookie utility switch based 
 * of the board type
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: PASSED / FAILED
 **************************************************************************
*/
int 
do_cli_cookie_switches(cli_cookie_cmd *cmd) 
{
    int ret = PASSED;
    
    switch (cmd->board_type) {
        case MOTHER_BOARD:
            /* function in platform_cookie.c */
            return (alter_mb_cookie_x(CLI_MODE, cmd));
        case VM_MODULE:
            return (alter_vm_cookie_cli(CLI_MODE, cmd));  
        case WIC_MODULE:
            return (cli_cookie_wicdclevel(cmd));
        case SM_MODULE:
            /* function in platform_cookie.c */
            return (cli_cookie_smdclevel(cmd));
        case PSU_MODULE:
            /* function in platform_psu.c */
            return (psu_show_cookie_x(CLI_MODE, cmd));
        default:
            printf("cli cmd: type %c not suported\n", cmd->board_type);
            ret = FAILED;
            break;
    }

    return (ret);
}

/*************************************************************************
 Function: cli_cookie_smdclevel
 *
 * This function is to cli cookie scan sm daughtercard cookie on table
 * 
 * Input: cli_cookie_cmd *cmd
 *
 * Output: PASSED / FAILED 
 **************************************************************************
*/
static int 
cli_cookie_smdclevel (cli_cookie_cmd *cmd)
{
    
    if ((cmd->dc_type) == CLI_COOKIE_SMDC) {
        return (alter_sm_dc_cookie_cli(CLI_MODE, cmd));
    } else {
        /* function in platform_cookie.c */
        return (alter_sm_cookie_cli(CLI_MODE, cmd));
    }
}


/*************************************************************************
 Function: cli_cookie_wicdclevel
 *
 * This function is to cli cookie scan wic daughtercard cookie on table
 * 
 * Input: cli_cookie_cmd *cmd
 * 
 * Output: PASSED / FAILED 
 **************************************************************************
*/
static int 
cli_cookie_wicdclevel (cli_cookie_cmd *cmd)
{
    if ((cmd->dc_type) == CLI_COOKIE_WICDC) {
        return (alter_wic_dc_cookie_cli(CLI_MODE, cmd));
    } else {        
            /* functions in platform_cookie.c */
        return (alter_wic_cookie_cli(CLI_MODE, cmd));
    }
}
/*************************************************************************
 Function: cli_cookie_strncmp
 *
 * This function is to compare cookie size and index field
 * 
 * Input: char *str
 *        cli_cookie_cmd *cmd
 *
 * Output: return (index)  or
 *         return CLI_UNKNOWN_INDEX
 **************************************************************************
*/  
int 
cli_cookie_strncmp (char *str, cli_cookie_cmd *cmd)
{   
    int index = 0;
    cookie_4_table *cookie_ptr = cookie_4_info;
        /* compare 2nd keyword for cookie size*/
    if (strncmp(str, "-S128", strlen("-S128")) == 0){
    	if ((cmd->board_type == MOTHER_BOARD) || (cmd->board_type == SM_MODULE)
	    || (cmd->board_type == WIC_MODULE)){
            printf("\nCookie size not support for 128 on this module.\n");
            return CLI_UNKNOWN_INDEX;
        }else 
        if ((cmd->board_type == PSU_MODULE)||(cmd->board_type == BACK_PLANE)){
            printf("\nCookie size not support for 128 on this module.\n");
            return CLI_UNKNOWN_INDEX;
        }else {
            index = CLI_S128;
        }
    } else 
    if (strncmp(str, "-S256", strlen("-S256")) == 0){
        if(cmd->board_type == PSU_MODULE) {
            index = CLI_S256;
        }else 
        if(cmd->board_type == BACK_PLANE){
            index = CLI_S256;
        }else {
            printf("\nCookie size not support for 256 on this module.\n");
            return CLI_UNKNOWN_INDEX;
        }
    } else
    if (strncmp(str, "-S512", strlen("-S512")) == 0){
        if ((cmd->board_type == MOTHER_BOARD) || (cmd->board_type == SM_MODULE)
	    || (cmd->board_type == WIC_MODULE)) {
            index = CLI_S512;
        }else {
            printf("\nCookie size not support for 512 on this module.\n");
            return CLI_UNKNOWN_INDEX;
        }
    } else { /* compare 2nd level keyword from cookie_4_table */
        for(index = 0;; index++ ){
            if(cookie_ptr[index].p_sn == NULL){
                return CLI_UNKNOWN_INDEX;
            } 

            if(strncmp(str, (char *)cookie_ptr[index].p_sn, 
                strlen(str)) == 0){
                /* printf("\n%s == %s", str, cookie_ptr[index].p_sn); */
                break;
            }
        }
        /* set the type become cookie change */
        cmd->type = CLI_COOKIE_CHANGE;
    }
    return (index);
}

/*************************************************************************
 Function: cli_get_test_target
 *
 * This function is to get test target
 *
 * Input: int index
 *        char *str
 *        cli_token *tlist
 *        
 * Output: FAILED / PASSED
 **************************************************************************
*/
static int 
cli_get_test_target(int index, char *str, cli_token *tlist)
{
    cli_token temp;
    cli_value *ptr;
    int slot, len; 
    int max_slot = 0;
    int min_slot = 0;
    char buf[80];
    uint smi;

    memset(buf, 0, sizeof(buf));
    /* ie. test MB , so index based MB */
    switch (index) {
        case CLI_TEST_MB:
            cli_insert_node(tlist, "motherboard tests");
            return (PASSED);
            break;
        case CLI_TEST_DATA_PLN:
            cli_insert_node(tlist, "test data plane");
            return (PASSED);
            break;
        case CLI_TEST_IO:
            cli_insert_node(tlist, "i/o interface test");
            return (PASSED);
            break;
        case CLI_TEST_AIM: /* AIM */
/*
            strcpy(buf, "test AIM ");
            min_slot = get_aim_min_slot();
            max_slot = get_aim_max_slot();
*/          
            break;
        case CLI_TEST_VM: /* VM */
            strcpy(buf, "test VM ");
            min_slot = 0;
            max_slot = get_max_num_vm();
            break;
        case CLI_TEST_WIC: /* WIC */
            strcpy(buf, "test WIC Slot ");
            min_slot = FIRST_SLOT;
            max_slot = get_max_wic_slots();
            break;
        case CLI_TEST_SM: /* SM */
            strcpy(buf, "test SM Slot ");
            min_slot = slot_start_with();
            max_slot = get_max_sm_slots();
            break;
        case CLI_TEST_MBWO: /* MB-WO */
            cli_insert_node(tlist, "motherboard tests");
            break;
        case CLI_TEST_ISM: /* ISM */
            cterr('f', 0, "ISM not suported on this platform\n");
            break;
        default:
            return (FAILED);
            break;
    }

    /* initialize */
    temp.vlist = NULL;
    temp.token = ',';

    len = strlen(buf);
    cli_parse_token(&str, &temp);
    for (ptr = temp.vlist; ptr != NULL; ptr = ptr->next) {
        switch (index) {
            case CLI_TEST_AIM: /* AIM */
            case CLI_TEST_VM: /* VM */
            case CLI_TEST_WIC: /* WIC */
            case CLI_TEST_SM: /* SM */
            case CLI_TEST_ISM: /* ISM */
                slot = cli_str2num(ptr->value, CLI_DEC);
                if ((slot > max_slot) || (slot < min_slot)) {
                    cli_del_node(&temp);
                    return (FAILED);
                }
        
                strcpy(&buf[len], ptr->value);
                cli_insert_node(tlist, buf);
                break;
            case CLI_TEST_MBWO: /* MB-WO */
                slot = cli_str2num(ptr->value, CLI_DEC);
                switch(slot){
                    case E0: /* E0 */ /* GE PHY0 */
                        smi = PHY0;
                        break;
                    case E1: /* E1 */ /* GE PHY1 */
                        smi = PHY1;
                        break;
                    case E2: /* E2 */ /* GE PHY2 */
                        smi = PHY2;
                        break;
                    case E3: /* E3 */ /* GE switch or GE PHY3 */
                        smi = PHY3;
                        break;
#ifdef N2G_OPPO
                    case E4: /* E4 */ /* GE switch */
                        smi = PHY4;
                        break;
                    case E5: /* E5 */ /* FE PHY */
                        smi = PHY5;  
                        break;
#endif
                    default:
                        return (FAILED);
                }
                
                if (smi == MB_SMI_INVALID) {
	                  assert(!"ge_phy_flag - invalid SMI port");
	                  return(FAILED);
                }
#ifndef LINUX_APP
                /* Set GE Phy [smi] test flag to disable*/
                ge_phy_test_flags[smi] = 0x00;
#endif
                
                break;
            default:
                return (FAILED);
        }
    }
    /* delete the node */
    cli_del_node(&temp);

    return (PASSED);
}

/*************************************************************************
 Function: cli_setup_testgrp
 *
 * This function is to assign the flag for the test that selected in the command.
 * 
 * Input: cli_token *tlist
 *
 * Output: none. 
 * Note: this function compare the menu item name with user input. 
 **************************************************************************
*/
static void 
cli_setup_testgrp(cli_token *tlist)
{
    submenu_xtable_t *miptr = main_menu_table;
    cli_value *ptr;
    int i;

    /* function in diag.c */
    int max_menu_table = cli_main_menu_table_size();

    for (i = 0; i < max_menu_table; i++, miptr++) {
        for (ptr = tlist->vlist; ptr != NULL; ptr = ptr->next) {
            if (strncasecmp(miptr->x_title, ptr->value, 
                        strlen(miptr->x_title)) == 0) {
                miptr->x_flags |= MF_DOGRP;
            }
        }
    }
}

/*************************************************************************
 Function: cli_dogrp_diags
 *
 * This function is to run execute routine.
 * 
 * Input: int flag
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_dogrp_diags (int flag)
{
    boolean all_items_done;

    all_items_done = cli_execgrp_diags(flag);
    if (all_items_done) {
        menu_pr_err_accum();
    } else {
        /*
         * User did <BREAK>.  Display accumulated errors here only if
         * not a continuous run because display will occur in menu() as
         * a result of <BREAK>.
         */
        if (!(DIAGFLAG & D_CONTINUOUS)) {
            menu_pr_err_accum();
        }
        if (monjmpptr) {
#ifdef LINUX_APP
            longjmp(*monjmpptr, 1);  /* Back to previous point */
#else
            longjmp(monjmpptr, 1);  /* Back to previous point */
#endif
        }
    }
}

/*************************************************************************
 Function: cli_execgrp_diags
 *
 * This function is to run execute routine.
 * 
 * Input: int flag
 *
 * Output: none. 
 **************************************************************************
*/
static int 
cli_execgrp_diags (int flag)
{
    submenu_xtable_t *miptr = main_menu_table;
    jmp_buf *savjmp = monjmpptr; /* save original jmpbuf */
    jmp_buf localjmp;
    int abort;
    char i;
    int max_menu_table = cli_main_menu_table_size();

    abort = setjmp(localjmp);
    if (!abort) {
        monjmpptr = &localjmp; /* redirect breaks */
        for (i = 0; i < max_menu_table; i++, miptr++) {

            /* 
             * Check that if NetBooted, MF_NOTNET not set and
             * the flag is set
             */
            if ((!netflashbooted || !(miptr->x_flags & MF_NOTNET))) {
                if (!(miptr->x_flags & flag)) {
                    continue;
                }

                /* Check for existence of the menu item (i.e. interface) */
                if (!(miptr->x_xfunc) || (*miptr->x_xfunc)(miptr->x_xparam)) {
                    /*
                     * Before executing the menu item function, if
                     * pre_diag_exec is non-NULL, call it as a preliminary
                     * (e.g., to restore mempools on this platform).
                     */
                    if (pre_diag_exec) {
                        (*pre_diag_exec)();
                    }
                    /* proceed the function with parameters */
                    (*miptr->x_pfunc)(miptr->x_pparam);

                    if (miptr->x_flags & MF_SHOW_ERRCOUNT) {
                        /*
                         * This (submenu) item doesn't give error
                         * summary by itself, as it is also used as a
                         * component of a larger diagnostic.
                         */
                        prcomplete(testpass, errcount, 0);
                    }
                    errcount = 0;
                }
            }
        }
        monjmpptr = savjmp;  /* redirect breaks back */
        return (TRUE);
    } else {
        monjmpptr = savjmp;  /* redirect breaks back */
        return (FALSE);  /* user did BREAK before all items completed */
    }
}

/*************************************************************************
 Function: cli_cleanup_testgrp
 *
 * This function is to restore the flag to initialize value.
 * 
 * Input: none
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_cleanup_testgrp()
{
    submenu_xtable_t *miptr = main_menu_table;
    int i;
    /* function in diag.c */
    int max_menu_table = cli_main_menu_table_size();

    for (i = 0; i < max_menu_table; i++, miptr++) {
        miptr->x_flags &= ~MF_DOGRP;
    }

}

/*************************************************************************
 Function: cli_reset_mb_wo
 *
 * This function is to restore the flag to initialize value.
 * 
 * Input: none
 *
 * Output: none. 
 **************************************************************************
*/
static void 
cli_reset_mb_wo()
{
#ifndef LINUX_APP
    int i;
    
#ifdef N2G_OPPO 
    int max_mb_wo_et = 5;
#else
    int max_mb_wo_et = 3;
#endif

    for(i = 0 ; i <= max_mb_wo_et; i++) {
        ge_phy_test_flags[i] = 0x03;
    }
#else
    printf("%s:%s currently not supported\n", __FUNCTION__, __FILE__);
#endif

}

/*************************************************************************
 Function: test
 *
 * This function is to call the submenu test to do the all test like in the menu
 * interface also have the specific keyword to test specific hardware.              
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
test (int argc, char *argv[])
{   
    testname("Test");
    jmp_buf *savjmp = monjmpptr;  /* save original jmpbuf */
    jmp_buf menujmp;
    cli_token tlist;
    char *keyword_1st[] = {"MB", "IO", "AIM", "VM", "WIC", "SM", 
                           "All", "-", "MB-WO", "ISM", "DATA_PLN", NULL};
    int count = 1, index = 0, flag = 0;
    int continuous;
    char *str = NULL;

    /* initialize */
    envflag = INDIAG; /* set the environment flag */
    tlist.vlist = NULL;
    /* ie. test MB WIC 0,1,2 SM 1,2 -CX 
    */
    while(count < argc)
    {
        /* get 1st level keyword */
        index = cli_get_keyword(argv[count], keyword_1st);
        /* ie. test MB WIC 0,1,2 SM 1,2 -CX 
           index = MB 
        */
        switch (index) {
            /* ie. SM, VM, WIC, MBWO that have slots */
            case CLI_TEST_AIM: /* AIM */
                break;
            case CLI_TEST_VM: /* VM */
            case CLI_TEST_WIC: /* WIC */
            case CLI_TEST_SM: /* SM */
            case CLI_TEST_MBWO: /* MB-WO */
            case CLI_TEST_ISM: /* ISM */
                if (++count >= argc) {
                    goto error;
                }
            /* ie. test MB , IO no slots */
            case CLI_TEST_MB: /* MB */
            case CLI_TEST_DATA_PLN: /* DATA_PLANE */
            case CLI_TEST_IO: /* IO */
                if (flag & MF_DOALL) {
                    goto error;
                }
                /* init the test target based on index */
                if (cli_get_test_target(index, argv[count], &tlist)) {
                    goto error;
                }

                flag |= MF_DOGRP;
                break;
            /* execute all the test in test menu items */
            case CLI_TEST_ALL: /* All */
                if (flag & MF_DOGRP) {
                    goto error;
                }

                flag |= MF_DOALL;
                break;
            /* toogle flag test environment */
            case CLI_TEST_DASH:
                str = argv[count];
                if (cli_check_option(str))
                {
                    goto error;
                }
                /* skip '-' */
                str++;
                while (str[0] != '\0') {
                    menu_flags(str[0]);
                    str++;
                }
                break;
            default:
                goto error;
        }
        count++;
    }

    /* setup test group */
    if (flag & MF_DOGRP) {
        cli_setup_testgrp(&tlist);
        cli_del_node(&tlist);
    }

    /* initialize */
    err_accum = 0;
    errcount = 0;
    testpass = 0;
    hkeepflags &= ~H_USRINT;
    initsigs();
    continuous = (DIAGFLAG & D_CONTINUOUS);
    if(continuous) {
        testpass = 1;
    }

    async_select = 0; /* LINGL: for 24m_diag.c get_async_clk() */
    switch (setjmp(menujmp)) {
        case 0:
            monjmpptr = &menujmp; /* redirect breaks */
            while(1) {
                 /*
                 * If in continuous mode AND the platform has a
                 * real time clock, display the time before execution
                 * of the item in each pass.
                 */
                if (((NVRAM)->diagflag & D_CONTINUOUS) && 
                    (menu_display_real_time != NULL)) {
                    (*menu_display_real_time)();
                }

                /* RJULIAN - This was originally in the diag file.
                 * Shouldn't we want it here too?
                 * I just saw it at the end.  Shouldn't a pre?? something
                 * be before?
                 * Before executing the menu item function, if
                 * pre_diag_exec is non-NULL, call it as a preliminary
                 * (e.g., to restore mempools on this platform).

                if (mb_board_type() == BDTYPE_CAVIUM_THREE_GORGES) {
                    if (pre_diag_exec) {
                        (*pre_diag_exec)();
                    }
                }
                                 */
                /* entry to to run the test */
                cli_dogrp_diags(flag);
    
                alarm(0); /* kill alarm if set */
                if (!continuous) {
                    break;
                }
                testpass++;  /* increment our pass count */
            }
            monjmpptr = savjmp;  /* redirect breaks back */
            break;
        case 1:   /* console break (out of run) */
            if (continuous) {
                menu_pr_err_accum();
            }
        default:  /* longjmp(menujmp, X > 1) */
            monjmpptr = savjmp;  /* redirect breaks back */
            break;
    }
    /* clean up the test group */
    if (flag & MF_DOGRP) {
        cli_cleanup_testgrp();
    }
    /* Reset the motherboard test without Ethernet to default value */
    if (index == CLI_TEST_MBWO) { 
       cli_reset_mb_wo();
    }
    
    /*
     * Before going back to diagmon, if
     * pre_diag_exec is non-NULL, call it as a preliminary
     * (e.g., to restore mempools on this platform).
     */
    if (pre_diag_exec) {
        (*pre_diag_exec)();
    }

    return (PASSED);

error:
    printf("usage: test [All | \n"
           "       {MB | MB-WO {E0|E1|E2} | DATA_PLN |\n" 
           "       | IO | VM <slot 1> | WIC <slot 1-%d> | \n" 
           "       SM <slot 1-%d>} | \n"
           "       -{A | C | D | E | L | M | O | P | Q | S | T | V | \n"
           "         W | X | U | Y}]\n", 
           get_max_wic_slots(),
           get_max_sm_slots());
    /* Reset the motherboard test without Ethernet to default value */
    if (index == CLI_TEST_MBWO) { 
        cli_reset_mb_wo();
    }
    /* delete the node */
    cli_del_node(&tlist);
    
    return (FAILED);
}

/*************************************************************************
 Function: auth
 *
 * This function is to authenticate smart chip Screen
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
auth (int argc, char *argv[])
{   
    testname("Auth");
    int count = 1, index = 0;
    sc_context *con, cont;
    cli_cookie_cmd cmd;
    char *keyword_1st[] = {"MB", "VM", "SM", "SM:DC", "WIC", "WIC:DC", "NM", "ISM", NULL};
    uchar cookie[COOKIE_SIZE_512];
    
    /* initialize */
    envflag = INDIAG; /* set the environment flag */
    
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->interface = SCC_I2C_IF;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
     /* get 1st level keyword, the target */
    index = cli_get_keyword(argv[count], keyword_1st);
    /*
      ie. auth MB , index is MB keyword
    */
    switch (index) {
        case CLI_AUTH_VM: /* VM */
        case CLI_AUTH_SM: /* SM */
        case CLI_AUTH_SMDC: /* SM */
        case CLI_AUTH_WIC: /* WIC */
        case CLI_AUTH_WICDC: /* WIC */
        case CLI_AUTH_NM: /* NM */
        case CLI_AUTH_ISM: /* ISM */
            if (++count >= argc) {
                goto error;
            }
        case CLI_AUTH_MB: /* MB */
            /* MB doesn't have slot */
            if ((index == 0) && (argc > 2))
            {
                goto error;
            }
            /* init the auth based on index */
            if (cli_get_auth_target(index, argv[count], &cmd, con)) {
                return (FAILED);
            }

            break;
        default:
            goto error;
    }

    plat_init_smart_eeprom_context(con, cmd.board_type, cmd.slot, 
                                           cookie);

    if(smart_cookie_authenticate_retest(con))
        {
            goto error;
        }

       
    return PASSED;
    
error:
    printf("usage: auth [MB | VM <slot 1> |\n"
           "       WIC <slot 1-%d> | SM <slot 1-%d> \n"
           "       WIC:DC <slot WIC>:<slot DC> | \n" 
           "       SM:DC <slot SM>:<slot DC> | \n",
          get_max_wic_slots(), get_max_sm_slots());
    
    return FAILED;
}

/*************************************************************************
 Function: quack
 *
 * This function is to program & authenticate digital signature.       
 * 
 * Input: argc, argv[]
 *
 * Output: none. 
 **************************************************************************
*/
int 
quack (int argc, char *argv[])
{   
    testname("Quack");
    int count = 1, index = 0;
    sc_context *con, cont;
    cli_cookie_cmd cmd;
    char *keyword_1st[] = {"MB", "VM", "SM", "SM:DC", "WIC", "WIC:DC", "NM", "ISM", NULL};
    uchar cookie[COOKIE_SIZE_512];
    
    /* initialize */
    envflag = INDIAG; /* set the environment flag */
    
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->interface = SCC_I2C_IF;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
     /* get 1st level keyword, the target */
    index = cli_get_keyword(argv[count], keyword_1st);
    /*
      ie. quack MB , index is MB keyword
    */
    switch (index) {
        case CLI_AUTH_VM: /* VM */
        case CLI_AUTH_SM: /* SM */
        case CLI_AUTH_SMDC: /* SM DC*/
        case CLI_AUTH_WIC: /* WIC */
        case CLI_AUTH_WICDC: /* WIC DC*/
        case CLI_AUTH_NM: /* NM */
        case CLI_AUTH_ISM: /* ISM */
            if (++count >= argc) {
                goto error;
            }
        case CLI_AUTH_MB: /* MB */
            /* MB doesn't have slot */
            if ((index == 0) && (argc > 2))
            {
                goto error;
            }

            if (cli_get_auth_target(index, argv[count], &cmd, con)) {
                return (FAILED);
            }
            
            break;
        default:
            goto error;
    }

    plat_init_smart_eeprom_context(con, cmd.board_type, cmd.slot, 
                                           cookie);

    act2_init_cont((void*)con);  /* must be called after plat_init */

    if (index == 0) {
        act2_prog(0);
    } else {
        if (smart_cookie_program_dig_sign_submenu_x(con, TRUE)) {
            goto error;
        }
    }
       
    return PASSED;
    
error:
    printf("usage: quack [MB | VM <slot 1> |\n"
           "       WIC <slot 1-%d> | SM <slot 1-%d> | \n" 
           "       WIC:DC <slot WIC>:<slot DC> | \n" 
           "       SM:DC <slot SM>:<slot DC> | \n",
          get_max_wic_slots(), get_max_sm_slots());
    
    return FAILED;
}

/*************************************************************************
 Function: cli_support_mb_wic
 *
 * This function is to check which platform support MB WICs.
 *
 * Input:
 *
 * Output: none.
 **************************************************************************
*/
static int
cli_support_mb_wic (void)
{
    return TRUE;
}

/*************************************************************************
 Function: cli_support_sm
 *
 * This function is to check which platform support SM.
 *
 * Input:
 *
 * Output: none.
 **************************************************************************
*/
static int
cli_support_sm (void)
{
    return TRUE;
}

/*************************************************************************
 Function: cli_support_psu
 *
 * This function is to check which platform support PSU.
 *
 * Input:
 *
 * Output: none.
 **************************************************************************
*/
static int
cli_support_psu (void)
{
    return TRUE;
}

/*************************************************************************
 Function: cli_support_cf
 *
 * This function is to check which platform support CF.
 *
 * Input:
 *
 * Output: none.
 **************************************************************************
*/
static int
cli_support_cf (void)
{
    printf("\n Overlord is not support CF %s \n", __FUNCTION__);
    return FALSE;
}

/*************************************************************************
 Function: cli_support_vm
 *
 * This function is to check which platform support VM.
 *
 * Input:
 *
 * Output: none.
 **************************************************************************
*/
static int
cli_support_vm (void)
{
    return TRUE;
}

/*************************************************************************
 * Function: volt_freq
 *
 * This function is to display platform voltage and frequency margin
 * Input: argc, argv[]
 *
 * Output: return PASSED / FAILED 
 **************************************************************************
*/
int 
volt_freq(int argc, char *argv[])
{   
    testname("cli volt freq");
	  
    if (argc > 1) {
        goto error;
    } else {
        show_margins_x(FALSE, DISPLAY_HCI);
        printf("\n\n");
        return (PASSED);
    }

error:
    printf("usage: voltfreq\n");
    return (FAILED);
}

/*************************************************************************
 * Function: vltmrgn
 *
 * This function is to do voltage margin
 * Input: argc, argv[]
 *
 * Output: return PASSED / FAILED
 * Note: Overlord have 2 voltage margin with 3 differnet modes, 
 * margin high, normal and margin low.
 * User can add another keyword content for supporting different platform.
 **************************************************************************
*/
int vltmrgn(int argc, char *argv[])
{
    testname("cli volt mrgn");
    int index, count = 1, type, level, rc = FAILED;
    char *keyword_1st_ovld[] = {"PSU", "DDR", "-", NULL};
    char *keyword_1st_usd[] = {"PSU1_0", "PSU1_2", "PSU2_5", "PSU3_3", "DDR", "-", NULL};
    char *str = NULL;
    char **keyword_1st;

    if (argc != 3) {
        goto error;
    } else {
        while(count < argc)
        {
            if (is_usd_machines()) {
                keyword_1st = keyword_1st_usd;
                /* get 1st level keyword */
                index = cli_get_keyword(argv[count], keyword_1st);

                switch (index) {
                case 0: /* PSU1_0 */
                    type = MRGN_1ST;
                    break;
                case 1: /* PSU1_2 */
                    type = MRGN_2ND;
                    break;
                case 2: /* PSU2_5 */
                    type = MRGN_3RD;
                    break;
                case 3: /* PSU3_3 */
                    type = MRGN_4TH;
                    break;
                case 4: /* DDR */
                    type = MRGN_5TH;
                    break;
                case 5: /* - */
                    str = argv[count];

                    if (cli_check_option(str))
                    {
                        goto error;
                    }
                    /* skip '-' */
                    str++;

                    if (strncmp(str, "H", strlen("H")) == 0){
                        level = MRGN_LV4;
                    } else
                    if (strncmp(str, "N", strlen("N")) == 0){
                        level = MRGN_LV3;
                    } else
                    if (strncmp(str, "L", strlen("L")) == 0){
                        level = MRGN_LV2;
                    } else {
                        level = 0; /* using for report error */
                        goto error;
                    }
                    break;

                default:
                    goto error;
                }
            } else { /* Overlord/Juno */
                keyword_1st = keyword_1st_ovld;
                /* get 1st level keyword */
                index = cli_get_keyword(argv[count], keyword_1st);

                switch (index) {
                case 0: /* PSU */
                    type = MRGN_1ST;
                    break;
                case 1: /* DDR */
                    type = MRGN_2ND;
                    break;
                case 2: /* - */
                    str = argv[count];

                    if (cli_check_option(str))
                    {
                        goto error;
                    }
                    /* skip '-' */
                    str++;

                    if (strncmp(str, "H", strlen("H")) == 0){
                        level = MRGN_LV4;
                    } else
                    if (strncmp(str, "N", strlen("N")) == 0){
                        level = MRGN_LV3;
                    } else
                    if (strncmp(str, "L", strlen("L")) == 0){
                        level = MRGN_LV2;
                    } else {
                        level = 0; /* using for report error */
                        goto error;
                    }
                    break;

                default:
                    goto error;
                }
            }

            count++;
        }
    }

    rc = vtg_mrgn_x(type, level);
    return (rc);

error:
    if (is_usd_machines()) {
        printf("usage: vmrgn [PSU1_0 -{H | N | L} | PSU1_2 -{H | N | L} |\
 PSU2_5 -{H | N | L} | PSU3_3 -{H | N | L} | DDR -{H | N | L}]\n");
    } else {
        printf("usage: vmrgn [PSU -{H | N | L} | DDR -{H | N | L}]\n");
    }
    printf("Margin H:high N:normal L:low level\n");
    return (rc);
}


/*************************************************************************
 * Function: freqmrgn
 *
 * This function is to do frequency margin
 * Input: argc, argv[]
 *
 * Output: return PASSED / FAILED
 * Note: Overlord have 1 frequency margin with 5 differnet modes.
 **************************************************************************
*/
int freqmrgn(int argc, char *argv[])
{
    testname("cli frq mrgn");
    boolean spread;
    int index, count = 1, type, level, rc = FAILED;
    char *keyword_1st[] = {"CLK", "-", "EN", "DIS", NULL};
    char *str = NULL;

    if (argc != 4) {
        goto error;
    } else {
        while(count < argc)
        {
            /* get 1st level keyword */
            index = cli_get_keyword(argv[count], keyword_1st);

            switch (index) {
            case 0: /* CLK */
                type = MRGN_1ST;
                break;
            case 1: /* - */
                str = argv[count];

                if (cli_check_option(str))
                {
                    goto error;
                }
                /* skip '-' */
                str++;

                /* LL will pass 'L' condition,
                 * L will not pass 'LL' condition.
                 * so we compare LL first than we compare L 
                 */
                if (strncmp(str, "HH", strlen("HH")) == 0){
                    level = MRGN_LV5;
                } else
                if (strncmp(str, "H", strlen("H")) == 0){
                    level = MRGN_LV4;
                } else
                if (strncmp(str, "N", strlen("N")) == 0){
                    level = MRGN_LV3;
                } else
                if (strncmp(str, "LL", strlen("LL")) == 0){
                    level = MRGN_LV1;
                } else 
                if (strncmp(str, "L", strlen("L")) == 0){
                    level = MRGN_LV2;
                } else {
                    level = 0; /* using for report error */
                    goto error;
                }
                break;
            case 2: /* EN */
                spread = ENABLE;
                break;
            case 3: /* DIS */
                spread = DISABLE;
                break;

            default:
                goto error;
             }

            count++;
        }
    }

    rc = freq_mrgn_x(type, level, spread);
    return (rc);

error:
    printf("usage: fmrgn [CLK {EN|DIS} -{HH | H | N | L | LL}]\n");
    printf("Margin HH:highest H:high N:normal L:low LL:lowerst level\n");
    printf("ENABLE/DISABLE spread on CLK\n");
    return (rc);
}


/* end of file cli_cmd.c */

/***********************************************************************
$Log: cli_cmd.c,v $
Revision 1.30  2018/02/09 09:11:18  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.29.24.1  2018/01/20 06:29:54  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.29.8.1  2017/10/13 22:39:53  hondwang
Add E5 for Wifi PCBA info

Revision 1.29  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.28  2014/06/03 10:53:32  erwu2
python menu collapsed to main trunk

Revision 1.27  2014/04/21 21:37:52  mcharon
replace get_hwic_cookie_id with get_cookie_id

Revision 1.26  2014/02/19 00:45:22  ptong
Deleted sys_regs.h

Revision 1.25  2014/02/11 09:52:52  hroni
enhance voltage margin support for USD

Revision 1.24  2013/12/18 06:32:45  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.23  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.22  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.21  2013/08/27 21:39:59  mcharon
user lower case to execute act2 menu. remove is_act2

Revision 1.20  2013/05/09 23:49:54  mcharon
support cli discovery of daughter card

Revision 1.19  2013/03/11 03:33:15  alpeng
supporting CLI for NGIO-DC

Revision 1.18  2013/02/22 03:44:56  alpeng
update EHWIC to WIC for supporting CLI test cmd

Revision 1.17  2013/01/31 10:48:45  alpeng
supported CLI cmds for voltage and freq margin

Revision 1.16  2012/11/17 01:15:17  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.15  2012/11/12 20:35:22  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.14  2012/10/08 08:42:07  alpeng
support discovery cmd for DC on NGWIC

Revision 1.13  2012/09/26 06:11:13  alpeng
display DIMM size on CLI discovery

Revision 1.12  2012/08/30 07:43:48  alpeng
infrom user with warning when device is vacant

Revision 1.11  2012/08/24 09:30:50  alpeng
adding check AC in and 12V output for psu cookie of CLI dicscovery cmd

Revision 1.10  2012/06/25 07:02:14  alpeng
revert method for storing diag flag

Revision 1.9  2012/06/12 16:54:05  ywen
Support cli cookie size 512 for NGWIC.

Revision 1.8  2012/06/05 09:33:44  aarwang
- Clean up compiler warnings.

Revision 1.7  2012/05/08 12:13:10  steja
Support cli cookie SM cookie size 512

Revision 1.6  2012/05/08 06:12:57  alpeng
change the CLI cmd from PVDM to VM

Revision 1.5  2012/05/04 20:01:45  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.4  2012/04/10 09:41:17  alpeng
support CLI cmd:disflag, setflag, repeat and history

Revision 1.3  2012/04/05 16:20:02  palin2
Add CLI command, "disrtc", support.

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

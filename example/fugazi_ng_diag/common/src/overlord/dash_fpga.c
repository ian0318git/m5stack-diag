/* $Id: dash_fpga.c,v 1.75 2020/01/31 07:17:40 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/dash_fpga.c,v $
 *------------------------------------------------------------------
 *
 * Filename: dash_fpga.c
  
 * Description: dash fpga related code
 * Copyright (c) 2014-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "queryflags.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "i2c_address.h"
#include "linux_api.h" /* print_offset_val */
#include "uio_utils.h"
#include "duart16552.h" /*01.09 add for fifo flush*/
#include "platform_sensor.h"
#include "ngio.h"
#include "nvmonvars.h"
#include "ethernet.h" /* for SFP definition */

#include "plat_defs.h" /* board_type */
#include "cross_platform.h" /* board_type */
#include "platform_cookie.h" /* get_plat_sku_cookie() */
#include "linux_pciutils.h"  /* for check juno plx */

extern int is_tam_aikido_on(void); 
static void disable_top_cp_intr(int bit);
static void enable_top_cp_intr(int bit);
static void clr_platform_poe_psu_intr(void);
extern uint32 cterr_db_print (char *fmtptr, ...);
/* static void clear_top_cp_msg_intr(int bit); */

//extern unsigned char dash_fpga_fw[];

static int board_type = BDTYPE_UNKNOWN;

/*****************************************************************************
 *
 * Function   : set_board_type
 * Description: set board type variable to distinguish overlord or juno 
 * Inputs     : none  
 *
 * Outputs    : none  
 *
 *****************************************************************************/
void
set_board_type (void)
{
    unsigned int fpga_ver = 0, fpga_brd = 0;
    unsigned int brd_type, brd_subtype;

    get_platform_ver(0, 0, &fpga_ver, 0, &fpga_brd);
    brd_type = ((fpga_brd & FPGA_BD_TYPE_MSK) >> FPGA_BD_TYPE_SHFT);
    brd_subtype = (((fpga_brd & FPGA_BD_SUBTYPE_HI_MSK) >> FPGA_BD_SUBTYPE_HI_SHFT) |
                   (fpga_brd & FPGA_BD_SUBTYPE_LO_MSK));

    if (brd_type == FPGA_BD_TYPE_ROUTE_PROC) {
        switch(brd_subtype) {
        case FPGA_BD_SUBTYPE_OVLD:
            board_type = BDTYPE_OVERLORD;
            printf("FPGA board type is OVERLORD\n");
            break;
        case FPGA_BD_SUBTYPE_JUNO:
            board_type = BDTYPE_JUNO;
            printf("FPGA board type is JUNO\n");
            break;
        case FPGA_BD_SUBTYPE_UTAH:
            board_type = BDTYPE_UTAH;
            printf("FPGA board type is UTAH\n");
            break;
        case FPGA_BD_SUBTYPE_SWORD:
            board_type = BDTYPE_SWORD;
            printf("FPGA board type is SWORD\n");
            break;
        case FPGA_BD_SUBTYPE_DAGGER:
            board_type = BDTYPE_DAGGER;
            printf("FPGA board type is DAGGER\n");
            break;
        case FPGA_BD_SUBTYPE_NEPTUNE:
        case FPGA_BD_SUBTYPE_NEPTUNE_TMP:
        case FPGA_BD_SUBTYPE_NEPTUNE_TMP_1:
            board_type = BDTYPE_NEPTUNE;
            printf("FPGA board type is NEPTUNE\n");
            break;
        case FPGA_BD_SUBTYPE_TRITON:
            board_type = BDTYPE_TRITON;
            printf("FPGA board type is TRITON\n");
            break;
        case FPGA_BD_SUBTYPE_PROTEUS:
            board_type = BDTYPE_PROTEUS;
            printf("FPGA board type is PROTEUS\n");
            break;
        case FPGA_BD_SUBTYPE_NESO:
            board_type = BDTYPE_NESO;
            printf("FPGA board type is NESO\n");
            break;
        case FPGA_BD_SUBTYPE_GOLDBEACH:
            board_type = BDTYPE_GOLDBEACH;
            printf("FPGA board type is GOLDBEACH\n");
            break;
        case FPGA_BD_SUBTYPE_NEPTUNIUM:
            board_type = BDTYPE_NEPTUNIUM; 
            printf("FPGA board type is NEPTUNIUM\n");
            break;
        case FPGA_BD_SUBTYPE_URANIUM:  
            board_type = BDTYPE_URANIUM;   
            printf("FPGA board type is URANIUM\n");
            break;
        case FPGA_BD_SUBTYPE_THORIUM:  
            board_type = BDTYPE_THORIUM;   
            printf("FPGA board type is THORIUM\n");
            break;
        case FPGA_BD_SUBTYPE_RADIUM:   
            board_type = BDTYPE_RADIUM;    
            printf("FPGA board type is RADIUM\n");
            break;
        case FPGA_BD_SUBTYPE_POLONIUM: 
            board_type = BDTYPE_POLONIUM;  
            printf("FPGA board type is POLONIUM\n");
            break;
        case FPGA_BD_SUBTYPE_THALLIUM: 
            board_type = BDTYPE_THALLIUM;  
            printf("FPGA board type is THALLIUM\n");
            break;
        case FPGA_BD_SUBTYPE_VG400:
            board_type = BDTYPE_VG400;
            printf("FPGA board type is Vg400\n");
            break;
        }
    }
    if (board_type == BDTYPE_UNKNOWN) {
        cterr('f',0,"FPGA board type unknown. brd type reg= %#.8x", fpga_brd);
    }
}

/*****************************************************************************
 *
 * Function   : mb_board_type
 * Description: This function returns the board type
 * Inputs     : none
 *
 * Outputs    : BDTYPE_OVERLORD  0x00
 *              BDTYPE_JUNO      0x01
 *
 *****************************************************************************/
int
mb_board_type (void)
{
    return(board_type);
}

/*******************************************************************************
 *
 * Function   : is_overlord / is_juno / is_utah / is_neptune etc
 *              neptune - high end than overlord
 *              triton - aka overlord-MLK, proteus - aka utah-MLK
 * Description: check the board type is overlord or juno
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
boolean
is_overlord (void)
{
    return (mb_board_type() == BDTYPE_OVERLORD);
}

boolean
is_juno (void)
{
    return (mb_board_type() == BDTYPE_JUNO);
}

int is_neptune (void)
{
    return (mb_board_type() == BDTYPE_NEPTUNE);
}

int is_triton (void)
{
    return (mb_board_type() == BDTYPE_TRITON);
}

int is_proteus (void)
{
    return (mb_board_type() == BDTYPE_PROTEUS);
}

int is_neso (void)
{
    return (mb_board_type() == BDTYPE_NESO);
}

int is_ntpn_machines (void)
{
    return((mb_board_type() == BDTYPE_NEPTUNE) ||
           (mb_board_type() == BDTYPE_TRITON) ||
           (mb_board_type() == BDTYPE_PROTEUS) ||
           (mb_board_type() == BDTYPE_NESO));
}

int is_vg450 (void)
{
    char sku_name[50];
    int rc = FALSE;

    /*
     * VG-450 uses the Neptune board and FPGA.
     * Only the PID field in the cookie can tell the difference.
     */
    if (is_neptune()) {
        memset(sku_name, '\0', sizeof(sku_name));
        get_mb_pid(sku_name);

        if (strncmp(sku_name, SKU_VG450_STR, strlen(SKU_VG450_STR)) == 0) {
            rc = TRUE;
        }
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : is_curie_1ru (including Radium, Polonium and Thallium)
 *              Radium - Juno MLK 
 *              Polonium - Sword MLK 
 * Description: check the board type is curie_1ru 
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_1ru (void) 
{
    if (is_radium() || is_polonium() || is_thallium()) {
        return (TRUE); 
    } else { 
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : is_curie_1ru_4ge_port (including Radium and Thallium)
 *              Radium - Juno MLK 
 *              Thallium - Dagger MLK 
 * Description: check the board type is curie_1ru 
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_1ru_4ge_port (void) 
{
    if (is_radium() || is_thallium()) {
        return (TRUE); 
    } else { 
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : is_curie_2ru (including Uranium and Thorium)
 * Description: check the board type is curie_2ru
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_2ru (void)
{
    if (is_uranium() || is_thorium()) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : is_curie (including Curie 1RU and 2RU)
 * Description: check the board type is curie
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie (void)
{
    if (is_curie_1ru() || is_curie_2ru()) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

#define CURIE2RU_P1B_BDREV  2

int is_curie_2ru_p1a(void)
{
    unsigned int bd_rev = 0;

    get_platform_bd_rev(&bd_rev);

    /* p1a = 1; p1b = 2; */
    if (is_curie_2ru() && bd_rev < CURIE2RU_P1B_BDREV) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int curie2ru_config_nim_ethx_1g(int slot, int onoff)
{
    int eth_port;
    char cmd[30];

    if (is_curie_2ru()) {
        switch (slot) {
        case 1:
            eth_port = 7;
            break;
        case 2:
            eth_port = 6;
            break;
        default:
            return (FALSE);
            break;
        }

        if (onoff) {
            sprintf(cmd, "ethtool -s eth%d autoneg off speed 1000", eth_port);
        } else {
            sprintf(cmd, "ethtool -s eth%d autoneg on", eth_port);
        }

        system(cmd);
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : is_curie_1ru_p1c_and_later
 * Description: check the board type is curie_1ru 
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_1ru_p1c_and_later(void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0, result = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    result =  (fpga_ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT; 

    /* p1a = 0; p1b = 1; p1c = 2; p2 = 3; */
    if (is_curie_1ru() && result > 1) { 
        return (TRUE); 
    } else {
        return (FALSE); 
    } 
}

/*******************************************************************************
 *
 * Function   : is_curie_1ru_p2_and_later
 * Description: check the board type is curie_1ru p2 or later build
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_1ru_p2_and_later(void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0, result = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    result =  (fpga_ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT; 

    /* p1a = 0; p1b = 1; p1c = 2; p2 = 3; */
    if (is_curie_1ru() && (result > 2)) { 
        return (TRUE); 
    } else {
        return (FALSE); 
    } 
}

/*******************************************************************************
 *
 * Function   : is_curie_1ru_p1c_and_older
 * Description: check the board type is curie_1ru p1c or older build
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_curie_1ru_p1c_and_older(void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0, result = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    result =  (fpga_ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT; 

    /* p1a = 0; p1b = 1; p1c = 2; p2 = 3; */
    if (is_curie_1ru() && result < 3) { 
        return (TRUE); 
    } else {
        return (FALSE); 
    } 
}

int is_neptunium (void)
{
    return (mb_board_type() == BDTYPE_NEPTUNIUM); 
}

int is_uranium (void)
{
    return (mb_board_type() == BDTYPE_URANIUM);
}

int is_thorium (void)
{
    return (mb_board_type() == BDTYPE_THORIUM); 
}

int is_radium (void)
{
    return (mb_board_type() == BDTYPE_RADIUM);
}

int is_polonium (void)
{
    return (mb_board_type() == BDTYPE_POLONIUM); 
}

int is_thallium (void)
{
    return (mb_board_type() == BDTYPE_THALLIUM); 
}

void prepare_pcie_sw_info (unsigned int *ngio_bus_num) {

    FILE *fp;
    char *fname = "/tmp/host_pci_device_id", cmd[32];
    struct pci_dev *dev;
    unsigned int dev_id[3] = {PLX_PCIE_SW_DID_8618, PLX_PCIE_SW_DID_8617,
                              PLX_PCIE_SW_DID_8604};
    unsigned short pcie_dev_id;
    unsigned int pcie_bus_num = 0, ia = 0, detected_id;
    boolean hit_plx_pcie_sw = FALSE;
    boolean hit_idt_pcie_sw = FALSE;
    unsigned short pcie_ven_id = PLX_PCIE_SW_VID;

    fp = fopen(fname, "r");
    if (fp == NULL) {
        sprintf(cmd, "touch %s ", fname);
        system(cmd);
        msleep(1);
        fp = fopen(fname, "w");
        if (fp == NULL) {
            printf("Failed to create %s\n", fname);
            return;
        }
       
        if (is_ntpn_machines() || is_vg450()) { 
             if ((hit_plx_pcie_sw == FALSE) && 
                 (hit_idt_pcie_sw == FALSE)) {
                pcie_ven_id = PERICOM_PCIE_SW_VID;
                pcie_dev_id = PERICOM_PCIE_SW_DID;
                dev = diag_pci_get_device(pcie_ven_id, pcie_dev_id, NULL);

                if (dev != NULL) {
                    /* get idt pcie switch */
                    pcie_bus_num = dev->bus;
                    detected_id = pcie_dev_id;
                } else {
                    printf("Cannot detect PERICOM PCIe switch \
                            on platform\n");
                    dev = NULL;
                    fclose(fp);
                    return;
                }
            }
        } else { 
            /* o2 and usd */
            /* check pcie sw is plx ? */
            for (ia = 0; ia < 3; ia++) {
                pcie_dev_id = dev_id[ia];
                dev = diag_pci_get_device(pcie_ven_id, pcie_dev_id, NULL);

                if (dev != NULL) {
                    /* get plx pcie switch */
                    hit_plx_pcie_sw = TRUE;
                    pcie_bus_num = dev->bus;
                    detected_id = pcie_dev_id;
                    break;
                }
            }

            /* there is no plx pcie device, it could be IDT pcie sw  */
            if (hit_plx_pcie_sw == FALSE) {
                pcie_ven_id = IDT_PCIE_SW_VID;
                pcie_dev_id = IDT_PCIE_SW_DID;
                dev = diag_pci_get_device(pcie_ven_id, pcie_dev_id, NULL);

                if (dev != NULL) {
                    /* get idt pcie switch */
                    hit_idt_pcie_sw = TRUE;
                    pcie_bus_num = dev->bus;
                    detected_id = pcie_dev_id;
                } 
            }
        }

        /* ngio bus number equal to (pcie bus num + 1) */
        *ngio_bus_num = pcie_bus_num + 1;
        dev = NULL;

        /* we write only once */
        fprintf(fp, "%x  %x", detected_id, *ngio_bus_num);
        fclose(fp);
    } else {
        fscanf(fp, "%x  %x", &detected_id, ngio_bus_num);
        fclose(fp);
    }

    return;
}

boolean 
is_plx (void)
{
    FILE *fp;
    char *fname = "/tmp/host_pci_device_id";
    uint device_id;
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru() || is_vg400()) {
        /* Goldbeach didn't have PCIe Switch*/
        return (FALSE);
    }
    fp = fopen(fname, "r");
    if (fp == NULL) {
         printf("Error: there is no pcie device id initialized \n");
         return (FALSE);
    }
    else {
        fscanf(fp, "%x", &device_id);
        fclose(fp);
    }

    if ((device_id == IDT_PCIE_SW_DID) ||
        (device_id == PERICOM_PCIE_SW_DID)) {
        return (FALSE);
    } else { 
        return (TRUE);
    }
}

/* for code consistence, we put is_juno_plx function here */
boolean
is_juno_plx (void)
{
    return((mb_board_type() == BDTYPE_JUNO) && is_plx()); 
}

int is_utah(void)
{
    return(mb_board_type() == BDTYPE_UTAH);
}

boolean
is_utah_plx (void)
{
    return((mb_board_type() == BDTYPE_UTAH) && is_plx()); 
}

boolean
is_not_plx(void)
{
    return (!is_plx());
}

boolean
is_plx_wrapper(void)
{
    return (is_plx());
}

int is_sword(void)
{
    return(mb_board_type() == BDTYPE_SWORD);
}

int is_dagger(void)
{
   return(mb_board_type() == BDTYPE_DAGGER);
}

int is_goldbeach(void)
{
    return(mb_board_type() == BDTYPE_GOLDBEACH);
}

int is_vg400(void)
{
    return(mb_board_type() == BDTYPE_VG400);
}

int is_usd_machines(void)
{
    return((mb_board_type() == BDTYPE_UTAH) ||
           (mb_board_type() == BDTYPE_SWORD) ||
           (mb_board_type() == BDTYPE_DAGGER));
}

int is_us_machines(void)
{
    return((mb_board_type() == BDTYPE_UTAH) ||
           (mb_board_type() == BDTYPE_SWORD)); 
}

int is_dg_machines(void)
{
    return((mb_board_type() == BDTYPE_DAGGER) ||
           (mb_board_type() == BDTYPE_GOLDBEACH)); 
}

/*Used not to display some items of LED test for Utah*/
int is_utah_false(void)
{
    return(mb_board_type() != BDTYPE_UTAH);
}

int exist_sm_slot1(void)
{
    return(is_utah() || is_sword());
}

/*******************************************************************************
 *
 * Function   : get_plat_sku_fpga
 * Description: Find out the platform SKU
 * Inputs     : None
 * Outputs    : SKU enum value
 *
 *******************************************************************************
 */
int get_plat_sku_fpga (void)
{
    uint board_type;

    board_type = mb_board_type();

    if (board_type == BDTYPE_OVERLORD) {
        return(SKU_ISR4451);
    } else if (board_type == BDTYPE_JUNO) {
        return(SKU_ISR4431);
    } else if (board_type == BDTYPE_UTAH) {
        return(SKU_ISR4351);
    } else if (board_type == BDTYPE_SWORD) {
        return(SKU_ISR4331);
    } else if (board_type == BDTYPE_DAGGER) {
        return(SKU_ISR4321);
    } else if (board_type == BDTYPE_NEPTUNE) {
        return(SKU_ISR4461);
    } else if (board_type == BDTYPE_TRITON) {
        return(SKU_ISR4452E);
    } else if (board_type == BDTYPE_PROTEUS) {
        return(SKU_ISR4452);
    } else if (board_type == BDTYPE_NESO) {
        return(SKU_ISR4432);
    } else if (board_type == BDTYPE_GOLDBEACH) {
        return(SKU_ISR4221);
    } else if (board_type == BDTYPE_URANIUM) {
        return(SKU_C8300_2N2S_4T2X);
    } else if (board_type == BDTYPE_THORIUM) {
        return(SKU_C8300_2N2S_6T);
    } else if (board_type == BDTYPE_RADIUM) {
        return(SKU_C8300_1N1S_4T2X);
    } else if (board_type == BDTYPE_THALLIUM) {  
        return(SKU_C8300_1N1S_6T);
    } else if (board_type == BDTYPE_VG400) {
        return(SKU_VG400);
    } else {
        printf("FPGA: Platform SKU unknown\n");
        return(SKU_INVALID);
    }

}

/*
 *********************************************************************
 *
 * Function   : chk_plat_sku
 * Description: This function compare the sku num between 
 *              FPGA and cookie. if not the same, using FPGA sku num.
 *              since FPGA always updated before cookie is programmed.
 *              and also pass platform fpga sku back.
 *                 
 * Inputs     : sku_num - platform sku number
 * Outputs    : Ture/False
 *
 *********************************************************************
 */
int chk_plat_sku (int *sku_num) {

    int sku_fpga = get_plat_sku_fpga();
    int sku_cookie = get_plat_sku_cookie();

    *sku_num = sku_fpga;

    if (sku_fpga == SKU_ISR4461) {
        if (sku_cookie == SKU_VG450) {
            printf("Platform SKU check passed.\n");
            return (TRUE);
        }
    }

    if (sku_fpga == sku_cookie) {
        printf("Platform SKU check passed.\n");
        return (TRUE);
    } else { 
        printf("Platform SKU check failed, FPGA is %d, cookie is %d.\n",
               sku_fpga, sku_cookie);
        printf("SKU_ISR4321 - %d\n", SKU_ISR4321);
        printf("SKU_ISR4331 - %d\n", SKU_ISR4331);
        printf("SKU_ISR4351 - %d\n", SKU_ISR4351);
        printf("SKU_ISR4431 - %d\n", SKU_ISR4431);
        printf("SKU_ISR4451 - %d\n", SKU_ISR4451);
        printf("SKU_ISR4461 - %d\n", SKU_ISR4461);
        printf("SKU_ISR4452E - %d\n", SKU_ISR4452E);
        printf("SKU_ISR4452 - %d\n", SKU_ISR4452);
        printf("SKU_ISR4432 - %d\n", SKU_ISR4432);
        printf("SKU_ISR4221 - %d\n", SKU_ISR4221);
        printf("SKU_C8300_2N2S_4T2X - %d\n", SKU_C8300_2N2S_4T2X);
        printf("SKU_C8300_2N2S_6T - %d\n", SKU_C8300_2N2S_6T);
        printf("SKU_C8300_1N1S_4T2X - %d\n", SKU_C8300_1N1S_4T2X);
        printf("SKU_C8300_1N1S_6T - %d\n", SKU_C8300_1N1S_6T);
        printf("SKU_VG400 - %d\n", SKU_VG400);
        return (FALSE);
    }
}

/*
 *********************************************************************
 *
 * Function   : get_plat_sku
 * Description: This function is based on FPGA to determine the plat SKU.
 *      
 * Inputs     : NONE
 * Outputs    : sku num
 *
 *********************************************************************
 */
int get_plat_sku (void) {

    return (get_plat_sku_fpga());
}

/*-------------------------------------------------------------------
 *
 * Function: byteswap32
 * 
 * wrapper function for dswap32. if we need to swap, this function wil
 * call dswap32.
 *
 * Input: int num; number to be swapped
 *
 * Output: org, swapped value
 *
 *-------------------------------------------------------------------
 */
static int
byteswap32 (int num)
{
    return num;
}

/*-------------------------------------------------------------------
 *
 * Function:
 * clean env alert (max1617) via write FPGA reg
 * 0x285 stand for (2 bytes|read|50Mhz|standard I2c mode)
 *
 *
 * Input: int addr; i2c addrss
 *
 * Output: return i2c status
 *
 *-------------------------------------------------------------------
 */
int
clean_env_alert (int addr)
{
    unsigned long base_addr = get_platform_i2c_addr(2); /* get 0x30200 */
    ovld_i2c_ctrl_t *i2c_ctrl = (ovld_i2c_ctrl_t *)base_addr;
 
    assert(dash_fpga);

    i2c_ctrl->sla_addr = addr;
    i2c_ctrl->ctrl = 0x285; /* perform read from FPGA */
    msleep(1);

    return (i2c_ctrl->stat);

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_intr_stat
 * get interrupt status from max1617
 *
 * Input: NONE
 *
 * Output: intr status
 *
 *-------------------------------------------------------------------
 */
int
get_platform_env_intr_stat (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    return (intr_sts_cntl->env_int_stat);

}

/*-------------------------------------------------------------------
 *
 * Function: enable_platform_env_intr
 * enable enviornmental control intr
 *
 * Input: NONE
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
enable_platform_env_intr (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    intr_sts_cntl->env_int_en |= EXT_ENV_INTR_EN;

    enable_top_cp_intr(FPGA_ENV_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function: disable_platform_env_intr
 * disable enviornmental control intr
 *
 * Input: NONE
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
disable_platform_env_intr (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    intr_sts_cntl->env_int_en &= ~EXT_ENV_INTR_EN;


    disable_top_cp_intr(FPGA_ENV_INTR);

    clear_snsr_alert();  /*need to clear the setting on Max1617 side*/

}


/*-------------------------------------------------------------------
 *
 * Function: get_platform_poe_psu_intr_stat
 * get interrupt status from poe psu
 *
 * Input: NONE
 *
 * Output: intr status
 *
 *-------------------------------------------------------------------
 */
int
get_platform_poe_psu_intr_stat (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    print_offset_val("get_poe_psu_intr:", (unsigned long)dash_fpga,
      (unsigned long)&intr_sts_cntl->poe_psu_int_stat,__LINE__, __FILE__);
      
    return (intr_sts_cntl->poe_psu_int_stat);
}

/*-------------------------------------------------------------------
 *
 * Function: enable_platform_poe_psu_intr
 * enable poe psu intr
 *
 * Input: poe_type - poe dc, poe2 output/present, poe1 output/present
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
enable_platform_poe_psu_intr (int poe_type)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    intr_sts_cntl->poe_psu_int_en |= poe_type;

    enable_top_cp_intr(FPGA_PWR_SUPPLY_INTR);
    
    /*
    print_offset_val("enable_poe_psu_intr:", (unsigned long)dash_fpga,
      (unsigned long)&intr_sts_cntl->poe_psu_int_en,__LINE__, __FILE__);
    */
}

/*-------------------------------------------------------------------
 *
 * Function: disable_platform_poe_psu_intr
 * disable poe psu intr
 *
 * Input: poe_type - poe dc, poe2 output/present, poe1 output/present
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
disable_platform_poe_psu_intr (int poe_type)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    intr_sts_cntl->poe_psu_int_en &= ~poe_type;

    disable_top_cp_intr(FPGA_PWR_SUPPLY_INTR);

    /* 
    print_offset_val("disable_poe_psu_intr:", (unsigned long)dash_fpga,
      (unsigned long)&intr_sts_cntl->poe_psu_int_en,__LINE__, __FILE__);
    */
}

/*-------------------------------------------------------------------
 *
 * Function: clr_platform_poe_psu_intr
 * clear poe psu intr
 *
 * Input: NONE
 * Output: NONE
 * NOTE: the register for poe psu intr is 'RC' 
 *       so we read the reigster to cleanup interrupt
 *       Only poe dc interrupt should be cleared on daughter card.
 *
 *-------------------------------------------------------------------
 */
void
clr_platform_poe_psu_intr (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;
    int dummy, count = 1000;

    assert(dash_fpga);

    do {
        /* read to clear interrupt */
        /* it will clear poe psu1/2 output/present interrupt */
        dummy = intr_sts_cntl->poe_psu_int_stat;
        count--;
        msleep(1);
    } while ((dummy & 0x55) && (count > 0));
    
    if (count == 0)
        printf("clr_platform_poe_psu_intr: Cannot clear intr poe_intr:0x%x \n", dummy); 
    return;
}

/*-------------------------------------------------------------------
 *
 * Function: poe_dc_intr_hndlr
 * Description: intr handler for poe dc. 
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
void poe_dc_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);

    printf("Call %s: irq:%d clr intr..", __FUNCTION__, irq);  
//    clr_platform_poe_psu_intr(); 
    printf("Fixed me: we need another func. to clean up poe dc intr.\n");

    printf("curr time:%s", ctime(&clk));

    return;
}

/*-------------------------------------------------------------------
 *
 * Function: poe2_output_intr_hndlr
 * Description: intr handler for poe2 output 
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
void poe2_output_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);

    printf("Call %s: irq:%d clr intr..", __FUNCTION__, irq);
    clr_platform_poe_psu_intr();
    printf("curr time:%s", ctime(&clk));

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: poe2_present_intr_hndlr
 * Description: intr handler for poe2 output 
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
void poe2_present_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);

    printf("Call %s: irq:%d clr intr..", __FUNCTION__, irq);
    clr_platform_poe_psu_intr();
    printf("curr time:%s", ctime(&clk));

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: poe1_output_intr_hndlr
 * Description: intr handler for poe1 output 
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
void poe1_output_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);

    printf("Call %s: irq:%d clr intr..", __FUNCTION__, irq);
    clr_platform_poe_psu_intr();
    printf("curr time:%s", ctime(&clk));

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: poe1_present_intr_hndlr
 * Description: intr handler for poe2 output 
 *
 * Input: irq - irq number  *arg - not used
 * Output: NONE
 *-------------------------------------------------------------------
 */
void poe1_present_intr_hndlr (int irq, void *arg) {

    time_t clk = time(NULL);

    printf("Call %s: irq:%d clr intr..", __FUNCTION__, irq);
    clr_platform_poe_psu_intr();
    printf("curr time:%s", ctime(&clk));

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: reset_plat_dev
 * reset device on control plan
 * cpld dev_rst_ctr register (0x1C)
 * following maskss are suported: 
 * FPGA_RST_USB_CONS, FPGA_RST_GE   
 * FPGA_RST_PCIE,
 * FPGA_RST_USB1_DIS--if 1 disables power
 * FPGA_RST_USB0_DIS--if 1 disbles power; 
 * FPGA_RST 
 * FPGA_RST_ACT2, FPGA_RST_FLASH    
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
reset_plat_dev (unsigned int mask)
{
    unsigned int tmp32;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;

    mask = byteswap32(mask);        

    /* to avoid effect previous running projects, 
     * we skip aikido here instead of common code; 
     * Since SW suggest do not reset AIKIDO, 
     * it is ready after rommon boot up. 
     */
    if (is_curie_1ru() || is_curie_2ru()) {
        if ((is_tam_aikido_on() == TRUE) && (mask == FPGA_RST_ACT2)) {
            return ;
        }
    }

    tmp32 = cpld->dev_rst_ctrl;
    tmp32 |= mask;        
    cpld->dev_rst_ctrl = tmp32;
}

/*-------------------------------------------------------------------
 *
 * Function: unreset_plat_dev
 * unreset device on control plan
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
unreset_plat_dev (unsigned int mask)
{
    unsigned int tmp32;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;

    mask = byteswap32(mask);        
    
    /* to avoid effect previous running projects, 
     * we skip aikido here instead of common code; 
     * Since SW suggest do not reset AIKIDO, 
     * it is ready after rommon boot up. 
     */
    if (is_curie_1ru() || is_curie_2ru()) {
        if ((is_tam_aikido_on() == TRUE) && (mask == FPGA_RST_ACT2)) {
            return ;
        }
    }

    tmp32 = cpld->dev_rst_ctrl;
    tmp32 &= ~mask;
    cpld->dev_rst_ctrl = tmp32;
}

/*-------------------------------------------------------------------
 *
 * Function: platform_irq1_test
 * 
 * test irq0 and irq0 intr test from cpld
 * make sure our KLM intr handler deassert the bit 
 *
 * Input: NONE
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
platform_irq0_test (void)
{
    unsigned int tmp32;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    /* write 0xCA to enable register */
    tmp32 = byteswap32((0xCA << 24) | (0x140000));
    cpld->tst = tmp32;

}


/*-------------------------------------------------------------------
 *
 * Function: get_platform_ver
 * 
 * get platform version
 * df ctrl0 reg offset 0x58 
 * for sel_platform_ctrl0_reg and unsel_platform_ctrl_reg
 * only these bits are valild
 * FPGA_SPI_DBG_SEL                   0x10000000 //RO
 * FPGA_STORED_SPI_SEL                0x20000000 ///RO
 * FPGA_BOOT_SPI_SEL                  0x40000000
 * FPGA_BOOT_SPI_SEL_OVRIDE           0x80000000 
 *
 * Input: verbose: if flag set to true then print version
 * Output: cpld_ver: cpld version
 *         cpld_Brd: cpld board revision
 *         fpga_brad: fpga brd rev
 *         always returns 0
 *
 *-------------------------------------------------------------------
 */
int
get_platform_ver (unsigned int verbose, unsigned int *cpld_ver,
                      unsigned int *fpga_ver, unsigned int *cpld_brd,
                      unsigned int *fpga_brd)
{
    unsigned int tmp32;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    //    assert(dash_cpld);
    assert(dash_fpga);
    tmp32 = fpga->ver;
    *fpga_ver = byteswap32(tmp32);
    tmp32 = fpga->brd;
    *fpga_brd = (tmp32);

   if (verbose) {
        printf("fpga version @%#lx=%#x; fpga brd info: @%#lx=%#x \n",
               (unsigned long)&fpga->ver - dash_fpga, *fpga_ver,
               (unsigned long)&fpga->brd - dash_fpga, *fpga_brd);
        printf("board rev=%#x; major=%#x; minor=%#x; debug rev=%#x\n",
               (fpga->ver & 0x07000000) >> 24,
               (fpga->ver & 0x007F0000) >> 16,
               (fpga->ver & 0x0000FF00) >> 8,
               fpga->ver & 0x000000FF);
    }
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_bd_rev
 * 
 * based on get_platform_ver() to return [26:24]@84
 *
 * Input:  brev - pointer of board revision
 * Output: none
 *
 *-------------------------------------------------------------------
 */
void get_platform_bd_rev (unsigned int *brev) 
{
    unsigned int dummy1 = 0, dummy2 = 0, fpga_ver_brdrev = 0, dummy3 = 0;

    get_platform_ver(0, &dummy1, &fpga_ver_brdrev, &dummy2, &dummy3);
    *brev = ((fpga_ver_brdrev & DASH_FPGA_HW_BRD_REV) >> DASH_FPGA_HW_BRD_OFF);
    return; 
}

/*-------------------------------------------------------------------
 *
 * Function: get_secure_jtag_status
 * 
 * Check FPGA register 0xA8 , the value = 0xC4 that mean
 * the Secure JTAG is Functioning.
 *
 * Input: None
 * Outputs    : Tur/False
 *
 *-------------------------------------------------------------------
 */
int get_secure_jtag_status (void)
{
    unsigned int tmp32, fpga_jtag ;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    assert(dash_fpga);
    tmp32 = fpga->jtg_sts;
    fpga_jtag = byteswap32(tmp32);
    /* BST Test will check the JTAG Status Register. */
    printf("FPGA Secure JTAG Reg:%#lx=%#x \n",
           (unsigned long)&fpga->jtg_sts - dash_fpga, fpga_jtag);
    if ((fpga_jtag & SECURE_JTAG_MASK) == SECURE_JTAG_WORK) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}
/*-------------------------------------------------------------------
 *
 * Function: sata_cfg
 *
 * Description: we have different fpga setting which is decided by
 *              different mode on overdrive.
 *
 * Input:  mode - current mode that user want to test.
 *
 * Output: none
 *
 *-------------------------------------------------------------------
 */
void
sata_cfg (boolean mode)
{
    unsigned int tmp;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    assert(dash_fpga);

    tmp = fpga->ext_pin_ctrl;
    
    if (mode) {
    /* Case1: passive mode */
    /* based on overdrive HFS 4.4, clear bit 4 and set bit 6 */
       tmp |= 0x40;
       tmp &= ~0x10;
    } else {
    /* Case2: controller mode */
    /* based on overdrive HFS 4.4, set bit 4 and set bit 6 */
       tmp |= 0x50;
    }

    /* o2 HFS, before write to FPGA external pin ctrl register
     * we need to write magic number 0xCA to bit [15:8]
     */
    tmp |= 0xCA00;

    fpga->ext_pin_ctrl = tmp;
    
#if 0
    if (fpga->ext_pin_ctrl & 0x2)
    /* Case2 */
        ;// printf("\nDash FPGA: NGWIC slots is in Controller mode \n");
    else
    /* Case1 */
        ;//printf("\nDash FPGA: NGWIC slot is in Pass thru mode \n");
#endif
    return;
}

boolean
is_sata_present (int sata_num)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    if (sata_num == SATA_NUM_ONE) {
        if (ngio->sata_ctrl & NGIO_SATA_ONE_PRSNT)
            return (TRUE);
        else
            return (FALSE);
    } else {
        /* SATA NUM TWO */
        if (ngio->sata_ctrl & NGIO_SATA_TWO_PRSNT)
            return (TRUE);
        else
            return (FALSE);
    }

}

/*-------------------------------------------------------------------
 *
 * Function: reset_platform_ext_dev
 *  sys level register; offset 0x4
 * 
 * reset ext devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 * FPGA_EXT_FP_PCIE_RST           0x200 
 * FPGA_FP_RST                0x100 
 * FPGA_EXT_POE_RST               0x80 
 * FPGA_EXT_BAR_RST               0x40 
 * FPGA_EXT_CLK_RST               0x20 
 * FPGA_EXT_I2C_MUX_RST           0x10 
 * FPGA_EXT_PCIE_SWITCH_HLT   0x8 
 * FPGA_EXT_PCIE_SWITCH_RST   0x4 
 * FPGA_EXT_GE_RST            0x2
 * FPGA_EXT_GE_QUAD_RESET         0x1
 *
 * ----------Neptune----------
 * FPGA_EXT_10GE_DUAL_RST         0x800
 * FPGA_EXT_PSU_I2C_MUX_RST       0x400 
 * FPGA_EXT_FP_PCIE_RST           0x200 
 * FPGA_FP_RST                    0x100 
 * FPGA_EXT_POE_RST               0x80 
 * Reserved
 * Reserved
 * FPGA_EXT_I2C_MUX_RST           0x10 
 * Reserved
 * FPGA_EXT_PCIE_SWITCH_RST       0x4 
 * FPGA_EXT_GE_RST                0x2
 * FPGA_EXT_GE_QUAD_RESET         0x1
 * 
 * OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
reset_platform_ext_dev (int bit)
{
    assert(dash_fpga);

    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->ext_rst |= bit;
}

/*-------------------------------------------------------------------
 *
 * Function: unreset_platform_ext_dev
 *  sys level register; offset 0x4
 * 
 * unreset ext devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 * FPGA_EXT_FP_PCIE_RST           0x200 
 * FPGA_FP_RST                0x100 
 * FPGA_EXT_POE_RST               0x80 
 * FPGA_EXT_BAR_RST               0x40 
 * FPGA_EXT_CLK_RST               0x20 
 * FPGA_EXT_I2C_MUX_RST           0x10 
 * FPGA_EXT_PCIE_SWITCH_HLT   0x8 
 * FPGA_EXT_PCIE_SWITCH_RST   0x4 
 * FPGA_EXT_GE_RST            0x2
 * FPGA_EXT_GE_QUAD_RESET         0x1
 *
 * ----------Neptune----------
 * FPGA_EXT_10GE_DUAL_RST         0x800
 * FPGA_EXT_PSU_I2C_MUX_RST       0x400 
 * FPGA_EXT_FP_PCIE_RST           0x200 
 * FPGA_FP_RST                    0x100 
 * FPGA_EXT_POE_RST               0x80 
 * Reserved
 * Reserved
 * FPGA_EXT_I2C_MUX_RST           0x10 
 * Reserved
 * FPGA_EXT_PCIE_SWITCH_RST       0x4 
 * FPGA_EXT_GE_RST                0x2
 * FPGA_EXT_GE_QUAD_RESET         0x1
 *
 * OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
unreset_platform_ext_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->ext_rst &= ~bit;
}

/*-------------------------------------------------------------------
 *
 * Function: reset_platform_in_dev
 *  sys level register; offset 0x8
 * 
 * reset int devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 *  FPGA_IN_NIOS_RST              0x1000000 
 *  FPGA_IN_I2C_15_RST            0x8000
 *  .
 *  .
 *  .
 *  FPGA_IN_I2C_0_RST             0x0001
 *  OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
reset_platform_in_dev (int bit, int print)
{
    volatile unsigned int *msg;
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    assert(dash_fpga);
    bit = byteswap32(bit);

    if (bit == FPGA_IN_NIOS_RST) {
        /* tell nios we are about to reset it */
        msg = (volatile unsigned int *)(dash_fpga + 0x34010);
        *msg = 1;
        sleep(1);
    }
    sys->in_rst |= bit;
    msleep(10);

    if (print) {
        printf("bit mask %#x\n", bit); /*0x1000000 */
        print_offset_val("reset", dash_fpga, (ulong)&sys->in_rst, __LINE__, 0);
    }
}

int
reset_nios (int bit, int print)
{
    volatile uint16_t *msg;
    volatile uint16_t *msg_status;
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    int count = 0;

    msg = (volatile uint16_t *)(dash_fpga + 0x34010);
    msg_status = (volatile uint16_t *)(dash_fpga + 0x34000);
        
    /* tell nios we are about to reset it */
    *msg = 1;
    sleep(1);

    /* check to make sure nios is prepared to be reset */
    for (count = 0; count < 10; count++) {
        sleep(1);
        if (*msg_status == 0x0001)
            break;
    }
    if ((*msg_status != 0x0001 )) {
        printf("NIOS status indicates NIOS is not ready for reset.\n");
     //   return(FAILED);
    }

    sys->in_rst |= bit;

    while (!(sys->in_rst & FPGA_IN_NIOS_RST_TAKEN)) {
        msleep(1);
        if (count++ > 1000)
            break;
    }
    if (!(sys->in_rst & FPGA_IN_NIOS_RST_TAKEN)) {
        printf("NIOS cannot be put into reset.\n");
        return(FAILED);
    }

    if (print) {
        printf("bit mask %#x\n", bit); /*0x1000000 */
        print_offset_val("reset", dash_fpga, (ulong)&sys->in_rst, __LINE__, 0);
    }
    return(PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: set_nios_mode
 *  to set nios mode to normal mode, disable mode, or diagnostic mode
 * 
 *
 * Input: mode: NIOS mode, NIOS_DISABLE_MODE (0), 
 *              NIOS_NORMAL_MODE (0x1), NIOS_DIAG_MODE (0x3)
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int 
set_nios_mode (int mode)
{
    volatile uint16_t *msg;
    volatile uint16_t *msg_status;
    volatile uint16_t *version;
    uint16_t msg_value, target_msg_status;
    int count = NIOS_MAX_RETRY, is_valid_mode;

    msg = (volatile uint16_t *)(dash_fpga + NIOS_MODE_REG); /* 0x34010 */
    msg_status = (volatile uint16_t *)(dash_fpga + NIOS_STATUS_REG); /* 0x34000 */
    version = (volatile uint16_t *)(dash_fpga + NIOS_VERSION_REG);


    /* HW suggest to simpify NIOS setup algorithm,
     *  10 times and 300000 us for each polling */
    if (is_curie_1ru() || is_curie_2ru() || is_usd_machines()) {
        for (count = 0; count < NIOS_MAX_RETRY; count++) {
            if (mode == NIOS_DISABLE_MODE) {
                if (*msg_status != NIOS_NORMAL_CHECK) {
                    break; 
                } 
            } else { /* normal mode */
                if (*msg_status == NIOS_NORMAL_CHECK) {
                    break; 
                } 
            }
            usleep(NIOS_POLLING_DELAY); 
            *msg = mode; 
        }

        if (count == NIOS_MAX_RETRY) { 
            printf("Failed to setup NIOS mode @0x34010 = %d\n", mode); 
            printf("NIOS status register @0x34000 = 0x%x\n", *msg_status); 
            return (FAILED); 
        } else {
            return (PASSED); 
        }
    }

    if (*version < NIOS_MIN_VERSION) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("NIOS version 0x%X\n", *version);
        }
        return (FAILED);
    }
    
    is_valid_mode = (mode == NIOS_DISABLE_MODE || mode == NIOS_NORMAL_MODE ||
                  mode == NIOS_DIAG_MODE);
    if (!is_valid_mode) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Unknown NIOS mode (0x%X)\n", mode);
        }
        return (FAILED);
    }

    /* set mode and check value */
    msg_value = mode;
    target_msg_status = (mode == NIOS_NORMAL_MODE) ? NIOS_NORMAL_CHECK : mode;

    for (count = 0; count < NIOS_MAX_RETRY; count++) {
        // printf("msg status %x, target %x\n",*msg_status, target_msg_status );
        /* check disable status */
        if (*msg_status == target_msg_status) {
            break;
        }
        /* enable/disable nios */
        *msg = msg_value;
        usleep(NIOS_POLLING_DELAY); 
    }
    if ((*msg_status != target_msg_status )) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("NIOS mode (%x) ignored.\n", mode);
        }
        return(FAILED);
    }

    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function: show_cpu_temperature
 * Description: get cpu temperature from nios mem register
 *
 * Input: none
 *
 * Output: none
 *
 *-------------------------------------------------------------------
 */
void show_cpu_temperature(void) {

    unsigned long addr = 0;
    assert(dash_fpga);

    /* 0x34000 + 0xD90 for cpu temperature on nios */
    addr = ((unsigned long)dash_fpga) + (NIOS_STATUS_REG + NIOS_CPU_TEMP_OFF);

    nios_mbox_mem_t *nios_cpu_temp = (nios_mbox_mem_t *)addr;
    uint32_t status = nios_cpu_temp->cpu_tmp;

    printf("CPU temperature : %d Celcius  \n", status);

    return;
}

/*-------------------------------------------------------------------
 *
 * Function: unreset_platform_in_dev
 *  sys level register; offset 0x8
 * 
 * unreset int devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 *  FPGA_IN_NIOS_RST              0x1000000 
 *  FPGA_IN_I2C_15_RST            0x8000
 *  .
 *  .
 *  .
 *  FPGA_IN_I2C_0_RST             0x0001
 *  OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
unreset_platform_in_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->in_rst &= ~bit;

}


/*-------------------------------------------------------------------
 *
 * Function : get_platform_top_intr
 * get interrupt status, can be any of following bits
 *
 *  FPGA_INTR_CTRL_REG_OFFSET
 *  FPGA_SYNC_ETH_PLL_STS    
 *  FPGA_GE_SYNC_INTR_STS    
 *  FPGA_CPU_CP_FP           
 *  FPGA_CPU_FP_CP           
 *  FPGA_CPU_CP_NIOS         
 *  FPGA_CPU_NIOS_CP         
 *  FPGA_PWR_SUPPLY_INTR_STS 
 *  FPGA_ENV_INTR_STS        
 *  FPGA_MISC_INTR_STS       
 *  FPGA_MOD_OIR_INTR_STS    
 *  FPGA_UART_INTR_STS       
 *  FPGA_C2W_INTR_STS        
 *  FPGA_SFP_INTR_STS
 * INPUT: none
 * OUTPUT: intr status
 * -------------------------------------------------------------------
*/
int
get_platform_top_intr (void)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    //    print_offset_val("intr sts @:", dash_fpga, (ulong)&intr_sts_cntl->sts, __LINE__, 0);
    return (intr_sts_cntl->top_sts);
}

/*-------------------------------------------------------------------
 *
 * Function : enable_top_cp_intr
 * enable interrupt to intel at top level
 * see fuction get_platform_intr_sts(int bit) for valid parameter.
 * input: bit. 
 * INPUT: bit representing interrupt type
 * OUTPUT: intr status
 * -------------------------------------------------------------------
*/
static void
enable_top_cp_intr (int bit)
{
    unsigned int plane = get_platform_plane();
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    bit = byteswap32(bit);

    intr_sts_cntl->top_en |= bit;

    uio_enable_intr();
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val("OIR EN", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->oir_en,
                         __LINE__,  __FILE__);
        print_offset_val("OIR STATUS", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->oir_sts,
                     __LINE__,  __FILE__);
        print_offset_val("TOP EN", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->top_en,
                         __LINE__,  __FILE__);
        print_offset_val("TOP STATUS", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->top_sts,
                         __LINE__,  __FILE__);
    }

}

/*-------------------------------------------------------------------
 *
 * Function : disable_top_cp_intr
 * disable interrupt to intel at top level
 * see fuction get_platform_intr_sts(int bit) for valid parameter.
 * input: bit. 
 * INPUT: bit representing interrupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
static void
disable_top_cp_intr (int bit)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    bit = byteswap32(bit);
    intr_sts_cntl->top_en &= ~bit;

}

/*-------------------------------------------------------------------
 *
 * Function : get_platform_sfp_intr_sts
 * get status of sfp interrupt
 * INPUT:  dev ...sfp number which can be from 0 to 3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
int
get_platform_sfp_intr_sts()
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    //    printf("function %s:  @%#x=%#x \n", __FUNCTION__, addr, byteswap32(intr_sts_cntl->sfp_sts));
    return (intr_sts_cntl->sfp_sts);
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_sfp_intr (int dev)
 * enable sfp interrupt
 * INPUT: dev ...sfp number which can be from 0 to 3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sfp_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    intr_sts_cntl->sfp_en |= dev;
    enable_top_cp_intr(FPGA_SFP_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_sfp_intr (int dev)
 * disable sfp interrupt
 * INPUT: dev ...sfp number which can be from 0 to 3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_sfp_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);
    intr_sts_cntl->sfp_en &= ~dev;

    clean_platform_sfp_override_intr(dev);
    disable_top_cp_intr(FPGA_SFP_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function : sfp_intr_hndlr
 * Description: intr hndlr for sfp
 * INPUT:  irq - irq number; p -- no use
 *         SFP has tx fault, loss sig and present interrupts
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
sfp_intr_hndlr (int irq, void *p)
{
    unsigned int sfp_sts;
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;

    /* check which sfp and which interrput is coming */
    switch (irq) {
        case SFP0:
            sfp_sts = sfp_stat_ctrl->sfp0_intr;
            break;
        case SFP1:
            sfp_sts = sfp_stat_ctrl->sfp1_intr;
            break;
        case SFP2:
            if (is_neptune() || is_vg450()) {
                sfp_sts = sfp_stat_ctrl->sfp_p0_intr;
            } else { 
                sfp_sts = sfp_stat_ctrl->sfp2_intr;
            }
            break;
        case SFP3:
            if (is_neptune() || is_vg450()) {
                sfp_sts = sfp_stat_ctrl->sfp_p1_intr;
            } else { 
                sfp_sts = sfp_stat_ctrl->sfp3_intr;
            }
            break;
        default:
            printf("error: not support this SFP port num %d\n", irq);
            break;
    }

    /* SFP interrupt status register will clean while read */

    if (sfp_sts & (SFP_TX_FAULT_INTR | SFP_LOSS_SIG_INTR | SFP_PRESENT_INTR)) {
        if (sfp_sts & SFP_TX_FAULT_INTR) 
            printf("\n\n****SFP%d TX Fault Interrupt Detect.\n\n", irq);
        if (sfp_sts & SFP_LOSS_SIG_INTR) 
            printf("\n\n****SFP%d Loss Signal Interrupt Detect.\n\n", irq);
        if (sfp_sts & SFP_PRESENT_INTR) 
            printf("\n\n****SFP%d Present Interrupt Detect.\n\n", irq);
    } else {
        printf("\n\n****SFP port %d unknown interrupt detect.\n", irq);
        printf("****interrupt register is 0x%x.\n", sfp_sts);
    }

    return;
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_sfp_override_intr
 * Description: write sfp override to force interrupt 
 * INPUT: dev - sfp number.  sfp0 to sfp3        
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sfp_override_intr (int dev)
{
    unsigned int sfp_intr_en, sfp_ovrid;
    unsigned int sfp_num;
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;

    /* we minus dev to 1 because dev start from 1, but sfp start from 0 */
    sfp_num = dev -1;

    /* prepare the 3 overrides for sfp interrupt */
    sfp_ovrid = SFP_TX_FAULT_INTR_OVERRIDE;
    sfp_ovrid |= SFP_LOSS_SIG_INTR_OVERRIDE;
    sfp_ovrid |= SFP_PRESENT_INTR_OVERRIDE;

    /* prepare the 3 interrupt types enable for sfp */
    sfp_intr_en = SFP_TX_FAULT_INTR_EN;
    sfp_intr_en |= SFP_LOSS_SIG_INTR_EN;
    sfp_intr_en |= SFP_PRESENT_INTR_EN;

    /* enable intr and override */
    switch(sfp_num) {
        case SFP0:
            sfp_stat_ctrl->sfp0_conf |= sfp_intr_en;
            msleep(10);
            sfp_stat_ctrl->sfp0_conf |= sfp_ovrid;
            break;
        case SFP1:
            sfp_stat_ctrl->sfp1_conf |= sfp_intr_en;
            msleep(10);
            sfp_stat_ctrl->sfp1_conf |= sfp_ovrid;
            break;
        case SFP2:
            if (is_neptune() || is_vg450()) {
                sfp_stat_ctrl->sfp_p0_conf |= sfp_intr_en;
                msleep(10);
                sfp_stat_ctrl->sfp_p0_conf |= sfp_ovrid;
            } else { 
                sfp_stat_ctrl->sfp2_conf |= sfp_intr_en;
                msleep(10);
                sfp_stat_ctrl->sfp2_conf |= sfp_ovrid;
            } 
            break;
        case SFP3:
            if (is_neptune() || is_vg450()) {
                sfp_stat_ctrl->sfp_p1_conf |= sfp_intr_en;
                msleep(10);
                sfp_stat_ctrl->sfp_p1_conf |= sfp_ovrid;
            } else { 
                sfp_stat_ctrl->sfp3_conf |= sfp_intr_en;
                msleep(10);
                sfp_stat_ctrl->sfp3_conf |= sfp_ovrid;
            } 
            break;
        default:
            printf("error: not support this SFP port num %d. \n", sfp_num);
            break;
    }

    return;
}

/*-------------------------------------------------------------------
 *
 * Function : clean_platform_sfp_override_intr
 * Description: clean sfp override register
 * INPUT: dev - sfp number.  sfp0 to sfp3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clean_platform_sfp_override_intr (int dev)
{
    uint32_t sfp_conf;
    unsigned int sfp_num = dev;
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;

    /* prepare the 3 overrides for sfp interrupt */
    sfp_conf = SFP_TX_FAULT_INTR_OVERRIDE;
    sfp_conf |= SFP_LOSS_SIG_INTR_OVERRIDE;
    sfp_conf |= SFP_PRESENT_INTR_OVERRIDE;
    sfp_conf |= SFP_TX_FAULT_INTR_EN;
    sfp_conf |= SFP_LOSS_SIG_INTR_EN;
    sfp_conf |= SFP_PRESENT_INTR_EN;

    /* check which sfp and which interrput is coming */
    switch(sfp_num) {
        case SFP0:
            sfp_stat_ctrl->sfp0_conf &= ~sfp_conf;
            break;
        case SFP1:
            sfp_stat_ctrl->sfp1_conf &= ~sfp_conf;
            break;
        case SFP2:
            if (is_neptune() || is_vg450()) {
                sfp_stat_ctrl->sfp_p0_conf &= ~sfp_conf;
            } else { 
                sfp_stat_ctrl->sfp2_conf &= ~sfp_conf;
            }
            break;
        case SFP3:
            if (is_neptune() || is_vg450()) {
                sfp_stat_ctrl->sfp_p1_conf &= ~sfp_conf;
            } else { 
                sfp_stat_ctrl->sfp3_conf &= ~sfp_conf;
            }
            break;
        default:
            printf("error: not support this SFP port num %d. \n", sfp_num);
    } 

    return;
}


/*-------------------------------------------------------------------
 *
 * Function : get_platform_i2c_sts
 * Description: get i2c status
 * INPUT: dev: c2w number. can be from 1 to 11 
 * OUTPUT: i2c status
 * -------------------------------------------------------------------
*/
int
get_platform_i2c_sts (void)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    assert(dash_fpga);

    //    printf("function %s:  @%#x=%#x \n", __FUNCTION__, addr, intr_sts_cntl->c2w_sts);
    
    return (intr_sts_cntl->c2w_sts);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_c2w_intr
 * Description: enable c2w interrupt
 * INPUT: dev: c2w number. can be from 1 to 11 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_c2w_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);

    dev = byteswap32(dev);
    
    intr_sts_cntl->c2w_en |= dev;
    print_offset_val("", dash_fpga, (ulong)&intr_sts_cntl->c2w_en, __LINE__,
                     __FILE__);

    enable_top_cp_intr(FPGA_I2C_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_c2w_override_intr
 * Description: enable c2w overide interrupt
 * INPUT: dev: c2w number. can be from 1 to 11 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_c2w_override_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    assert(dash_fpga);
    dev = byteswap32(dev);
    intr_sts_cntl->c2w_ovr |= dev;
    /*
    print_offset_val("", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->c2w_ovr,
                     __LINE__, __FILE__);
    */
    enable_platform_c2w_intr(dev);
        //    enable_top_cp_intr(FPGA_C2W_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : clear_platform_c2w_intr
 * Description: clear c2w  interrupt
 * INPUT: dev: c2w number. can be from 1 to 11 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clear_platform_c2w_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);

    dev = byteswap32(dev);

    intr_sts_cntl->c2w_sts &= ~dev;
    /*
    print_offset_val("clear_platform_c2w_intrr", dash_fpga, (ulong)&intr_sts_cntl->c2w_sts, __LINE__,
                     __FILE__);
    */
}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_c2w_intr
 * Description: disable c2w interrupt
 * INPUT: dev: c2w number. can be from 1 to 11 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_c2w_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);

    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->c2w_en &= ~dev;

    /* might as well disable override reg too */
    intr_sts_cntl->c2w_ovr &= ~dev;

    disable_top_cp_intr(FPGA_I2C_INTR);
    
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_wic_intr
 * Description: enable wic intr
 * INPUT: dev:  wic number, 1, 2, or 3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_wic_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);
    
    ngiowic_enable_intr(dev, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR);

    intr_sts_cntl->oir_en |= FPGA_OIR_NGWIC1 << (dev - 1);

    enable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_wic_override_intr
 * Description: enable wic override intr
 * INPUT: dev: wic number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_wic_oir_override_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    intr_sts_cntl->oir_ovr |= dev;

    enable_platform_wic_oir_intr(dev);

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_wic_intr
 * Description: disable wic OIR intr
 * INPUT: dev: wic number 1,2, or 3
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_wic_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);

    ngiosm_disable_intr(dev, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR);
    
    intr_sts_cntl->oir_en &= ~(FPGA_OIR_NGWIC1 << (dev - 1));
    intr_sts_cntl->oir_ovr &= ~dev;

    disable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_sm_oir_intr
 * Description: disable sm OIR intr
 * INPUT: dev: sm device, 1 or 2
 *  FPGA_OIR_SATA
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sm_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    ngiosm_enable_intr(dev, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR);
        
    intr_sts_cntl->oir_en |= dev;

    enable_top_cp_intr(FPGA_OIR_INTR);
    
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_sm_override_intr
 * Description: disable sm OIR override intr
 * INPUT: dev: wic number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sm_oir_override_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);
    
    printf("function %s:  @%#lx = %#x \n", __FUNCTION__, addr, byteswap32(intr_sts_cntl->oir_en));
    
    intr_sts_cntl->oir_ovr |= dev;
    enable_platform_sm_oir_intr(dev);
}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_sm_intr
 * Description: disable SM OIR intr
 * INPUT: dev: SM number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_sm_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);
    
    ngiosm_disable_intr(dev, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR);

    printf("function %s:  @%#lx = %#x \n", __FUNCTION__, addr, byteswap32(intr_sts_cntl->oir_en));
    intr_sts_cntl->oir_en &= ~dev;
    intr_sts_cntl->oir_ovr &= ~dev;

    disable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_pim_oir_intr
 * Description: enable PIM intr
 * INPUT: dev: pim number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_pim_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    ngiopim_enable_intr(0, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR); 

    intr_sts_cntl->oir_en |= FPGA_OIR_PIM; 

    enable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_pim_override_intr
 * Description: enable pim override intr
 * INPUT: dev: pim number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_pim_oir_override_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    intr_sts_cntl->oir_ovr |= FPGA_OIR_PIM; 

    enable_platform_pim_oir_intr(dev);

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_pim_intr
 * Description: disable pim OIR intr
 * INPUT: dev: pim number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_pim_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);
    ngiopim_disable_intr(0, NGIO_FLT_INTR | NGIO_INS_INTR | NGIO_REM_INTR);
    
    intr_sts_cntl->oir_en &= ~FPGA_OIR_PIM; 
    intr_sts_cntl->oir_ovr &= ~FPGA_OIR_PIM; 

    disable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_sata_oir_intr
 * Description: enable wic intr
 * INPUT: dev: wic number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sata_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    ngiosata_enable_intr(0,NGIO_SATA2_INS | NGIO_SATA2_RMV |
                         NGIO_SATA1_INS | NGIO_SATA1_RMV);

    intr_sts_cntl->oir_en |= FPGA_OIR_SATA;

    enable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_wic_override_intr
 * Description: enable wic override intr
 * INPUT: dev: wic number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_sata_oir_override_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    dev = byteswap32(dev);

    intr_sts_cntl->oir_ovr |= FPGA_OIR_SATA;

    enable_platform_sata_oir_intr(dev);

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_wic_intr
 * Description: disable wic OIR intr
 * INPUT: dev: wic number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_sata_oir_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    dev = byteswap32(dev);
    ngiosata_disable_intr(0, NGIO_SATA2_INS | NGIO_SATA2_RMV |
                          NGIO_SATA1_INS | NGIO_SATA1_RMV);
    
    intr_sts_cntl->oir_en &= ~FPGA_OIR_SATA;
    intr_sts_cntl->oir_ovr &= ~FPGA_OIR_SATA;

    disable_top_cp_intr(FPGA_OIR_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : get_blink_status
 * Description: get blink status.
 * INPUT: type: type of led can be any of following values
 * LED_CTRL_MISC                 0x0
 * LED_CTRL_CF                   0x1
 * LED_CTRL_PWR                  0x2
 * LED_CTRL_POE_PWR              0x3
 * LED_CTRL_POE_DAUGH            0x4
 * LED_CTRL_HD_DRIVER  y          0x5
 * LED_CTRL_BLINK_DURA           0x6
 * LED_CTRL_RJ45_BLINK_EN        0x7
 * LED_CTRL_ETH_BLINK_EN         0x8
 * LED_CTRL_RJ45_ONOFF           0x9
 * LED_CTRL_SFP_ONOFF            0xA
 * LED_CTRL_MGMT_ONOFF           0xB
 * LED_CTRL_DEBUG                0xC
 * OUTPUT: lede status
 * -------------------------------------------------------------------
*/
unsigned int
get_blink_status (int type)
{
    unsigned long addr = get_platform_led_ctrl_base();
    led_t *led_ctrl = (led_t *)addr;
    
    return (led_ctrl->eth_blink_en & type);
}

/*-------------------------------------------------------------------
 *
 * Function : get_led_status
 * Description: get led  status.
 * INPUT: type: type of led .
 * OUTPUT: led status
 * -------------------------------------------------------------------
*/
int 
get_led_status (int dev)
{
    unsigned int value;
    unsigned long addr = get_platform_led_ctrl_base();
    led_t *led_ctrl = (led_t *)addr;
       
    assert(dash_fpga);    
    
    switch(dev) {
       case LED_CTRL_MISC:
         value = led_ctrl->misc;
       break;
       case LED_CTRL_CF:
         value = led_ctrl->cf;
       break;
       case LED_CTRL_PWR:
         value = led_ctrl->pwr;
       break;
       case LED_CTRL_POE_PWR:
         value = led_ctrl->poe_pwr;
       break;
       case LED_CTRL_POE_DAUGH:
         value = led_ctrl->poe_daugh;
       break;       
       case LED_CTRL_HD_DRIVER:
         value = led_ctrl->hd_driver;
       break;
       case LED_CTRL_ENV:
         value = led_ctrl->env;
       break;
       case LED_CTRL_BLINK_DURA:
         value = led_ctrl->blink_duration;
       break;
       case LED_CTRL_RJ45_BLINK_EN:
         value = led_ctrl->rj45_blink_en;
       break;
       case LED_CTRL_ETH_BLINK_EN:
         value = led_ctrl->eth_blink_en;
       break;
       case LED_CTRL_RJ45_ONOFF:
         value = led_ctrl->rj45_onoff;
       break;
       case LED_CTRL_SFP_ONOFF:
         value = led_ctrl->sfp_onoff;
       break;
       case LED_CTRL_MGMT_ONOFF:
         value = led_ctrl->mgmt_onoff;
       break;
       case LED_CTRL_DEBUG:
         value = led_ctrl->debug;
       break;
      default:
         //printf("default \n");  
       break;
    }
    
    return (value);
}

/*-------------------------------------------------------------------
 *
 * Function : set_led_reg
 * Description: set led regiser
 * INPUT: type: type of device .
 *         bit: bit to set
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
set_led_reg (int dev, int bit)
{
    unsigned long addr = get_platform_led_ctrl_base();
    led_t *led_ctrl = (led_t *)addr;

    assert(dash_fpga);
    bit = byteswap32(bit);
    switch(dev) {
       case LED_CTRL_MISC:
         led_ctrl->misc = bit;
       break;
       case LED_CTRL_CF:
         led_ctrl->cf = bit;
       break;
       case LED_CTRL_PWR:
         led_ctrl->pwr = bit;
       break;
       case LED_CTRL_POE_PWR:
         led_ctrl->poe_pwr = bit;
       break;
       case LED_CTRL_POE_DAUGH:
         led_ctrl->poe_daugh = bit;
       break;       
       case LED_CTRL_HD_DRIVER:
         led_ctrl->hd_driver = bit;
       break;
       case LED_CTRL_ENV:
         led_ctrl->env = bit;
       break;
       case LED_CTRL_BLINK_DURA:
         led_ctrl->blink_duration = bit;
       break;
       case LED_CTRL_RJ45_BLINK_EN:
         led_ctrl->rj45_blink_en = bit;
       break;
       case LED_CTRL_ETH_BLINK_EN:
         led_ctrl->eth_blink_en = bit;
       break;
       case LED_CTRL_RJ45_ONOFF:
         led_ctrl->rj45_onoff = bit;
       break;
       case LED_CTRL_SFP_ONOFF:
         led_ctrl->sfp_onoff = bit;
       break;
       case LED_CTRL_MGMT_ONOFF:
         led_ctrl->mgmt_onoff = bit;
       break;
       case LED_CTRL_DEBUG:
         led_ctrl->debug = bit;
       break;
      default:
         //printf("default \n");  
       break;
    }
}

/*-------------------------------------------------------------------
 *
 * Function : set_led_off
 * Description: turn off led
 * INPUT: type: type of device .
 *         bit: bit to set
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
set_led_off (int dev, int bit)
{
    unsigned long addr = get_platform_led_ctrl_base();
    led_t *led_ctrl = (led_t *)addr;

    assert(dash_fpga);
    bit = byteswap32(bit);
    switch(dev) {
       case LED_CTRL_MISC:
         led_ctrl->misc &= ~bit;
       break;
       case LED_CTRL_CF:
         led_ctrl->cf &= ~bit;
       break;
       case LED_CTRL_PWR:
         led_ctrl->pwr &= ~bit;
       break;
       case LED_CTRL_POE_PWR:
         led_ctrl->poe_pwr &= ~bit;
       break;
       case LED_CTRL_POE_DAUGH:
         led_ctrl->poe_daugh &= ~bit;
       break;       
       case LED_CTRL_HD_DRIVER:
         led_ctrl->hd_driver &= ~bit;
       break;
       case LED_CTRL_BLINK_DURA:
         led_ctrl->blink_duration &= ~bit;
       break;
       case LED_CTRL_RJ45_BLINK_EN:
         led_ctrl->rj45_blink_en &= ~bit;
       break;
       case LED_CTRL_ETH_BLINK_EN:
         led_ctrl->eth_blink_en &= ~bit;
       break;
       
       case LED_CTRL_RJ45_ONOFF:
         led_ctrl->rj45_onoff &= ~bit;
       break;
       case LED_CTRL_SFP_ONOFF:
         led_ctrl->sfp_onoff &= ~bit;
       break;
       case LED_CTRL_MGMT_ONOFF:
         led_ctrl->mgmt_onoff &= ~bit;
       break;
       case LED_CTRL_DEBUG:
         led_ctrl->debug &= ~bit;
       break;
      default:
          //printf("default \n");
       break;
    }
}

/*-------------------------------------------------------------------
 *
 * Function : get_platform_uart_mux_ctrl_reg
 * Description: get value of mux ctrl reg of uart
 * INPUT: NONE
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
unsigned int
get_platform_uart_mux_ctrl_reg (void)
{
    unsigned long addr = get_platform_uart_mux_addr();
    console_t *uart_mux_cntl = (console_t *)addr;

    assert(dash_fpga);

    return (uart_mux_cntl->multiplex);

}

/*-------------------------------------------------------------------
 *
 * Function : switch_console_usb
 * Description: switch console of usb
 * INPUT: source, type of consoel
 * OUTPUT: passed or failed
 * -------------------------------------------------------------------
*/
int 
switch_console_usb (int source)
{
    unsigned long addr = get_platform_uart_mux_addr();
    console_t *uart_mux_cntl = (console_t *)addr;
    int bit = 0;
    
    assert(dash_fpga);
    bit = byteswap32(MUX_REG_USB_MUX_SEL | MUX_REG_USB_MANUAL_MUX_SEL);
    //bit = byteswap32(bit);

    if(source == USB_CONSOLE_SRC) {
    	  uart_mux_cntl->multiplex |= bit;
    } else if (source == RJ45_CONSOLE_SRC) {
        uart_mux_cntl->multiplex &= ~bit;
    } else {
    	cterr('f', 0, "failed on parse console source\n");
    	return FAILED;
    }
    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_uart_intr 
 * Description: enable uart interrupt
 * INPUT: dev, uart number, from 0 to 7
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_uart_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);

    dev = byteswap32(dev);
    
    intr_sts_cntl->uart_en |= dev;

    print_offset_val("", dash_fpga, (ulong)&intr_sts_cntl->uart_en, __LINE__,
                     __FILE__);
        

    enable_top_cp_intr(FPGA_UART_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_uart_override_intr
 * Description: enable uart override interrupt
 * INPUT: dev, uart number, from 0 to 7
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_uart_override_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    assert(dash_fpga);
    dev = byteswap32(dev);
    intr_sts_cntl->uart_ovr |= dev;
    /*
    print_offset_val("", (unsigned long)dash_fpga, (unsigned long)&intr_sts_cntl->uart_ovr,
                     __LINE__, __FILE__);
    */
    enable_platform_uart_intr(dev);
        //enable_top_cp_intr(FPGA_UART_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function : clear_platform_uart_intr
 * Description: clear uart interrupt
 * INPUT: dev, uart number, from 0 to 7
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clear_platform_uart_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);

    dev = byteswap32(dev);

    intr_sts_cntl->uart_sts &= ~dev;
    /*
    print_offset_val("clear_platform_c2w_intrr", dash_fpga, (ulong)&intr_sts_cntl->uart_sts, __LINE__,
                     __FILE__);
    */

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_uart_intr
 * Description: disable uart interrupt
 * INPUT: dev, uart number, from 0 to 7
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_uart_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->uart_en &= ~dev;

    /* disable uart override too just in case */
    intr_sts_cntl->uart_ovr &= ~dev;

    disable_top_cp_intr(FPGA_UART_INTR);
    
}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_uart_ovr_intr
 * Description: disable uart override interrupt
 * INPUT: dev, uart number, from 0 to 7
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_uart_ovr_intr (int dev)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    /* disable uart override too just in case */
    intr_sts_cntl->uart_ovr &= ~dev;
    
}

/*-------------------------------------------------------------------
 *
 * Function : enable_poe_psu_intr
 * Description: enable PSU and ENV interrupt
 * INPUT: bit : poe number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_poe_psu_intr (int bit)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);
    bit = byteswap32(bit);

    intr_sts_cntl->poe_psu_int_en |= bit;

}

/*-------------------------------------------------------------------
 *
 * Function : disable_poe_psu_intr
 * Description: disable PSU and ENV interrupt
 * INPUT: bit : interrupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_poe_psu_intr (int bit)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);
    bit = byteswap32(bit);

    intr_sts_cntl->poe_psu_int_en &= ~bit;

    print_offset_val("disable_poe_psu_intr:", (unsigned long)dash_fpga,
          (unsigned long)&intr_sts_cntl->poe_psu_int_en,__LINE__, __FILE__);
}

/*-------------------------------------------------------------------
 *
 * Function : get_poe_psu_intr
 * Description: get poe power suppy intr status
 * INPUT: bit : NONE
 * OUTPUT: status
 * -------------------------------------------------------------------
*/
int
get_poe_psu_intr (void)
{
    unsigned long addr = get_platform_ps_env_base();
    psu_t *intr_sts_cntl = (psu_t *)addr;

    assert(dash_fpga);

    print_offset_val("get_poe_psu_intr:", (unsigned long)dash_fpga,
          (unsigned long)&intr_sts_cntl->poe_psu_int_stat,__LINE__, __FILE__);

    return (intr_sts_cntl->poe_psu_int_stat);
}

#ifndef UTAH
/*-------------------------------------------------------------------
 *
 * Function : enable_platform_mcu_intr
 * Description: enable Environmental MCU interrupt 
 * INPUT: bit : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_mcu_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_env_mcu_base(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->misc_intr |= FPGA_MISC_ENV_MCU;
    mcu->intr_en    |=  ENV_MCU_RX_DATA | ENV_MCU_TX_DONE;
    
    /*
    print_offset_val("enable_platform_mcu_intr:", (unsigned long)dash_fpga,
                     (unsigned long)&intr_sts_cntl->misc_intr,
                     __LINE__, __FILE__);
    */
    
    enable_top_cp_intr(FPGA_MISC_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_mcu_override_intr
 * Description: enable Environmental MCU override interrupt
 * INPUT: dev : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_mcu_override_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->misc_ovr |= FPGA_MISC_ENV_MCU;
    /*
    print_offset_val("enable_platform_mcu_intr:", (unsigned long)dash_fpga,
                     (unsigned long)&intr_sts_cntl->misc_intr,
                     __LINE__, __FILE__);
    */
    enable_platform_mcu_intr(dev);
    //    enable_top_cp_intr(FPGA_MISC_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : clear_platform_mcu_override_intr
 * Description: clear Environmental MCU override interrupt
 * INPUT: dev : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clear_platform_mcu_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long mcu_addr = get_platform_env_mcu_base(plane);
    //    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    volatile unsigned int sts;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    sts = mcu->sts;
    mcu->sts &= ~(sts);  /* level 3 */

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_mcu_override_intr
 * Description:  disable Environmental MCU interrupt
 * INPUT: dev : mcu interupt type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_mcu_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_env_mcu_base(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    mcu->intr_en    &= ~( ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    intr_sts_cntl->misc_ovr &= ~FPGA_MISC_ENV_MCU;

    disable_top_cp_intr(FPGA_MISC_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_vm_mcu_intr
 * Description:   enable VOLTAGE MCU interrupt
 * INPUT: dev : vm mcu type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_vm_mcu_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_vm_base(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->misc_intr |= FPGA_MISC_VM_MCU;
    mcu->intr_en    |=  ENV_MCU_RX_DATA | ENV_MCU_TX_DONE;
    
    enable_top_cp_intr(FPGA_MISC_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function :enable_platform_vm_mcu_override_intr 
 * Description:   enable VOLTAGE MCU override interrupt
 * INPUT: dev : vm mcu type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_vm_mcu_override_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    intr_sts_cntl->misc_ovr |= FPGA_MISC_VM_MCU;

    enable_platform_mcu_intr(dev);


}

/*-------------------------------------------------------------------
 *
 * Function : clear_platform_vm_mcu_override_intr 
 * Description:   clear VOLTAGE MCU override interrupt
 * INPUT: dev : vm mcu type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clear_platform_vm_mcu_intr (int dev)
{
    
    unsigned int plane = CP;
    unsigned long mcu_addr = get_platform_vm_base(plane);

    volatile unsigned int sts;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    sts = mcu->sts;
    mcu->sts &= ~(sts);  /* level 3 */

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_vm_mcu_override_intr 
 * Description:   disable VOLTAGE MCU override interrupt
 * INPUT: dev : vm mcu type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_vm_mcu_intr (int dev)
{
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    unsigned long mcu_addr = get_platform_vm_base(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    env_dwnld_t *mcu = (env_dwnld_t *)mcu_addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    mcu->intr_en    &= ~( ENV_MCU_RX_DATA | ENV_MCU_TX_DONE);

    intr_sts_cntl->misc_ovr &= ~FPGA_MISC_VM_MCU;
    
    disable_top_cp_intr(FPGA_MISC_INTR);

}
#endif

/*-------------------------------------------------------------------
 *
 * Function : get_platofrm_uart_sts
 * Description:  get uart status.
 * INPUT: dev : uart number
 * OUTPUT: uart status
 * -------------------------------------------------------------------
*/
int
get_platform_uart_sts (int dev)
{
    uart_t *uart;
    unsigned int plane = CP;
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    assert(dash_fpga);
    dev = byteswap32(dev);

    uart = (uart_t *)get_platform_uart_addr(dev);
    uart->dlm = 0xFFFF;
    printf("uart%d", dev);
    print_offset_val("status:", dash_fpga, (ulong)&intr_sts_cntl->top_sts, __LINE__, 0);
    print_offset_val("ier:", dash_fpga, (ulong)&uart->dlm, __LINE__, 0);
    print_offset_val("lcr:", dash_fpga, (ulong)&uart->lcr, __LINE__, 0);

    return (intr_sts_cntl->uart_sts & dev);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_cp_intr_misc
 * Description: enable platform misc interrupt
 * INPUT: bit : misc type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_cp_intr_misc (int bit)
{
    unsigned int plane = get_platform_plane();
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    
    bit = byteswap32(bit);

    if (intr_sts_cntl->misc_intr & bit) {
        /* already enable, so do nothing...*/
    } else {
        intr_sts_cntl->misc_intr |= bit;
    }

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_cp_intr_misc
 * Description: disable platform misc interrupt
 * INPUT: bit : misc type
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_cp_intr_misc (int bit)
{
    unsigned long addr = get_platform_intr_ctrl_addr(CP);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;

    bit = byteswap32(bit);

    intr_sts_cntl->top_en &= ~bit;

}

/*-------------------------------------------------------------------
 *
 * Function : dash_reset_ext
 * Description: dsiplay option to reset ext device
 * INPUT: val, not used
 * OUTPUT: always return PASSED
 * -------------------------------------------------------------------
*/
int
dash_reset_ext (int val)
{
    unsigned char c;
    unsigned int mask = 0;
    unsigned int d = 0;
        printf("Enter '0' for Cavium CPU PCIe Reset.\n");
        printf("Enter '1' for Cavium CPU Reset.\n");
        printf("Enter '2' for POE Daughter Card Reset.\n");
        printf("Enter '3' for Baromater Reset.\n");
        printf("Enter '4' for Ext SyncE Clock Reset.\n");
        printf("Enter '5' for I2C Mux Reset.\n");
        printf("Enter '6' for Ext Swtich Reset Halt.\n");
        printf("Enter '7' for Ext PCIe Swtich Reset.\n");
        printf("Enter '8' for Ext GE Reset.\n");
        printf("Enter '9' for Ext GE QUAD Reset.\n");
        c = getchar();
        switch (c) {
        case '0':
            mask = (FPGA_EXT_FP_PCIE_RST);
            break;
        case '1':
            mask = (FPGA_EXT_FP_RST);
            break;
        case '2':
            mask = FPGA_EXT_POE_RST;
            break;
        case '3':
            mask = FPGA_EXT_BAR_RST;
            break;
        case '4':
            mask = FPGA_EXT_CLK_RST;
            break;
        case '5':
            mask = FPGA_EXT_I2C_MUX_RST;
            break;
        case '6':
            mask = FPGA_EXT_PCIE_SWITCH_HLT;
            break;
        case '7':
            mask = FPGA_EXT_PCIE_SWITCH_HLT;
            break;
        case '8':
            mask = FPGA_EXT_PCIE_SWITCH_RST;
            break;
        case '9':
            mask = FPGA_EXT_GE_RST;
            break;
        case 'a':
            mask = FPGA_EXT_GE_QUAD_RST;
        case 'b':
            d = gethex_answer("Enter C2W device number. "
                              "Enter 17 to reset all.\n", 0,
                              0, 100);
            mask = FPGA_IN_I2C_0_RST << d;
            break;
        }

        if (val) {
            reset_platform_ext_dev(mask);
        } else {
            unreset_platform_ext_dev(mask);
        }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : dash_reset_int
 * Description: dash reset internal device
 * INPUT: val, not used
 * OUTPUT: always return PASSED
 * -------------------------------------------------------------------
*/
int
dash_reset_int (int val)
{
    unsigned int c;
    unsigned int mask = 0;
        printf("Enter '0' for NIOS Reset.\n");
        printf("Enter '1' for I2C.\n");
        printf("Enter '2' for USB consoel.\n");
        printf("Enter '3' for GE 88E1512 PHY.\n");
        printf("Enter '4' for FPGA.\n");
        printf("Enter '5' for USB Port1 disable.\n");
        printf("Enter '6' for USB Port0 disable.\n");
        printf("Enter '7' for Main FPGA reset.\n");
        printf("Enter '8' for QUACK.\n");
        printf("Enter '9' for USB to Compact flash controller.\n");
        c = getdec_answer("Enter", 4, 0, 9);
        switch (c) {
        case  0 :
            mask = FPGA_IN_NIOS_RST;
            break;
        case  1 :
            c = getdec_answer("Enter I2c controller number", 0, 0, 17);
            gfy_i2c_reset((goofy_i2c_t *)(dash_fpga + FPGA_I2C_BASE +
                          ( c * FPGA_I2C_OFFSET)));
            return (PASSED);

        case  2 :
            mask = FPGA_RST_USB_CONS;
            break;
        case  3 :
            mask = FPGA_RST_GE;
            break;
        case  4 :
            mask = FPGA_RST_PCIE;
            break;
        case  5 :
            mask = FPGA_RST_USB1_DIS;
            break;
        case  6 :
            mask = FPGA_RST_USB0_DIS;
            break;
        case  7 :
            mask = FPGA_RST;
            break;
        case  8 :
            mask = FPGA_RST_ACT2;
            break;
        case  9 :
            mask = FPGA_RST_FLASH;
            break;
        default:
            break;
            
        }

        if (c < 3) {
            if (val) {
                reset_platform_in_dev(mask, 1);
            } else {
                unreset_platform_in_dev(mask);
            }
        } else {
            if (val) {
                printf("writing to cpld to reset maks is %#x\n", mask);
                reset_plat_dev( mask);
            } else {
                printf("writing to cpld to un reset maks is %#x\n", mask);
                unreset_plat_dev(mask);
            }
        }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function :get_platform_fpga_fw
 * Description: retrun pointer to fpga fw
 * INPUT:  NONE
 * OUTPUT: fpga firmware
 * -------------------------------------------------------------------
*/
unsigned char
*get_platform_fpga_fw (void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    if (!dash_fpga_fw_array) {
        assert(!"o2x86_lnx: missing fpga file name in its command line argument");
    }
    return (unsigned char *)(((unsigned long)dash_fpga_fw_array));
    //    return (unsigned char *)(((unsigned long)dash_fpga_fw));


}

/*-------------------------------------------------------------------
 *
 * Function :get_platform_fpga_fw
 * Description: retrun pointer to fpga fw
 * INPUT:  NONE
 * OUTPUT: fpga firmware
 * -------------------------------------------------------------------
*/
unsigned int
get_platform_fpga_size (void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    return dash_fpga_fw_size;

}

/*-------------------------------------------------------------------
 *
 * Function : platform_enable_spi_intr 
 * Description: enale spi intr
 * INPUT:  NONE
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
platform_enable_spi_intr (void)
{
    enable_platform_cp_intr_misc(FPGA_MISC_FPGA_SPI); /* misc control */
    enable_top_cp_intr(FPGA_MISC_INTR); /* intr control */
}

/*-------------------------------------------------------------------
 *
 * Function : aux_multiplex
 * Description: set aux multiplex mode
 * INPUT:  mode: mode of aux
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
aux_multiplex (int mode)
{
    console_t *c = (console_t *)get_platform_uart_mux_addr();
    //    c->aux = MUX_SEL_AUX2FPGA;
    if (c->aux & 0x10) {
        printf("****Jumper installed. Mux select disabled.******\n");
        return;
    }
    
    c->aux = mode;
    switch (mode) {
    case 0:
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            printf("mode %d; AUX to FPGA UART6; Rangeley UART0 to SMBUS/PECI\n", mode);
            enable_platform_uart_intr(1<<6);
        }
        else {
            printf("mode %d; Aux1->cavecreek1; Cavium->fpga uart 7\n", mode);
            enable_platform_uart_intr(1<<7);
        }
        break;
    case 1:
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            printf("mode %d; AUX to NIOS UART8; Rangeley UART0 to SMBUS/PECI\n", mode);
            enable_platform_uart_intr(1<<8);
        }
        else {
            printf("mode %d; Aux1->fpga uart 8; Cavium->fpga uart 7\n", mode);
            enable_platform_uart_intr((1<<7) | (1<<8) );
        }
        break;
    case 2:
        printf("mode %d; AUX to Rangeley UART0\n", mode);
        // disable_platform_uart_intr(0x1FF);
                    
        break;
    default:
        assert(!"invalid mode: aux_multiplex");
        break;

    }
    print_offset_val("aux setting", (unsigned long)dash_fpga,
                      (ulong)&c->aux, __LINE__, __FILE__);

    enable_platform_uart_intr(0xFFFF);
    
    printf(" ");
}
/*-------------------------------------------------------------------
 *
 * Function : dash_uart_tx
 * Description: write a string to a given uart port
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *  
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
dash_uart_tx (int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk)
{
    unsigned int idx;
    uart_t *uart;
    unsigned int quot;
    char dll, dlm; // division latch least significant and most significant

    uart = (uart_t *)get_platform_uart_addr(port);

    quot = 50000000 / baud;
    dll = quot & 0xFF;
    dlm = (quot & 0xFF00) >> 8;

    uart->fcr = 0xC6;   /* tx rx reset */

    /* setup baud rate */
    uart->lcr = 0x83;   /* 0xc */
    uart->dll = dll;    
    uart->dlm = dlm;

    uart->lcr = 3;
    uart->fcr = 0x1; /*enable FIFO and 1 byte trigger level */

    if (!tx_sz)
        return(PASSED);
        
    if (is_int_lpbk) {
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            /* In Utah the flow control bits are enabled so in a 
             * loopback configuration you need to enable DTR and RTS 
             */
            uart->mcr = 0x13;   /* looopback mode, enable DTR and RTS */
        } else {
            uart->mcr = 0x10;   /* turn on looopback mode */
        }
    } else {
        uart->mcr &= ~0x10;     /* turn off looopback mode */
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            uart->mcr = 0x3;    /* enable DTR and RTS */
        }

    }
    for (idx = 0; idx < tx_sz; idx++) {
        uart->dll = (tx_str[idx] & 0xFF);
        usleep(1000);
    }

    return(PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : uart_rx
 * Description: try to retreive data at the uart port
 * INPUT:  port         - uart port
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
dash_uart_rx (int port, int *rx_sz, char* rx_str)
{
    uart_t *uart;
    int cnt = 0;
    char* c;

    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    cnt = 0;
    c = rx_str;
    while (uart->lsr & 1) {
        c[cnt] = uart->dll;
        cnt++;
        if (*rx_sz > 0) {
            if (cnt >= *rx_sz)
                return(PASSED);
        }
        usleep(2000); /*delay is important: works for baud 9600 */
    }
    *rx_sz = cnt;
    return(PASSED);
}

void
dash_uart_reset (int port)
{
    uart_t *uart;
    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    uart->fcr = 0xC6;   /* tx rx reset */
    uart->mcr &= ~0x10; /* turn off loopback mode */
    return;
}
/*-------------------------------------------------------------------
 *
 * Function : uart_lpbk_txrx
 * Description: write a string to a given uart port and try to retreive data
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *  
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
uart_lpbk_txrx (int port, char* test_str, int test_sz, char* rx_str,
                int *rx_sz, int baud, int is_int_lpbk)
{

    dash_uart_reset(port);
    
    dash_uart_tx(port, baud, test_str, test_sz, is_int_lpbk);
    dash_uart_rx(port, rx_sz, rx_str);
    
    dash_uart_reset(port);

    return(PASSED);
}

/**********************************************************************
 *
 * Function: flush_uart_fifo
 *
 * Description: Flush out Tx and Rx Fifo of UART.
 *              If data is in the internal shift register, this function
 *              will NOT stop transmitting, nor wait for the receive data
 *              to be ready.
 *
 *              16552/16550 has 16 bytes FIFO. It can also operate at 16450
 *              which does not have FIFO. We will have to find out if the
 *              FIFO is enabled by reading FCR.
 *              LSR[TEMT] indicates Xmit empty if it is set. Both the
 *              Tx Holding Register and the internal tx shift register
 *              are empty.
 *              LSR[DR] indicates that Rx data is received if it is set.
 *              Reading RBR will clear the bit. Rx FIFOs and Tx FIFOs
 *              can be cleared by reset. If in 16450 mode (non-FIFO),
 *              the Rx shifting register state is unknown, and cannot
 *              be flushed out immediately.
 *
 * Input:  lpbk: loopback mode
 *
 * Output: None
 *
 **********************************************************************
 */
/* copy from src/console16552.c and modified the iir(FCR) and lsr */
void
uart_lpbk (int lpbk)
{
    unsigned int tx;
    uart_t *uart;
    char buf[100];
    int port;
    int cnt = 0;

    port = getdec_answer("enter uart controller number", 7, 0, 8);
    printf("controller number %d\n", port);
    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    print_offset_val("uart cntrl:", (unsigned long)dash_fpga,
                         (unsigned long)uart,__LINE__, 0);

    /* setup baud rate */
    uart->lcr = 0x83;  /* 0xc */
    uart->dll = 0x58;
    uart->dlm = 0x14;
    /*
    uart->dll = 0xB2;
    uart->dlm = 0x1;
    */
    uart->lcr = 3;

    uart->fcr = 0x1; /*enable FIFO and 1 byte trigger level */
    if (lpbk) {
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            /* In Utah the flow control bits are enabled so in a 
             * loopback configuration you need to enable DTR and RTS 
             */
            uart->mcr = 0x13;   /* looopback mode, enable DTR and RTS */
        } else {
            uart->mcr = 0x10;   /* turn on looopback mode */
        }
    } else {
        uart->mcr &= ~0x10;     /* turn off looopback mode */
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach() || is_vg400()) {
            uart->mcr = 0x3;    /* enable DTR and RTS */
        }
    }
    //    uart->ier = 0xF /* enable all interrupt */

    printf("enter string \n");
    while (1) {
        fgets(buf, sizeof(buf), stdin);

        if (*buf == 'q' )
            break;
        //        printf("string entered: \"%s\" \n", buf);
        for (tx = 0; tx < strlen(buf); tx++) {
            uart->dll = (buf[tx] & 0xFF);
            usleep(1000);
        }

        cnt = 0;
        while (uart->lsr & 1) {
            printf("..%c..", uart->dll);
            if (cnt++ > 50) {
                break;
            }
            usleep(2000);

        }

        printf("\n");
        
    }
    printf(" ");
    uart->fcr = 0xC6;   /* tx rx reset */
    uart->mcr &= ~0x10; /* turn off loopback mode */
}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_uart_console_intr
 * Description: enable platform uart console intr
 * INPUT:  dev, uart number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_uart_console_intr (int dev)
{
    unsigned int plane = CP;
    console_t *usb_console = (console_t *)get_platform_uart_mux_addr();
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)get_platform_intr_ctrl_addr(plane);

    intr_sts_cntl->misc_intr |= FPGA_MISC_UART_CONSOLE;
    usb_console->intr_en |= FPGA_USB_CONSOLE_CABLE_INTR_EN |
        FPGA_USB_CONSOLE_INTR_EN;
    
    /*
    print_offset_val("enable_platform_mcu_intr:", (unsigned long)dash_fpga,
                     (unsigned long)&intr_sts_cntl->misc_intr,
                     __LINE__, __FILE__);
    */
    
    enable_top_cp_intr(FPGA_MISC_INTR);

}

/*-------------------------------------------------------------------
 *
 * Function : enable_platform_uart_console_override_intr
 * Description: enable platform uart console override intr
 * INPUT:  dev, uart number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
enable_platform_uart_console_override_intr(int dev)
{
    unsigned int plane = CP;
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)get_platform_intr_ctrl_addr(plane);

    assert(dash_fpga);

    intr_sts_cntl->misc_ovr |= FPGA_MISC_UART_CONSOLE;
    enable_platform_uart_console_intr(dev);
}

/*-------------------------------------------------------------------
 *
 * Function : clear_platform_uart_console_override_intr
 * Description: clear platform uart console override intr
 * INPUT:  dev, uart number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clear_platform_uart_console_override_intr (int dev)
{
    console_t *usb_console = (console_t *)get_platform_uart_mux_addr();

    assert(dash_fpga);

    usb_console->intr |= (FPGA_USB_CONSOLE_CABLE_INTR_EN |
        FPGA_USB_CONSOLE_INTR_EN);

}

/*-------------------------------------------------------------------
 *
 * Function : disable_platform_uart_console_override_intr
 * Description: disable platform uart console override intr
 * INPUT:  dev, uart number
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
disable_platform_uart_console_intr (int dev)
{
    unsigned int plane = CP;
    console_t *usb_console = (console_t *)get_platform_uart_mux_addr();
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)get_platform_intr_ctrl_addr(plane);

    /* disable interrupt */
    intr_sts_cntl->misc_intr &= ~FPGA_MISC_UART_CONSOLE;
    intr_sts_cntl->misc_ovr &= ~FPGA_MISC_UART_CONSOLE;
    usb_console->intr_en &= ~(FPGA_USB_CONSOLE_CABLE_INTR_EN |
        FPGA_USB_CONSOLE_INTR_EN);

    /* clear interrupt */
    clear_platform_uart_console_override_intr(dev);

    /*
    print_offset_val("enable_platform_mcu_intr:", (unsigned long)dash_fpga,
                     (unsigned long)&intr_sts_cntl->misc_intr,
                     __LINE__, __FILE__);
    */
    
    disable_top_cp_intr(FPGA_MISC_INTR);
}

/*-------------------------------------------------------------------
 *
 * Function : cpld_reset
 * Description: hard reset system
 * INPUT:  NONE
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
cpld_reset (void)
{
    assert(dash_cpld);
    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    cpld->magic_cookie = 0x90000001 | (0x4CB2F << 8);
    
    return;
}

/*-------------------------------------------------------------------
 *
 * Function : display_uart_regs
 * Description: display uart regs
 * INPUT:  mode - MENU_MODE / CLI_MODE /CTERR_MODE
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
display_uart_regs(int mode)
{
    int i;
    unsigned int plane = get_platform_plane();
    unsigned long addr = get_platform_intr_ctrl_addr(plane);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    uart_t *uart;

    if(mode == DISPLAY_CTERR){
   
        cterr_db_print("%s @%#x = %#x;  \n", "", 
                    &intr_sts_cntl->uart_en, 
                    intr_sts_cntl->uart_en);        
       
        cterr_db_print("%s @%#x = %#x;  \n", "", 
                    &intr_sts_cntl->top_en, 
                    intr_sts_cntl->top_en);    
        cterr_db_print("\n");

        for (i =  0; i < MAX_UART; i++) {

            uart = (uart_t *)get_platform_uart_addr(i);
            
            cterr_db_print("UART%d\n", i);           
            cterr_db_print("%s @%#x = %#x;  \n","dll",
                            &uart->dll,
                           uart->dll);
            cterr_db_print("%s @%#x = %#x;  \n","dlm",
                            &uart->dlm,
                           uart->dlm);
            cterr_db_print("%s @%#x = %#x;  \n","fcr",
                            &uart->fcr,
                           uart->fcr);
            cterr_db_print("\n");
            cterr_db_print("%s @%#x = %#x;  \n","lcr",
                            &uart->lcr,
                           uart->lcr);
            cterr_db_print("%s @%#x = %#x;  \n","mcr",
                            &uart->mcr,
                           uart->mcr);
            cterr_db_print("%s @%#x = %#x;  \n","lsr",
                            &uart->lsr,
                           uart->lsr);     
            cterr_db_print("\n");
            cterr_db_print("%s @%#x = %#x;  \n","msr",
                            &uart->msr,
                           uart->msr);            
            cterr_db_print("%s @%#x = %#x;  \n","scr",
                            &uart->scr,
                           uart->scr);
            cterr_db_print("\n\n");
        }

    } else {

        print_offset_val("", (unsigned long)dash_fpga, 
            (unsigned long)&intr_sts_cntl->uart_en, __LINE__,  NULL);
        
        print_offset_val("", (unsigned long)dash_fpga, 
            (unsigned long)&intr_sts_cntl->top_en, __LINE__,  NULL);
        printf("\n");
        
        for (i =  0; i < MAX_UART; i++) {
        
            uart = (uart_t *)get_platform_uart_addr(i); /* uart 8 */
            printf("UART%d\n", i);
            print_offset_val("dll",dash_fpga,(unsigned  long)&uart->dll, 0, 0);
        
            print_offset_val("dlm",dash_fpga,(unsigned  long)&uart->dlm, 0, 0);
        
            print_offset_val("fcr",dash_fpga,(unsigned  long)&uart->fcr, 0, 0);
            printf("\n");
            print_offset_val("lcr",dash_fpga,(unsigned  long)&uart->lcr, 0, 0);
        
            print_offset_val("mcr",dash_fpga,(unsigned  long)&uart->mcr, 0, 0);
            print_offset_val("lsr",dash_fpga,(unsigned  long)&uart->lsr, 0, 0);
            printf("\n");
            print_offset_val("msr",dash_fpga,(unsigned  long)&uart->msr, 0, 0);
            print_offset_val("scr",dash_fpga,(unsigned  long)&uart->scr, 0, 0);
            printf("\n\n");
        }
    }
}

 /******************************************************************************
 * Function: display_uart_regs_cterr_wrapper
 * 
 * Description: This function is a wrapper for passing
 *                    DISPLAY_CTERR to display_uart_regs()
 *
 * Input:   None
 *
 * Output:  None
 *****************************************************************************/
void 
display_uart_regs_cterr_wrapper (void)
{
    display_uart_regs(DISPLAY_CTERR);
}
/*-------------------------------------------------------------------
 *
 * Function : display_multiboot
 * Description: display fpga multi boot registers
 * INPUT:  dummy , not used
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
int
display_multiboot (int dummy)
{
    hdr_t *hdr;
    hdr = (hdr_t *)get_platform_multiboot_base();

    /*    
    for (i=0;i<0x34;i+=4) {
        print_offset_val("", dash_fpga, dash_fpga+FPGA_HEADER_OFFSET+i,
                         0, 0);
    }
    */
    cterr_db_print("@%#x=%#x\n",  0x0, hdr->reconf_ctrl);        /* 0x0  */
    cterr_db_print("@%#x=%#x\n",  0x4, hdr->reconf_sts);         /* 0x4  */
    cterr_db_print("@%#x=%#x\n",  0x8, hdr->upgrade_rev);        /* 0x8  */
    cterr_db_print("@%#x=%#x\n",  0xA, hdr->upgrade_data);       /* 0xc  */
    cterr_db_print("@%#x=%#x\n",  0x10, hdr->upgrade_flag);      /* 0x10 */
    cterr_db_print("@%#x=%#x\n",  0x14, hdr->upgrade_magic);     /* 0x14 */
    cterr_db_print("@%#x=%#x\n",  0x18, hdr->state_hist);        /* 0x18 */
    cterr_db_print("@%#x=%#x\n",  0x1C, hdr->result_hist);       /* 0x1c */
    cterr_db_print("@%#x=%#x\n",  0x20, hdr->code_sign_boot_sts);/* 0x20 */
    cterr_db_print("@%#x=%#x\n",  0x24, hdr->secure_boot_sts);   /* 0x24 */
    cterr_db_print("@%#x=%#x\n",  0x28, hdr->secure_boot_sys);   /* 0x28 */
    cterr_db_print("@%#x=%#x\n",  0x2C, hdr->secure_boot_core);  /* 0x2C */
    cterr_db_print("@%#x=%#x\n",  0x30, hdr->secure_boot_sig);   /* 0x30 */
    cterr_db_print("@%#x=%#x\n",  0x34, hdr->secure_boot_sig_sz);/* 0x34 */

    return PASSED;
}


/* the functions on below are for Utah, Sword, Dagger platforms */
/*********************************************************************
 *
 * Function:    get_fan_status
 *
 * Description: read fan status register to see if fan is rotating.
 *
 * Inputs:      command - including fan number, aggregate rotation alert
 *                        and NEBS fan tray/filter installed
 *
 * Output:      TRUE/FALSE
 *
 *********************************************************************
 */
int get_fan_status (void) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint32_t status = env_fan->status;

    assert(dash_fpga);

    return (status);
   
}

/*********************************************************************
 *
 * Function:    enable_fan_ctrl
 *
 * Description: enable fan rotating
 *
 * Inputs:      fan_no - fan number 
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void enable_fan_ctrl (uint fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);

    env_fan->ctrl |=  fan_no;


    return; 
}

/*********************************************************************
 *
 * Function:    disable_fan_ctrl
 *
 * Description: disable fan rotating.
 *
 * Inputs:      fan_no - fan number
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void disable_fan_ctrl (uint fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);

    env_fan->ctrl &=  ~fan_no;

    return; 
}

/*********************************************************************
 *
 * Function:    fan_pwm_slope_read
 *
 * Description: read fan PWM slope register
 *
 * Inputs:      NONE
 *
 * Output:      NONE
 *
 *********************************************************************
 */
uint fan_pwm_slope_read (void) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint pwm_slope = env_fan->pwm_slope;

    assert(dash_fpga);

    return (pwm_slope);
}

/*********************************************************************
 *
 * Function:    fan_pwm_slope_write
 *
 * Description: write fan PWM slope register
 *
 * Inputs:      NONE
 *
 * Output:      pwm_slope - PWM slope for writting.
 *
 *********************************************************************
 */
void fan_pwm_slope_write (int pwm_slope) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);
    env_fan->pwm_slope = pwm_slope;

    return;
}


/*********************************************************************
 *
 * Function:    tachometer_rps_read
 *
 * Description: return tachometer RPS for specific fan number.
 *
 * Inputs:      fan_num - fan number
 *
 * Output:      NONE
 *
 *********************************************************************
 */
uint tachometer_rps_read (int fan_num) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint tach_rps = 0xFFFF;

    assert(dash_fpga);

    switch (fan_num) {
    case 1: 
        tach_rps = env_fan->tach_rps1;
    break;
    case 2: 
        tach_rps = env_fan->tach_rps2;
    break;
    case 3: 
        tach_rps = env_fan->tach_rps3;
    break;
    case 4: 
        tach_rps = env_fan->tach_rps4;
    break;
    default:
        printf("Unknown fan number %d\n", fan_num);
    break;
    }
    
    return (tach_rps);
} 

/*********************************************************************
 *
 * Function:    fan_speed_rd
 *
 * Description: read fan speed 
 *
 * Inputs:      fan_no - fan number
 *
 * Output:      curr_spd - current speed 
 *
 *********************************************************************
 */
uint fan_speed_rd (int fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint curr_spd;

    assert(dash_fpga);

    switch (fan_no) {        
    case FAN_NO_1:
        curr_spd = env_fan->speed1;
    break;
    case FAN_NO_2:
        curr_spd = env_fan->speed2;
    break;
    case FAN_NO_3:
        curr_spd = env_fan->speed3;
    break;
    case FAN_NO_4:
        curr_spd = env_fan->speed4;
    break;
    default:
        printf("Unknown fan number %d \n", fan_no);
        return 0x7D1; /* larger than max speed, for detect error */
    break;
    }

    return (curr_spd);
}


/*********************************************************************
 *
 * Function:    fan_speed_wr
 *
 * Description: write fan speed
 *
 * Inputs:      fan_no - fan number, fan_spd - fan speed 
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void fan_speed_wr (int fan_no, uint curr_spd) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
        
    switch (fan_no) {
    case FAN_NO_1:
        env_fan->speed1 = curr_spd;
    break;
    case FAN_NO_2:;
        env_fan->speed2 = curr_spd;
    break;
    case FAN_NO_3:
        env_fan->speed3 = curr_spd;
    break;
    case FAN_NO_4:
        env_fan->speed4 = curr_spd;
    break;
    default:
        return;
    break;
    }
        
    return;
}

/*********************************************************************
 *
 * Function:    smartfan_is_busy
 *
 * Description: 
 *
 * Inputs:      
 *
 * Output:      NONE
 *
 *********************************************************************
 */
boolean smartfan_is_busy (void) {
    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    if ((env_fan->env_fan_smartfan_status & FAN_SMARTFAN_STAT_BUSY)
         == FAN_SMARTFAN_STAT_BUSY) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*********************************************************************
 *
 * Function:    smartfan_fifo_empty
 *
 * Description: 
 *
 * Inputs:      
 *
 * Output:      NONE
 *
 *********************************************************************
 */
boolean smartfan_fifo_empty (void) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    if ((env_fan->env_fan_smartfan_status & FAN_SMARTFAN_STAT_FIFO_EMPTY)
         == FAN_SMARTFAN_STAT_FIFO_EMPTY) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*********************************************************************
 *
 * Function:    smartfan_fifo_rd
 *
 * Description: 
 *
 * Inputs:      
 *
 * Output:      NONE
 *
 *********************************************************************
 */
uchar smartfan_fifo_rd (void) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    return ((unsigned char)(env_fan->env_fan_smartfan_fifo));
}

/*********************************************************************
 *
 * Function:    smartfan_start
 *
 * Description: 
 *
 * Inputs:      
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void smartfan_start (uchar fan_num) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    uint data = ((fan_num << FAN_SMARTFAN_CTRL_FAN_OFFSET)
                 | FAN_SMARTFAN_CTRL_START);
    env_fan->env_fan_smartfan_control = data;
}

/*******************************************************************************
 *
 * Function    : dash_fpga_reg_write
 * Description : Function performs DASH FPGA register write.
 * Inputs      : base_addr - DASH FPGA base address
 *               reg_offset - register offset
 *               wrval    - data for write
 * Outputs     : 
 *
 *******************************************************************************
 */
int dash_fpga_reg_write (uint reg_offset, uint wrval)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga);
    *(unsigned int *)(addr + reg_offset) = wrval;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : dash_fpga_reg_read
 * Description : Function to read DASH FPGA register.
 * Inputs      : base_addr - DASH FPGA base address
 *               reg_offset - register offset
 *               *rdval       - buffer to put read back register value
 * Outputs     : 
 *
 *******************************************************************************
 */
int dash_fpga_reg_read (uint reg_offset, uint *rdval)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga);
    *rdval = *(unsigned int *)(addr + reg_offset);
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: get_cpld_sys_status_led_ctrl_reg
 * return the register offset for system status LED.
 *
 * Input: none
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
unsigned int
get_cpld_sys_status_led_ctrl_reg (void)
{
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;

    return (cpld->led);
}

/*-------------------------------------------------------------------
 *
 * Function: set_cpld_sys_status_led_ctrl_reg
 * set the register offset for system status LED.
 *
 * Input: val - register value
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void
set_cpld_sys_status_led_ctrl_reg (unsigned int val)
{
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;

    cpld->led = val; 

    return;
}

/************************************************************
 * Function : CPU Error Display
 *
 * Display Error types from CPU
 *
 * Input : NONE
 *
 * Output : NONE
 ************************************************************
 */
void cpu_err_show (void) {
    unsigned int tmp32, cpld_cpu_err;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    tmp32 = cpld->cpu_err;
    cpld_cpu_err = byteswap32(tmp32);

    printf("CPU ERROR Register : %#lx=%#x \n",
             (unsigned long)&cpld->cpu_err - dash_cpld, cpld_cpu_err);

    if ((cpld_cpu_err & CPLD_CPU_INT_ERR) == 1) {
        printf("Internal Error Latched");
    } else if ((cpld_cpu_err & CPLD_CPU_MAC_CHE_ERR) == 1) {
        printf("Machine Check Error Latched");
    } else if ((cpld_cpu_err & CPLD_CPU_ERR_2) == 1) {
        printf("Error 2 Signal Latched");
    } else if ((cpld_cpu_err & CPLD_CPU_ERR_1) == 1) {
        printf("Error 1 Signal Latched");
    } else if ((cpld_cpu_err & CPLD_CPU_ERR_0) == 1) {
        printf ("Error 0 Signal Latched");
    }
}

/************************************************************
 * Function : CPU Reset Reason Display
 *
 * Display Reset Reasons from CPU
 *
 * Input : NONE
 *
 * Output : NONE
 ************************************************************
 */
void cpu_reset_reason_show (void) {
    unsigned int tmp32, cpld_cpu_reset;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    tmp32 = cpld->rst_reason;
    cpld_cpu_reset = byteswap32(tmp32);

    printf("CPU RESET REASON Register : %#lx=%#x \n",
            (unsigned long)&cpld->rst_reason - dash_cpld, cpld_cpu_reset);

    if ((cpld_cpu_reset & CPLD_CPU_PLTRST ) == 1) {
         printf ("The last reset was caused by the Intel Rangeley asserting the PLTRST_L signal");
    } else if ((cpld_cpu_reset & CPLD_CPU_INTEL_RST) == 1) {
         printf ("The last reset event was caused by the Intel CPU error signal");
    } else if ((cpld_cpu_reset & CPLD_CPU_THERMTRIP) == 1) {
        printf ("The last power cycle was caused by the Intel THERMTRIP signal ");
    }
}

/************************************************************
 * Function : IRQ0 Status Regsiter Display 
 *
 * Display IRQ0 Register Status
 *
 * Input : NONE
 *
 * Output : NONE
 *
 ************************************************************
 */
void irq0_status_show (void) {
    unsigned int tmp32, cpld_irq0_sts;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    tmp32 = cpld->intr_irq0_sts;
    cpld_irq0_sts = byteswap32(tmp32);

    printf("IRQ0 STATUS Register : %#lx=%#x \n",
            (unsigned long)&cpld->intr_irq0_sts - dash_cpld, cpld_irq0_sts);

    if ((cpld_irq0_sts & CPLD_CPU_PROCHOT_RST) == 1) {
        printf ("The CPU PROCHOT signal has asserted since the last read of this register");
    }
}

/************************************************************
 * Function : Get Miscellaneous Status 
 *
 * Input : NONE
 *
 * Output : address
 *
 ************************************************************
 */
int get_platform_misc_sts(void) {
    unsigned long addr ;
    addr = get_platform_intr_ctrl_addr(NIOS);
    fpga_intr_t *intr_sts_cntl = (fpga_intr_t *)addr;
    assert(dash_fpga);

    /* If Register Status is not 0 , means Interrupt has occured */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Interrup Register Status  = %#x \n", intr_sts_cntl->misc_sts);
    }
    return (intr_sts_cntl->misc_sts);
}

/*------------------------------------------------------------------
$Log: dash_fpga.c,v $
Revision 1.75  2020/01/31 07:17:40  leschen
Support Curie new cookie.

Revision 1.74  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.73  2019/12/31 07:48:06  alpeng
revert cookie to old one for RDT

Revision 1.72  2019/12/21 00:52:37  ptong
Curie PID change to C8300-1N1S-4T2X and C8300-1N1S-6G

Revision 1.71  2019/09/11 07:18:15  alpeng
CSCvr18160 - adjust NIOS mode setup on Utah

Revision 1.70  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.69  2019/06/26 08:49:41  alpeng
support side band signal test for neptune; remove local intr check for sfp, since fpga is not support anymore.

Revision 1.68  2018/08/30 06:59:55  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.67  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.66  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.65  2016/10/17 11:22:56  iachang
Supported Goldbeach Platform.

Revision 1.64  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.63.6.23  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.63.6.22  2018/01/02 01:51:35  leschen
VG450 changes back into using PID for the determination of board type.

Revision 1.63.6.21  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.63.6.20  2017/11/22 08:15:55  leschen
Support Barsoom VG450.

Revision 1.63.6.19  2017/10/13 19:44:48  ptong
Release V3.2.0 for P1C. Removed NGVM diag for VG450 and Neptune. Minor change in is_vg450()

Revision 1.63.6.18  2017/10/06 02:24:04  ptong
Fix get_platform_bd_rev to get the correct brd rev

Revision 1.63.6.17  2017/09/30 07:02:07  alpeng
support both p1c and p1b pcie switch port mapping

Revision 1.63.6.16  2017/09/28 08:16:25  leschen
Support VG450

Revision 1.63.6.15  2017/08/02 09:59:41  leschen
Update Neptune PID from ISR4462 to ISR4461.

Revision 1.63.6.14  2017/04/05 06:27:01  leschen
Sync with <ng_diag-tag-032917>

Revision 1.63.6.13  2017/03/13 08:27:30  leschen
Support latest Neptune/Triton/Proteus/Neso PID.

Revision 1.63.6.12  2017/03/07 02:07:36  alpeng
update ISR num to 4462 for neptune

Revision 1.63.6.11  2017/03/03 18:54:42  ptong
Change to the final Neptune board ID. Release V1.1.2 diag.

Revision 1.63.6.10  2017/01/17 23:16:45  ptong
Update FPGA board ID values assigned by SW

Revision 1.63.6.9  2017/01/04 08:32:38  alpeng
update sku num for neptune

Revision 1.63.6.8  2016/12/23 03:42:24  alpeng
update ext. device reset for nep; clean up msg for NTC

Revision 1.63.6.7  2016/12/22 06:09:21  alpeng
fix typo and clean up msg for NIOS

Revision 1.63.6.6  2016/11/18 07:06:53  leschen
Modify chk_plat_sku function to support Neptune.

Revision 1.63.6.5  2016/10/20 22:15:46  alpeng
update thule get i2c bus num for carrier card; update tc get i2c bus mechanism; dash_fpga for bypass plx conflict

Revision 1.63.6.4  2016/10/10 22:58:36  leschen
Support Neptune Pericom switch info.

Revision 1.63.6.3  2016/10/10 17:02:41  alpeng
support NIM module for neptune

Revision 1.63.6.2  2016/06/10 18:11:10  ptong
Add is_neso

Revision 1.63.6.1  2016/06/02 10:30:07  leschen
Support to check Neptune/Triton/Proteus board type.

Revision 1.66  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.65  2016/10/17 11:22:56  iachang
Supported Goldbeach Platform.

Revision 1.64  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.63  2016/03/04 19:19:15  ptong
Clean up obsolete ISR platfrom PID

Revision 1.62  2015/03/06 06:22:28  alpeng
store ngio_bus_number in case mutiple time execute diag

Revision 1.61  2015/03/05 07:18:35  alpeng
fix is_plx issue

Revision 1.60  2014/09/11 08:06:20  alpeng
dump cpu temperature during diag boot up

Revision 1.59  2014/06/03 19:03:12  mcharon
add debug message to poe_intr

Revision 1.58  2014/05/19 23:22:17  mcharon
remove printf

Revision 1.57  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.56  2014/03/05 02:23:09  hroni
USD machines does not have env mcu. Remove platform_mcu.c and platform_mcu.h and cleanup the related code

Revision 1.55  2014/02/22 05:06:07  mcharon
add uart test that bypassse tty driver

Revision 1.54  2014/02/18 09:11:11  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.53  2014/01/27 21:36:00  mcharon
break uart_lpbk_test into 3 separate functions: init/send/receive

Revision 1.52  2014/01/16 00:49:15  hroni
Fix NIOS_NORMAL_MODE and NIOS_DISABLE_MODE macro

Revision 1.51  2014/01/14 03:20:29  hroni
1. NIOS set messages are printed only when verbose flag is turned on. 2. change NIOS_ENABLE_MODE to NIOS_NORMAL_MODE

Revision 1.50  2014/01/14 02:44:11  hroni
support NIOS_DIAG_MODE. use NIOS_DIAG_MODE instead of NIOS_NORMAL_MODE

Revision 1.49  2014/01/08 07:55:29  hroni
add enable_nios() to enable and disable nios for USD series

Revision 1.48  2013/12/27 09:34:06  danchung
Add PLX pcie switch existence wrapper function

Revision 1.47  2013/12/18 11:39:40  danchung
Add Utah/Sword machines existence function

Revision 1.46  2013/12/18 09:33:59  hroni
plx does not support prbs internal loopback, hide the test item in pcie switch util

Revision 1.45  2013/12/16 02:10:07  hroni
uart_lpbk_test() support custom compare string

Revision 1.44  2013/12/12 07:47:04  alpeng
show fan detail while init diag

Revision 1.43  2013/11/18 07:27:43  alpeng
support get_pci_bus_num()

Revision 1.42  2013/11/18 07:19:25  hroni
uart loopback test support sword and dagger

Revision 1.41  2013/11/15 10:08:18  danchung
Add existence function of sm slot 1 for Utah and Sword

Revision 1.40  2013/11/01 13:22:16  danchung
Fix testcard failure on Utah.

Revision 1.39  2013/11/01 07:02:58  alpeng
support is_juno_plx()

Revision 1.38  2013/10/17 07:33:42  hroni
do FIFO Tx Rx reset after UART loopback test

Revision 1.37  2013/10/16 04:31:50  hroni
Overlord and Utah use the same Uart loopback test function that directly access Uart register instead of accessing Uart through tty driver.

Revision 1.36  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.35  2013/09/09 05:29:11  ptong
Add is_utah_p1a() check function

Revision 1.34  2013/09/05 01:58:27  alpeng
support mSATA test on Utah

Revision 1.33  2013/08/22 06:40:49  alpeng
support fan utility on Utah

Revision 1.32  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.31  2013/08/14 17:55:35  hroni
1. turn on option  enable/disable internal loopback to uart_lpbk_test()
2. support Utah AUX port selection for FPGA UART 6, Rangeley 0, and NIOS uart 8

Revision 1.30  2013/08/14 08:44:27  alpeng
support sfp interrupt

Revision 1.29  2013/08/07 09:40:21  alpeng
support SFP interrupt test and handler

Revision 1.28  2013/08/05 10:13:56  alpeng
support fan utility on Utah

Revision 1.27  2013/07/30 20:00:18  hroni
use primitive uart test method to test the uart loopback

Revision 1.26  2013/07/30 10:05:14  danchung
Modified for LED test on Utah.

Revision 1.25  2013/07/24 01:19:23  hroni
Enable DTR and RTS for Utah UART

Revision 1.24  2013/07/10 01:34:53  alpeng
moving get_plat_sku() to platform_cookie.c. since the sku number coming from cookie.

Revision 1.23  2013/07/09 09:49:09  alpeng
moving function is_platform() related to dash_fpga.c

Revision 1.22  2013/07/02 10:57:11  hroni
change max i2c controller number to 17

Revision 1.21  2013/03/14 18:18:46  mcharon
check reset bit after putting nios into reset before proceeding

Revision 1.20  2013/02/27 22:21:56  mcharon
add api to support switching between pass-thru & controller to support overdrive

Revision 1.19  2013/01/25 05:50:19  alpeng
supported poe psu interrupt

Revision 1.18  2013/01/07 22:53:16  mcharon
reset mask for flash should be FPGA_RST_FLASH

Revision 1.17  2012/11/29 02:14:27  palin2
To add HDD and Environmental LEDs control utility spoort.

Revision 1.16  2012/11/17 01:15:17  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.15  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.14  2012/09/27 22:04:03  mcharon
don't bundle fpga into image; support dynamic fpga download

Revision 1.13  2012/09/25 21:00:09  mcharon
support multiboot fpga programming

Revision 1.12  2012/09/20 00:13:00  mcharon
support oir

Revision 1.11  2012/09/19 09:20:44  alpeng
support OIR SM1 interrupt test

Revision 1.10  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.8  2012/08/11 00:13:29  mcharon
support reset/unreset mb act2 chip

Revision 1.6  2012/06/05 11:44:30  palin2
Clean up compiler warnings.

Revision 1.5  2012/05/31 21:21:02  palin2
Clean up compile warnings.

Revision 1.4  2012/05/02 02:05:28  mcharon
add config header support

Revision 1.3  2012/04/25 07:50:07  alpeng
support clean alert via accessing FPGA directly

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module

$Endlog$
*/

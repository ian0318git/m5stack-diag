/* $Id: nim_test.c,v 1.3 2017/03/30 08:34:09 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/nim_test.c,v $
 *------------------------------------------------------------------
 * Filename:   nim_test.c
 *
 * Description: intel nim test entry
 *
 * Copyright (c) 2015-2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/* 
 * PCIe related tests for NGIO
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nim_test_defs.h"
#include "types.h"
#include "error.h"
#include "platform_fru.h"
#include "setjmps.h"
#include "common.h"
#include "dreamliner_poe.h"

/* dreamliner defines */
#define PHY_ADDR              0x00

/* extern */
extern char *banner_string;
extern int tc_pcie_test(int, int); 
extern int dl_pcie_test(int, int); 
extern int tc_pcie_utils(int); 
extern int dl_pcie_utils(int); 
extern char *optarg;
extern int get_pcie_info(unsigned short, unsigned short,
                    unsigned char *, unsigned char *,
                    unsigned char *);

/* extern for dreamliner */
extern int nim_dm_cpss_extserv_init(unsigned int, unsigned int);
extern void nim_dm_cpss_extserv_cleanup(void);
extern int sw_init(void); 
extern int phy_unload_driver(void);
extern int phy_start_driver(int);
extern int set_module_ready(void);
extern void phy_lpbk_ge_clr(int, int);
extern void phy_lpbk_ge_init(int, int);
extern int dl_read_i2c(uchar, uchar, uint, uchar *);
extern void poe_si_flag_write(char *);

/* static */
static void nim_init(void);
static void nim_cleanup(void);
static void dl_init(void);
static void dl_cleanup(void);
static void dl_init_poe(void);

/* global */
volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;
int nim_slot; 
int nim_types; 
int lpbk_type; 
int nim_port; 
int module_num; 
int netflashbooted = 1;  /* used by menu.c */
int is_plat_poe = FALSE; 

/* used by linux_error.c */
/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar sm_pid[] = "SM-PID";
uchar wic_pid[] = "WIC-PID";
uchar dc_pid[] = "DC-PID";
uchar mb_loc[] = "MB";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";
uchar psu0_loc[] = "MB/PSU0";
uchar psu1_loc[] = "MB/PSU1";
uchar rps_loc[] = "MB/RPS";
uchar pvdm0_loc[] = "MB/PVDM0";
uchar backplane_loc[] = "MB/Backplane";
uchar risercard_loc[] = "MB/RiserCard";
uchar sm0_loc[] = "MB/SM0";
uchar sm1_loc[] = "MB/SM1";
uchar wic0_loc[] = "MB/WIC0";
uchar wic1_loc[] = "MB/WIC1";
uchar wic2_loc[] = "MB/WIC2";
uchar sm0wic_loc[] = "SM0/WIC";
uchar sm1wic_loc[] = "SM1/WIC";
uchar sm0pvdm0_loc[] = "SM0/PVDM0";
uchar sm0pvdm1_loc[] = "SM0/PVDM1";
uchar sm0pvdm2_loc[] = "SM0/PVDM2";
uchar sm1pvdm0_loc[] = "SM1/PVDM0";
uchar sm1pvdm1_loc[] = "SM1/PVDM1";
uchar sm1pvdm2_loc[] = "SM1/PVDM2";
uchar sm0wic0dc_loc[] = "SM0/WIC0/DC";
uchar sm1wic0dc_loc[] = "SM1/WIC0/DC";
uchar sm0wic1dc_loc[] = "SM0/WIC1/DC";
uchar sm1wic1dc_loc[] = "SM1/WIC1/DC";
uchar sm0dc_loc[] = "SM0/DC";
uchar sm1dc_loc[] = "SM1/DC";

fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
    { psu_pid,       psu0_loc },
    { psu_pid,       psu1_loc },
    { rps_pid,       rps_loc },
    { pvdm_pid,      pvdm0_loc },
    { backplane_pid, backplane_loc },
    { risercard_pid, risercard_loc },
    { sm_pid,        sm0_loc },
    { sm_pid,        sm1_loc },
    { wic_pid,       wic0_loc },
    { wic_pid,       wic1_loc },
    { wic_pid,       wic2_loc },
    { wic_pid,       sm0wic_loc },
    { wic_pid,       sm1wic_loc },
    { pvdm_pid,      sm0pvdm0_loc },
    { pvdm_pid,      sm0pvdm1_loc },
    { pvdm_pid,      sm0pvdm2_loc },
    { pvdm_pid,      sm1pvdm0_loc },
    { pvdm_pid,      sm1pvdm1_loc },
    { pvdm_pid,      sm1pvdm2_loc },
    { dc_pid,        sm0wic0dc_loc },
    { dc_pid,        sm1wic0dc_loc },
    { dc_pid,        sm0wic1dc_loc },
    { dc_pid,        sm1wic1dc_loc },
    { dc_pid,        sm0dc_loc },
    { dc_pid,        sm1dc_loc },
};
/* end of used by linux_error.c */


/* function tables for test/util */
int (*pcie_test[])(int slot, int sel) = {
        tc_pcie_test,
        dl_pcie_test,
};

int (*pcie_utils[])(int slot) = {
        tc_pcie_utils,
        dl_pcie_utils, 
};


/*************************************************************************
 *
 * Function   : main
 * Description: entry functions
 * Inputs     : argc - argument count 
 *              argv - argument vector 
 * Outputs    : NONE
 *
 **************************************************************************
 */
int main (int argc, char *argv[]) 
{
    int opt_ch, rc;
    unsigned int utils = FALSE, opt = 0, lpbk = FALSE, clr = FALSE, clean = FALSE;
    
    module_num = 0;  /* init in case user forget */
    nim_slot = 1;  /* init in case user forget */
    nim_types = 0;   /* init as default */
    lpbk_type = 0; 
    nim_port = 0; 

    printf("%s", banner_string);

    if (argc == 1) { 
        goto USAGE; /* missing arguments */
    }

    while ((opt_ch = getopt(argc, (char **)argv, "m:s:o:ut:l:p:c:evx")) >= 0) {
        switch(opt_ch) {
            case 'm':
                module_num = atoi(optarg); 
            break;
            case 's':
                nim_slot = atoi(optarg);
            break;
            case 'o':
                opt = atoi(optarg);
            break;
            case 'u':
                utils = TRUE;
            break;
            case 't': /* use -t to distinguish nim types */
                nim_types = atoi(optarg);
            break;
            case 'l': /* use -l to distinguish loopback types */
                      /* dreamliner: 0 for ge0, 1 for ge1, 2 for external loopback */
                lpbk = TRUE; 
                lpbk_type = atoi(optarg);
            break;
            case 'p': /* use -p to support port arg */
                      /* e.g. dreamliner front panel ports */
                nim_port = atoi(optarg);
            break;
            case 'c': /* use -c to support clear lpbk  */
                clr = TRUE; 
                lpbk_type = atoi(optarg);
            break;
            case 'e': /* use -e to support poe or none poe  */
                is_plat_poe = TRUE; 
            break;
            case 'v': /* use -v to display version and exit */
                return (PASSED);
            break;
            case 'x': /* use -x to support clear up  */
                clean = TRUE; 
            break;
            case '?':
                goto USAGE;
            break;
            default: 
                goto USAGE;
            break;
         }
    }

#if 0 
    if ((TACHI_ENTRY) && (nim_slot != 1)) {
        printf("Tachi entry only support one NIM slot \n");
    } else if ((TACHI_HIGH) && ((nim_slot < 1) || (nim_slot > 3)) {
        printf("Tachi high only support NIM 1~3 slots \n");
    } else {
        printf("dummy...\n");
    }
#endif 

    printf("module num = 0x%x, slot= %x\n", module_num, nim_slot);

    nim_init(); 

    /* special arg to satisfy dreamliner loopback test 
     * setup loopback on intel side and send packet from bmc side 
     */
    if (lpbk == TRUE) {
        phy_lpbk_ge_init(lpbk_type, nim_port); 
        return (0); /* skip cleanup to keep loopback state */
    }

    if (clr == TRUE) {
        phy_lpbk_ge_clr(lpbk_type, nim_port);  /* lpbk_type, 0 or 1 */
        nim_cleanup();
        return (0); 
    }

    if (utils == TRUE) { 
        rc = pcie_utils[module_num](nim_slot);	
    } else {
        rc = pcie_test[module_num](nim_slot, opt);	
    }
    if (clean == TRUE) {
    nim_cleanup();
    }
    return (rc); 


USAGE: 
    printf("========NIM test========\n");
    printf("./nim_test -m [module num] -s [slot num] \n");
    printf("\n");
    printf("========general  options========\n");
    printf("-m: module num:  \n");
    printf("    0  -  testcard\n");
    printf("    1  -  dreamliner\n");
    printf("\n");
    printf("-s [slot num]: nim slot  \n");
    printf("    1  -  slot 1\n");
    printf("\n");
    printf("-o [option]: option num  \n");
    printf("    option number \n");
    printf("\n");
    printf("-t [type]: nim type  \n");
    printf("    nim SKU type, defined by user \n");
    printf("\n");
    printf("-l [lpbk type]: nim loopback type  \n");
    printf("    nim internal/external loopback, defined by user \n");
    printf("\n");
    printf("-p [nim ports]: nim ports  \n");
    printf("    nim front panel ports, defined by user \n");
    printf("\n");
    printf("-c: nim clean up  \n");
    printf("    nim clean up code, defined by user \n");
    printf("\n");
    printf("-u: for utility \n");
    printf("\n");
    return (0);
}

/*************************************************************************
 *
 * Function   : dl_init
 * Description: dreamliner init functions
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void dl_init (void) 
{
    /* insert anyway */
    system("insmod /opt/nim/nim_dm.ko &> /dev/null"); 

    /* init nim_dm klm */
    if (nim_dm_cpss_extserv_init(0,nim_slot) != 0) {
        dl_cleanup(); 
        exit(-1);
    }

    /* init ge switch */
    if (sw_init() != 0) {
        printf("\n sw_init failed. \n");
        dl_cleanup(); 
        exit(-1);
    }

    if (phy_start_driver(PHY_ADDR) != 0) {
        printf("\n phy_start_driver failed. \n");
        dl_cleanup(); 
        exit(-1);
    }

    if (set_module_ready() != 0) {
        printf("\n phy_start_driver failed. \n");
        dl_cleanup(); 
        exit(-1);
    }

    dl_init_poe(); 
    return; 
}

/*
 * Function: dl_init_poe
 *
 * Description : return nim slots
 *
 * Inputs: NONE
 *
 * Output: nim_slot - global value
 *
 */
void dl_init_poe (void)
{
    uchar device_id0 = 0;
    poe_si_flag_write("FALSE");

    /* Read Device ID of both chips */
    if (dl_read_i2c(POE_I2C_ADDR0, ILP_ID, 1, &device_id0) == FAILED) {
        return;
    }
    if ((device_id0 & 0xf0) == TI_MSR_ID) {
        poe_si_flag_write("FALSE");
        printf("PoE chip is TI, Device ID = %x\n", device_id0);
        fflush(0);
    } else if ((device_id0 & 0xf0) == SI_MSR_ID) {
        poe_si_flag_write("TRUE");
        printf("PoE chip is SI, Device ID = %x\n", device_id0);
        fflush(0);
    } else {
        printf("PoE chip invalid , Device ID = %x\n", device_id0);
    }
    return; 
}
    


/*************************************************************************
 *
 * Function   : dl_cleanup
 * Description: dreamliner clean up functions 
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void dl_cleanup (void)
{
    phy_unload_driver();
    nim_dm_cpss_extserv_cleanup();
    system("rmmod nim_dm &> /dev/null");

    return; 
}

/*************************************************************************
 *
 * Function   : nim_init
 * Description: nim init functions, based on global module_num
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void nim_init (void) 
{
    /* nim init */
    switch (module_num) {
    case 0 : /* testcard */
        /* N/A for testcard */
    break; 
    case 1 : /* dreamliner */
        dl_init(); 
    break; 
    default: 
        printf("No init seq for this module num %d \n", module_num);
    break; 
    }
    return; 
}

/*************************************************************************
 *
 * Function   : nim_cleanup
 * Description: nim clean up functions, based on global module_num
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void nim_cleanup (void) 
{
    /* nim init */
    switch (module_num) {
    case 0 : /* testcard */
        /*  No needed */
    break; 
    case 1 : /* dream liner */
        dl_cleanup(); 
    break; 
    default: 
        printf("No init seq for this module num %d \n", module_num);
    break; 
    }

    return; 
}

/*************************************************************************
 *
 * Function   : get_ngio_pcie_bus_num
 * Description: Function to get system PCIe bus number for NGIO slots
 * Inputs     : void
 * Outputs    : bus number
 * 
 * Note       : used by linux_pcie.c 
 *************************************************************************
 */
uint32_t get_ngio_pcie_bus_num (void)
{
    unsigned short vendor_id = TACHI_PCIE_SW_VID;
    unsigned short device_id = TACHI_PCIE_SW_DID;
    unsigned char bus, dev, func;
 
    get_pcie_info(vendor_id, device_id, &bus, &dev, &func);
    return(bus);
}

/*************************************************************************
 *
 * Function   : check_poe_psu_present
 * Description: Function to check platform is supporting POE 
 *              information comes from nc
 * Inputs     : void
 * Outputs    : bus number
 *
 * Note       : used by linux_pcie.c
 *************************************************************************
 */
unsigned int check_poe_psu_present (unsigned int psu_no,
                                       unsigned int option)
{
   return (is_plat_poe); 
}

/*------------------------------------------------------------------
$Log: nim_test.c,v $
Revision 1.3  2017/03/30 08:34:09  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/12/21 12:43:43  hondwang
Fix dreamliner loopback issue

Revision 1.2  2016/04/20 08:54:00  benchen2
add tachi fru portion

Revision 1.1.2.12  2016/03/01 02:40:36  alpeng
update version for supporting daily-build

Revision 1.1.2.11  2016/02/18 02:53:02  alpeng
update version and banner

Revision 1.1.2.10  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.9  2015/12/18 09:27:31  alpeng
fix dreamliner msg, supporting switch diag menu dynamically

Revision 1.1.2.8  2015/12/17 03:46:30  alpeng
support dreamliner nc and poe

Revision 1.1.2.7  2015/12/09 10:35:56  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.6  2015/11/03 09:43:51  alpeng
update dreamliner utility to support sw bridge

Revision 1.1.2.5  2015/10/05 10:21:38  alpeng
support single test, update loopback test

Revision 1.1.2.4  2015/09/30 06:02:19  alpeng
update dreamliner util, test and menu

Revision 1.1.2.3  2015/09/14 08:02:29  alpeng
update to support dreamliner

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/


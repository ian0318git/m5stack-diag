/* $Id: platform_stub.c,v 1.6 2020/01/09 01:02:38 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_stub.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "types.h"
#include "common.h"
#include "cli_cmd.h"
#include "platform_stub.h"
#include "nmc93c46.h"
#include "legacy_smart_cookie.h"
#include "dash_fpga.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int get_max_sm_slots(void);
int ngiosm_present(void *p);
int ngiosm_i2c_reset(void *);
int ngiosm_i2c_unreset(void *);
struct ngio_intf_t *slot_get_ngiovm(int);
int ngiovm_i2c_reset(void *);
int ngiovm_i2c_unreset(void *);
int smart_cookie_read_write_eeprom(sc_context *, cli_cookie_cmd *);

struct ngio_intf_t *slot_get_ngioisp(int);
int ngioisp_i2c_reset(void *);
int ngioisp_i2c_unreset(void *);
uint32_t get_ngio_pcie_bus_num(void);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/

int netflashbooted = 1;

/**********************************************************************
 *
 * Function:    get_max_sm_slots
 *
 * Description: Dummy function
 *
 * Inputs:  NONE
 * Output:  PASSED
 **********************************************************************
 */
int get_max_sm_slots(void)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: slot_get_ngiosm
 *
 * Description: Dummy function
 *
 * Input:  None
 * Output: dummy pointer
 *
 **********************************************************************
 */
struct ngio_intf_t *slot_get_ngiosm(int slot)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: slot_get_ngioisp
 *
 * Description: Dummy function
 *
 * Input:  None
 * Output: dummy pointer
 *
 **********************************************************************
 */
struct ngio_intf_t *slot_get_ngioisp(int slot)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: ngiosm_i2c_reset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiosm_i2c_reset(void *p)
{
    return (OK);
}

/**********************************************************************
 *
 * Function: ngioisp_i2c_reset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngioisp_i2c_reset(void *p)
{
    reset_isp_dev(ISP_FPGA_RST_ACT2);
    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_present
 * Description: check if SM is present
 * INPUT:  p -- pointer to struct ngio_intf_t
 * OUTPUT: TRUE if SM is present
 * -------------------------------------------------------------------
*/
int
ngiosm_present (void *p)
{
    return (OK);
}


/**********************************************************************
 *
 * Function: ngiosm_i2c_unreset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiosm_i2c_unreset(void *p)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: ngioisp_i2c_unreset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngioisp_i2c_unreset(void *p)
{
    unreset_isp_dev(ISP_FPGA_RST_ACT2);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: slot_get_ngiovm
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
struct ngio_intf_t *slot_get_ngiovm(int slot)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: ngiovm_i2c_reset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiovm_i2c_reset(void *p)
{
    return (OK);
}

/**********************************************************************
 *
 * Function: ngiovm_i2c_unreset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiovm_i2c_unreset(void *p)
{
    return (OK);
}

/*
 * Function: get_ngio_pcie_dev_bus_num
 * description: based on PCIe bus to return ngio device bus number.
 * input : ngio_if->slot
 *         ngio_if->mod_type
 * output: ngio_pcie_bus_no - NGIO PCIe bus number.
 * Note  : dev_no is defined on platforms HFS.
 */
int
get_ngio_pcie_dev_bus_num (uint mod_type, uint slot)
{
    /* FIX me */
    return (PASSED);
}

/*
 * Function: host_ngio_10gkr_capability
 * Find out the platform's capability to support 10G-KR on the
 * NGIO interface. Retune a bit mask value by using
 * NGIO_GE0_BITMASK, NGIO_GE1_BITMASK, etc.
 *
 * input : ngio_if->mod_type
 *         ngio_if->slot
 *
 * return: Bit mask to indicate which NGIO GE port is 10G-KR
 *         capable. For example:
 *         0x1 mean GE0 is supported
 *         0x3 mean GE1 and GE0 are supported
 */
uint
host_ngio_10gkr_capability (uint mod_type, uint slot)
{

    return(PASSED);
}

type_t
smartchip_authenticate_retest(uchar type, uchar slot)
{
    return 0; 
}


int
smartchip_authenticate(uchar type, uchar slot)
{
    return 0; 
}

int is_10g_gesw (void) 
{
    return 0; 
}

uint32_t get_ngio_pcie_bus_num (void)
{
    return 0; 
}

/**********************************************************************
 *
 * Function: smart_cookie_read_write_eeprom
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int smart_cookie_read_write_eeprom (sc_context * con, cli_cookie_cmd * cli_cmd)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 *
 **********************************************************************
 */

float get_mem_overhead_factor(void) {
    return 0.1;
}

/*-------------------------------------------------------------------
 *
 * Function : is_curie_1ru
 * Description: Return TRUE if platform is Curie 1RU
 *              This function returns FALSE by default. If platform
 *              is curie_1ru, declare this function in platform code
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_curie_1ru (void)
    __attribute__((weak, alias("__is_curie_1ru")));
int __is_curie_1ru (void)
{
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_curie_2ru
 * Description: Return TRUE if platform is Curie 2RU
 *              This function returns FALSE by default. If platform
 *              is curie_2ru, declare this function in platform code
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_curie_2ru (void)
    __attribute__((weak, alias("__is_curie_2ru")));
int __is_curie_2ru (void)
{
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_goldbeach
 * Description: Return TRUE if platform is Goldbeach
 *              This function returns FALSE by default. If platform
 *              is Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_goldbeach (void)
    __attribute__((weak, alias("__is_goldbeach")));
int __is_goldbeach (void)
{
    return (FALSE);
}
/*-------------------------------------------------------------------
 *
 * Function : utah_port_is_linkup
 * Description: Return TRUE if platform is USD/Goldbeach
 *              This function returns FALSE by default. If platform
 *              is USD/Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int utah_port_is_linkup (void)
    __attribute__((weak, alias("__utah_port_is_linkup")));
int __utah_port_is_linkup (void)
{
    return (FALSE);
}
/*-------------------------------------------------------------------
 *
 * Function : netstat_main
 * Description: Return TRUE if platform is USD/Goldbeach
 *              This function returns FALSE by default. If platform
 *              is USD/Goldbeach, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int netstat_main (void)
    __attribute__((weak, alias("__netstat_main")));
int __netstat_main (void)
{
    return (FALSE);
}  
/******** History ******** 
$Log: platform_stub.c,v $
Revision 1.6  2020/01/09 01:02:38  jiajliu
Merge Curie 2RU to main trunk

Revision 1.5  2019/08/06 06:56:17  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4.38.1  2019/07/25 00:43:02  alpeng
add weak function is_curie_1ru() on tachi

Revision 1.4  2017/08/10 10:12:49  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.3  2016/05/06 17:44:27  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.10  2016/04/18 07:26:06  alpeng
remove ifdef tachi on linux_pcie.c, this function need to be done on intel side or nc cmd

Revision 1.1.2.9  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.8  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.7  2015/08/21 06:46:29  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.6  2015/08/11 07:44:28  meho
Added f35 nim tests.

Revision 1.1.2.5  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.4  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.3  2015/07/24 06:59:58  alpeng
Add ngio.c to support NIM test

Revision 1.1.2.2  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.1  2015/06/11 02:01:11  tirawan
Add files for Tachi BMC project



$Endlog$
*/

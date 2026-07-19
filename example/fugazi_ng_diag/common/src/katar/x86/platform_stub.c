/* $Id: platform_stub.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_stub.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "platform_aikido.h"
#include "/auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido/tam_library.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/
int netflashbooted = 1;


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int smart_cookie_read_write_eeprom (sc_context *con, cli_cookie_cmd *cli_cmd)
{
    return (PASSED);
}

int smart_cookie_read_x(sc_context *con_p, ushort size)
{
    return (PASSED);
}

int smart_cookie_read(sc_context *con)
{
    return (PASSED);
}

type_t smartchip_authenticate_retest(uchar type, uchar slot)
{
    return (PASSED);
}

int smartchip_authenticate(uchar type, uchar slot)
{
    return (PASSED);
}

int quack_version(sc_context *con)
{
    return (PASSED);
}

int pem_show_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    return (PASSED); 
}

int poe_reg_test ()
{
    return PASSED;
}

int poe_intr_test()
{
    return PASSED;
}

int is_poe_present (char *buf)
{
	return FALSE;
}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 * **********************************************************************
 */

float get_mem_overhead_factor(void) {
    return 0.006;
}

/**********************************************************************
 *
 * Function:    slot_start_with
 *
 * Description: This function return the number of the first SM Slot.
 *
 * Inputs:      none.
 * Output:      SM_SLOT1 (1).
 * *********************************************************************
 */

int slot_start_with (void)
{
    return (PASSED);
}

/**********************************************************************
 *
 * Function:    get_max_sm_slots
 *
 * Description: return max number of SM slots
 *
 * Inputs:      NONE
 * Output:      max number of SM slots
 * *********************************************************************
 */

int get_max_sm_slots (void)
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
 * **********************************************************************
 */
struct ngio_intf_t *slot_get_ngiosm(int slot)
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
 * **********************************************************************
 */
int ngiosm_i2c_reset(void *p) 
{
    return (PASSED);
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
    return (PASSED);
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
    return (PASSED);
}

/**********************************************************************
 * 
 * Function: slot_get_ngiowic
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
struct ngio_intf_t *slot_get_ngiowic(int slot)
{
    return (PASSED);
}

/**********************************************************************
 * 
 * Function: ngiowic_i2c_reset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiowic_i2c_reset(void *p)
{
    return (PASSED);
}

/**********************************************************************
 * 
 * Function: ngiowic_i2c_unreset
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 **********************************************************************
 */
int ngiowic_i2c_unreset(void *p) 
{
    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.6  2019/04/30 06:06:59  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.5  2019/04/12 01:35:56  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.4  2019/03/08 07:19:19  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.3  2019/02/12 08:06:31  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/01/29 01:54:22  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.2  2018/12/12 02:03:39  peteteng
 * Add Aikido FW upgrade through LPC
 *
 * Revision 1.1.2.1  2018/10/22 08:02:30  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.2  2018/10/22 03:04:32  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.1  2018/07/02 08:21:10  peteteng
 * Update katar tam lib
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */



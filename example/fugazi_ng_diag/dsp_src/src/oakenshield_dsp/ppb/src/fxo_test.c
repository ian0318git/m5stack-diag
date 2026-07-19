/* $Id: fxo_test.c,v 1.5 2021/04/15 00:53:07 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/fxo_test.c,v $ 
 *------------------------------------------------------------------
 * fxo_test.c
 * Tests for FXO
 *
 * Oct 2016 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "tstcodec_si3050.h"
#include "si3xxx_utils.h"
#include "diag_fpga.h"
#include "fxs_test.h"


/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/

/*===================================================================*
 *                             Globals                               *
 *===================================================================*/


/**********************************************************************
 *  Function:  oak_fxo_ports
 *
 *  Description: Get number of ports of the Oakenshield FXO card.
 *
 *  Input: Pointer to the port configuration
 *
 *  Returns: PASSED/FAILED
 *
 * ********************************************************************
 */
int oak_fxo_ports (oak_port_cfg *cfg)
{
    uchar board_id = 0xFF;

    board_id = get_oak_id();

    switch(board_id) {
    case BOARD_16FXS_2FXO:
        cfg->start = 0;
        cfg->size = 2;
        break;
    case BOARD_24FXS_4FXO:
        cfg->start = 0;
        cfg->size = 4;
        break;
    case BOARD_8FXS_12FXO:
        cfg->start = 0;
        cfg->size = 12;
        break;
    case VG400_2FXS_2FXO:
        cfg->start = 4;
        cfg->size = 2;
        break;
    case VG400_4FXS_4FXO:
        cfg->start = 0;
        cfg->size = 4;
        break;
    case VG400_6FXS_6FXO:
        cfg->start = 0;
        cfg->size = 6;
        break;
    case PHOENIX_132FXS_6FXO:
    case PHOENIX_84FXS_6FXO:
        cfg->start = 0;
        cfg->size = 6;
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return (FAILED);
        break;
    }
    return (PASSED);
}


/**********************************************************************
 *  Function:  reset_si3050
 *
 *  Description: Hard reset the Oakenshield FXO codec, and init the codec.
 *
 *  Input: None
 *
 *  Returns: number of ports. 0 for unknown Oakenshield
 *
 * ********************************************************************
 */
void reset_si3050 (void)
{
    oak_port_cfg port_cfg;
    int port;

    /* FPGA to reset the Codec */
    si3050_reset(TRUE);
    msleep(100); 
    si3050_reset(FALSE);
    msleep(100);


    if (oak_fxo_ports(&port_cfg) == FAILED) {
        cterr('f', 0, "%s: Unknown Oakenshield FXO card", __FUNCTION__);
    }

    bsp_debug_printf("\n\r%s: start %d for %d ports\n", __FUNCTION__, port_cfg.start,
                     port_cfg.size);

    for (port = port_cfg.start; port < (port_cfg.size + port_cfg.start); port++) {

        if (si3050_codec_init(port) == FAILED) {
            cterr('f', 0, "%s: Unable to initialize si3050 Codec for port %d",
                   __FUNCTION__, port);
        }
    }
}



/**********************************************************************
 *  Function: silab_fxo_lpbk_test
 *
 *  Description: Loopback test from ARM->TDM->FXO codec and back.
 *           This function is wrapper for the test, which tests
 *           ARM-TDM loopbacks.  The ARM will send a packet to
 *           the fxo through the tdm switch. The codec on the FXO is
 *           programmed in loopback mode. The packet is looped back
 *           from the codec through the TDM switch back to ARM.
 *           Example: TDM connection for ARM :
 *           ARM->TDM in stream 64->out stream 4->in stream 4->
 *           out stream 64.
 *
 *  Input: None
 *
 *  Returns:  PASSED or FAILED
 * ******************************************************************** 
 */
int silab_fxo_lpbk_test (void)
{
    oak_port_cfg port_cfg;
    int port, tdm_bus;
    char errstr[128];

    oak_fxo_ports(&port_cfg);

    /*
     * FXS has no power on control, but there is register CRR to reset the chip
     */
    reset_si3050();		/* Reset 3050 */
    for (port = port_cfg.start; port < (port_cfg.size + port_cfg.start); port++) {
         bsp_debug_printf("\r\n TDM FXO Codec Digital lpbk. port: %d\r", port);

        if (si3050_codec_digital_loopback(port) == FAILED) {
            tdm_bus = get_tdm_bus_num(port, FXO_TYPE);
            cterr('f', 0, "port %x Codec lpbk test failed.", port);
            sprintf(errstr, "\nport%d (TDM bus%d) loopback failed.\n", port, tdm_bus);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    }

    sprintf((char *)&(hd_if->bufmsg), "FXO Loopback test done!\n");


    return (PASSED);
}


/**********************************************************************
 *
 * Function: codec_si3050_read_write_reg
 *
 * Description: This send command to SI3050 to read or write register.
 *
 * Input : port, read/write flag
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si3050_read_write_reg (int port, int rd_wt_flg)
{
    int ret_code;
    int reg_num, data;


    reg_num = gethex_answer("\nEnter reg# to read/write",1,1,59);
    if (rd_wt_flg == TRUE) {
        ret_code = si3050_reg_read(port, reg_num, &data);
    } else {
        data = gethex_answer("\nEnter data to write",0,0,0xff);
        ret_code = si3050_reg_write(port, reg_num, data);
    }

    if (ret_code != PASSED) {
        cterr('f',0,"Codec port %d - %s reg %x failed", port,
			rd_wt_flg ? "read" : "write", reg_num);
    } else {
        if (rd_wt_flg == TRUE) {
            bsp_debug_printf("\nreg read is : 0x%2x\n", data & 0xff);
        }
    }

    return (ret_code);
}

/**********************************************************************
 *
 * Function: codec_si3050_read_rev
 *
 * Description: This send command to 3050 to get its rev.
 *
 * Input : port
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si3050_read_rev (int port)
{
    int ret_code, rev_code;

    ret_code = si3050_reg_read(port, SYSTEM_SIDE_CHIP_REV_REG, &rev_code);

    if (ret_code != PASSED) {
        cterr('f',0,"port %d SI3050 Read Codec Rev command failed.", port);
        return (FAILED);
    } else {
        if (((rev_code & 0xff) & SI3050_LINEID_MASK) == SILAB_ID_3019) {
            bsp_debug_printf("\nSi3050 Rev is %#X\n", (rev_code & 0xF)+0x9);
        } else {
            cterr('f',0,"Wrong Line side chip. The ID value is %x", rev_code);
            return (FAILED);
        }
    }

    ret_code = si3050_reg_read(port, LINE_SIDE_DEV_REV_REG, &rev_code);

    if (ret_code != PASSED) {
        cterr('f',0,"port %d SI3050 Read Line side Rev failed.", port);
        return(FAILED);
    } else {
        if ((rev_code & 0xC3) == 0x40) {
            /* Bit 6 has to be 1. Bits 7, 1:0 must be 0 */
            bsp_debug_printf("\nSi3050 Rev is %#X\n", ((rev_code >> 2) & 0xF) + 9);
        } else {
            cterr('f', 0, "Line-Side Device Revision wrong. Read %x", rev_code);
            return (FAILED);
        }
    }
    return (ret_code);
}


/**********************************************************************
 *
 * Function: si3050_codec_utilies
 *
 * Description: This utility is the FXO utilities
 *
 * Input : None
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int si3050_codec_utilities (void)
{
    oak_port_cfg port_cfg;
    int tst_item, port;

    oak_fxo_ports(&port_cfg);

    reset_si3050();		/* Reset 3050 */

    PRINT_STR("\r\r");
    PRINT_STR("1. SI3050 FXO Codec Reg Read\r");
    PRINT_STR("2. SI3050 FXO Codec Reg Write\r");
    PRINT_STR("3. Read 3050 Rev\r");
    PRINT_STR("0. Quit only\r");
    tst_item = gethex_answer("                Select an option > ", 0, 0, 3);

    if (tst_item) {


        port = gethex_answer("\nEnter voice port. ", port_cfg.start,
                              port_cfg.start, port_cfg.start+port_cfg.size);

        switch (tst_item) {
        case 1:  /* reg read */
            return (codec_si3050_read_write_reg(port, TRUE));
            break;
        case 2: /* reg write */
            return (codec_si3050_read_write_reg(port, FALSE));
            break;
        case 3: /* read rev */
            return (codec_si3050_read_rev(port));
            break;
        default:
            break;
        }
    }

    return (PASSED);
}

/******** History ********
$Log: fxo_test.c,v $
Revision 1.5  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.4  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.3.20.4  2018/05/09 18:33:45  haohsu
Fixed 2FXS/2FXO TDM mapping issue

Revision 1.3.20.3  2018/05/08 23:03:49  haohsu
Suport FXO Ring test

Revision 1.3.20.2  2018/04/13 03:18:01  haohsu
Code Change for Vg400 (return calibration value, skip FXO loopback in FXO SKus)

Revision 1.3.20.1  2018/01/26 09:42:04  haohsu
*** empty log message ***

Revision 1.3  2017/08/09 08:12:25  harrchan
Display TDM bus number when FXS/FXO loopback fail

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.5  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.1.2.4  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.1.2.3  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield




$Endlog$
*/


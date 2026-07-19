/* $Id: si3xxx_utils.c,v 1.4 2021/04/15 00:53:07 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/si3xxx_utils.c,v $ 
 *------------------------------------------------------------------
 * si3xxx_utils.c
 * Utils for FXS and FXO
 *
 * Oct 2016 - Owen Lin
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
#include "libtdm.h"
#include "libuart.h"
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "libppbint.h"
#include "arm_common.h"
#include "diag_ppb.h"
#include "si3xxx_utils.h"
#include "tdm_utils.h"
#include "si3226x.h"
#include "si3226x_registers.h"
#include "diag_fpga.h"


/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
int si3xxx_reg_read(int, int, int*);
int si3xxx_reg_write(int, int, int);
int si3xxx_reg_write_calibr(int* ,int ,int ,int);
int send_tdm_packet(uint8_t * ,int ,uint16_t ,uint16_t num_channel);
int send_tdm_packet_calibr(void);
ulong si3xxx_read_reg(uint8_t * ,int ,int *);
int send_6pkt_to_codec(int ,int ,uint16_t * ,int ,int);
int send_6pkt_to_codec_cal(int * ,int ,int ,uint16_t * ,int ,int);
int si_write_ram_codec(int ,uint ,uint16_t, uint16_t);
int si_write_ram_codec_calibr(int * ,int ,uint ,uint16_t ,uint16_t);
int si_read_ram_codec(int ,uint ,uint16_t* ,uint16 *);


/*===================================================================*
 *                             Globals                               *
 *===================================================================*/

uint8_t rd_packet[80];
uint8_t wr_packet[80];
uint8_t packetsize = 80;

extern volatile dspif_info_t *hd_if;

uint32_t codec_tdm_intr_received[MAX_NUM_PORT] = {0};


/***********************************************************************
 *
 * Function: tdm_isr
 *
 * This function set the tdm interrupt.
 *
 * Input : none
 *
 * Returns: none
 *
 **********************************************************************
 */
void tdm_isr(void)
{
    int ix;
    uint32_t intr_status;

    /* check which port received interrupt */
    for (ix = 0; ix < MAX_NUM_PORT; ix++) {
        if ((intr_status = sp_TdmIdentifyIntr(ix))!=0) {
            /* clear it */
            sp_TdmIntrClr(ix, intr_status);

            /* Keep port running */
            sp_TdmStop(TDM(ix));

            /* set the flag */
            codec_tdm_intr_received[ix] = 1;
        }
    }

    /* clear interrupt in the core */
}
/***********************************************************************
 *
 * Function: phoenix_port_transfer_which_to_tdm_port 
 *
 * This function will transfer pheonix which port to tdm port
 * different project.
 *
 * Input : board_id, which_port, codec_type
 *
 * Returns: TDM port. FAILED for the unknown board ID.
 *
 **********************************************************************
 */
int phoenix_port_transfer_which_to_tdm_port (uchar board_id, int which_port, 
        int codec_type)
{
    int tdm_port = 0xFF;

    switch(board_id) {
    case PHOENIX_144FXS:
        if (codec_type == FXS_CODEC) {
            if (which_port < 12) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else if (which_port < 24 )  {
                tdm_port = (which_port - which_port%2)*2 + 8 + which_port%2;
            } else if (which_port < 36 )  {
                tdm_port = (which_port - which_port%2)*2 + 16 + which_port%2;
            } else if (which_port < 48 )  {
                tdm_port = (which_port - which_port%2)*2 + 24 + which_port%2;
            } else {
                tdm_port = (which_port - which_port%2)*2 + 32 + which_port%2;
            }
        }
        break;
    case PHOENIX_132FXS_6FXO:
    case PHOENIX_84FXS_6FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 12) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else if (which_port < 24 )  {
                tdm_port = (which_port - which_port%2)*2 + 8 + which_port%2;
            } else if (which_port < 36 )  {
                tdm_port = (which_port - which_port%2)*2 + 16 + which_port%2;
            } else {
                tdm_port = (which_port - which_port%2)*2 + 56 + which_port%2;
            }
        } else {
                tdm_port = (which_port * 4) + 96;
        }
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return (FAILED);
        break;
    }
    return (tdm_port);
}
/***********************************************************************
 *
 * Function: get_tdm_port
 *
 * This function returns TDM porti.This formula will change base on 
 * different project.
 *
 * Input : which_port, codec_type
 *
 * Returns: TDM port. FAILED for the unknown board ID.
 *
 **********************************************************************
 */
int get_tdm_port (int which_port, int codec_type)
{
    uchar board_id = get_oak_id();
    int tdm_port = 0xFF;

    /* Please see the FPGA TDM Bus mapping Spec to set TDM Bus 
     * VG400 use same FXS TDM mapping as Oakenshield, but different 
     * mapping on FXO
     */

    switch(board_id) {
    case BOARD_16FXS_2FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        } else {
            tdm_port = which_port*4 + 32;
        }
        break;
    case BOARD_24FXS_4FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        } else {
            tdm_port = which_port*4 + 32;
        }
        break;
    case BOARD_8FXS_12FXO:
        if (codec_type == FXS_CODEC) {
            tdm_port = (which_port - which_port%2)*2 + which_port%2;
        } else {
            if (which_port < 4) {
                tdm_port = which_port*4 + 32;
            } else {
                tdm_port = (which_port - 4)*4 + 96;
            }
        }
        break;
    case BOARD_72FXS:
        if (codec_type == FXS_CODEC) {
            if (which_port < 34) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 34 - which_port%2)*2 + 96 + which_port%2;
            }
        }
        break;
    case VG400_2FXS_2FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        } else {
                tdm_port = (which_port -4)*4 + 96 ;
        }
        break;
    case VG400_4FXS_4FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        } else {
            if (which_port < 4) {
                tdm_port = which_port*4 + 32;
            } else {
                tdm_port = (which_port - 4)*4 + 96;
            }
        }
        break;
    case VG400_6FXS_6FXO:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        } else {
            if (which_port < 4) {
                tdm_port = which_port*4 + 32;
            } else {
                tdm_port = (which_port - 4)*4 + 96;
            }
        }
        break;
    case VG400_8FXS:
        if (codec_type == FXS_CODEC) {
            if (which_port < 8) {
                tdm_port = (which_port - which_port%2)*2 + which_port%2;
            } else {
                tdm_port = (which_port - 8 - which_port%2)*2 + 64 + which_port%2;
            }
        }
        break;
    case PHOENIX_144FXS:
    case PHOENIX_132FXS_6FXO:
    case PHOENIX_84FXS_6FXO:
        tdm_port = phoenix_port_transfer_which_to_tdm_port(board_id, which_port,
                codec_type);
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return (FAILED);
        break;
    }

    return (tdm_port);

}

/***********************************************************************
 *
 * Function: get_chan
 *
 * This function returns correct TDM channel.
 *
 * Input : tdm_port, channel_type
 *
 * Returns: Channel. FAILED for the unknown TDM port. 
 *
 **********************************************************************
 */
int get_chan(int tdm_port, int channel_type)
{
    int channel = 0xFF;
    uchar board_id = get_oak_id();
    
    if ((board_id == BOARD_72FXS) || (is_phoenix())) {
        if (channel_type == VOICE) {
            channel = tdm_port;
        } else {
            channel = tdm_port - tdm_port%2 + 2;
        }
    } else {
        if (tdm_port < 32) {
            if (channel_type == VOICE) {
                channel = tdm_port;
            } else {
                channel = tdm_port - tdm_port%2 + 2;
            }
        } else if (tdm_port < 64) {
            if (channel_type == VOICE) {
                channel = tdm_port;
            } else {
                channel = tdm_port + 2;
            }

        } else if (tdm_port < 96) {
            if (channel_type == VOICE) {
                channel = tdm_port;
            } else {
                channel = tdm_port - tdm_port%2 + 2;
            }

        } else if (tdm_port < 128) {
            if (channel_type == VOICE) {
                channel = tdm_port;
            } else {
                channel = tdm_port + 2;
            }
        } else {
            bsp_debug_printf("****Wrong TDM port, %d\n\r", tdm_port);
        }
    }
    return (channel);
}




/***********************************************************************
 *
 * Function: build_channel_num_arr
 *
 * This function will build channel number array for calibration load patch
 *
 * Input : which_port_arr[], max_fxs_port
 *
 * Returns: void
 *
 **********************************************************************
 */
int build_channel_num_arr(int *channel_num_arr, int max_fxs_ports)
{
    int port, tdm_port;
    uchar board_id = get_oak_id();
    
    for (port = 0; port < max_fxs_ports; port++) {         
        if (port%2 == 0) {
            switch(board_id) {
            case BOARD_16FXS_2FXO:
                if (port < 8) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 8 - port%2)*2 + 64 + port%2;
                }
                break;
            case BOARD_24FXS_4FXO:
                if (port < 8) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 8 - port%2)*2 + 64 + port%2;
                }
                break;
            case BOARD_8FXS_12FXO:
                tdm_port = (port - port%2)*2 + port%2;
                break;
            case BOARD_72FXS:
                if (port < 34) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 34 - port%2)*2 + 96 + port%2;
                }
                break;
            case VG400_2FXS_2FXO:
                if (port < 8) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 8 - port%2)*2 + 64 + port%2;
                }
                break;
            case VG400_4FXS_4FXO:
                if (port < 8) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 8 - port%2)*2 + 64 + port%2;
                }
                break;
            case VG400_6FXS_6FXO:
                if (port < 8) {
                    tdm_port = (port - port%2)*2 + port%2;
                } else {
                    tdm_port = (port - 8 - port%2)*2 + 64 + port%2;
                }
                break;
            case VG400_8FXS:
                tdm_port = (port - port%2)*2 + port%2;
                break;
            case PHOENIX_144FXS:
            case PHOENIX_132FXS_6FXO:
            case PHOENIX_84FXS_6FXO:
                tdm_port = phoenix_port_transfer_which_to_tdm_port(board_id, 
                        port, FXS_CODEC);
                break;
            default:
                cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
                return (FAILED);
                break;
            }
        }

        if (port%2 == 0) {
            if ((board_id == BOARD_72FXS) || (is_phoenix())) {
                channel_num_arr[port] = tdm_port - tdm_port%2 + 2;
            } else {
                if (tdm_port < 32) {
                    channel_num_arr[port] = tdm_port - tdm_port%2 + 2;
                } else if (tdm_port < 64) {
                    channel_num_arr[port] = tdm_port + 2;
                } else if (tdm_port < 96) {
                    channel_num_arr[port] = tdm_port - tdm_port%2 + 2;
                } else if (tdm_port < 128) {
                    channel_num_arr[port] = tdm_port + 2;
                } else {
                    bsp_debug_printf("****Wrong TDM port\n\r");
                    cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
                    return (FAILED);
                }
            }
        }

    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: is_fxo
 *
 * This function will check whether port is FXO or not
 *
 * Input : which_port
 *
 * Returns: TRUE/FALSE
 *
 **********************************************************************
 */
int is_fxo(int tdm_port)
{
    uchar board_id = get_oak_id();
    if (is_phoenix()) {
        if ((board_id != PHOENIX_144FXS) && (tdm_port >= 96 && tdm_port < 128)) {
            return (TRUE);
        }
        return (FALSE);
    } else if (board_id != BOARD_72FXS) {
        if ((tdm_port >= 32 && tdm_port < 64) ||
            (tdm_port >= 96 && tdm_port < 128)) {
            return (TRUE);
        }
    }
    return (FALSE);
}


/***********************************************************************
 *
 * Function: build_monitor_packet
 *
 * Description: initialize test destination and source buffers 
 *
 * Input : source buffer and destination buffer 
 *			data buffer, voice port
 *
 * Returns: None 
 *
 **********************************************************************
 */
static void build_monitor_packet(uint8_t *packet, uint16_t tdm_port)
{
    uint32_t ix, jx, channel_num, channel_num_remap;
    uint8_t monitor[40];
    uint8_t ci[40];
    uint32_t swbuf = NUM_OF_SIU0;

    for (ix = 0; ix < 40; ix++) {
        monitor[ix] = *packet++;
        ci[ix] = *packet++;
    }

    /*****************************/
    /* initialize source arrays  */
    /*****************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0; ix < COLS; ix++) {
        for (jx = 0; jx < (2*ROWS); jx++) {
            SWTU0_SOURCEBUFFER[swbuf][ix][jx] = 0x0;
        }
    }

    /* copy frame */
    /* Refer to Connection Memory */
    channel_num = get_chan(tdm_port, MONITOR);

    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        /* channel_num need remapping start from 0 on SIU1 */
        channel_num_remap = channel_num - PHOENIX_DB3_START_TDM7_PORT224;
        for (jx = 0; jx < 40; jx++) {
            SWTU0_SOURCEBUFFER[1][channel_num_remap][jx] = monitor[jx];
            SWTU0_SOURCEBUFFER[1][channel_num_remap + 1][jx] = ci[jx];
        }
    } else {
        for (jx = 0; jx < 40; jx++) {
            SWTU0_SOURCEBUFFER[0][channel_num][jx] = monitor[jx];
            SWTU0_SOURCEBUFFER[0][channel_num + 1][jx] = ci[jx];
        }
    }

    /************************************/
    /* clear out TDM desintation array  */
    /************************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0; ix < COLS; ix++) {
        for (jx = 0; jx < (ROWS*2); jx++) {
            SWTU0_DESTINATIONBUFFER[swbuf][ix][jx] = 0x0;
        }
    }

}

/***********************************************************************
 *
 * Function: build_voice_packet
 *
 * Description: initialize test destination and source buffers 
 *
 * Input : source buffer and destination buffer 
 *		data buffer; voice port
 *
 * Returns: None 
 *
 **********************************************************************
 */
static void build_voice_packet(uint8_t *packet, uint16_t tdm_port)
{

    uint32_t ix, jx, channel_num, channel_num_remap;
    uint32_t swbuf = NUM_OF_SIU0;

    /*****************************/
    /* initialize source arrays  */
    /*****************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0; ix < COLS; ix++) {
        for (jx = 0; jx < (2 * ROWS); jx++) {
            SWTU0_SOURCEBUFFER[swbuf][ix][jx] = 0x0;
        }
    }

    /* copy frame */
    /* Refer to Connection Memory */
    channel_num = get_chan(tdm_port, VOICE);
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        /* channel_num need remapping start from 0 on SIU1 */
        channel_num_remap = channel_num - PHOENIX_DB3_START_TDM7_PORT224;
        for (jx = 0; jx < 80; jx++) {
            SWTU0_SOURCEBUFFER[1][channel_num_remap][jx] = *packet++;
        }
    } else {
        for (jx = 0; jx < 80; jx++) {
            SWTU0_SOURCEBUFFER[0][channel_num][jx] = *packet++;
        }
    }

    /************************************/
    /* clear out TDM desintation array  */
    /************************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= tdm_port ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0; ix < COLS; ix++) {
        for (jx = 0; jx < (ROWS*2); jx++) {
            SWTU0_DESTINATIONBUFFER[swbuf][ix][jx] = 0xff;
        }
    }
}


/***********************************************************************
 *
 * Function: build_frame_packet
 *
 * Description: initialize test destination and source buffers 
 *
 * Input : source buffer and destination buffer 
 *		data buffer; voice port
 *
 * Returns: None 
 *
 **********************************************************************
 */
static void build_frame_packet(uint8_t *packet, uint16_t num_channel)
{
    uint32_t ix, jx, kx, num_channel_remap;
    uint32_t swbuf = NUM_OF_SIU0;

    /*****************************/
    /* initialize source arrays  */
    /*****************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= num_channel ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0, kx = 1; ix < COLS; ix++,kx++) {
        for (jx = 0; jx < (2*ROWS); jx++) {
            SWTU0_SOURCEBUFFER[1][ix][jx] = 0x0;
        }
    }
    /* copy frame */
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= num_channel ) {
        num_channel_remap = num_channel - PHOENIX_DB3_START_TDM7_PORT224;
        for (ix = 0; ix < num_channel_remap; ix++) {
            SWTU0_SOURCEBUFFER[1][ix][0] = *packet++;
        }
    } else {
        for (ix = 0; ix < num_channel; ix++) {
            SWTU0_SOURCEBUFFER[0][ix][0] = *packet++;
        }
    }
    /************************************/
    /* clear out TDM desintation array  */
    /************************************/
    /*
     * Each SIU only could handle max 255 tdm port.
     * Phoenix TDM port more than 255, so need to adopt SIU 1 sourcebuff
     */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= num_channel ) {
        swbuf = NUM_OF_SIU1;
    }
    for (ix = 0; ix < COLS; ix++) {
        for (jx = 0; jx < (ROWS*2); jx++) {
            SWTU0_DESTINATIONBUFFER[swbuf][ix][jx] = 0xff;
        }
    }
}


/***********************************************************************
 *
 * Function: build_monitor_packet_calibr
 *
 * Description: initialize test destination and source buffers 
 *
 * Input : source buffer and destination buffer 
 *			data buffer, voice port
 *
 * Returns: None 
 *
 **********************************************************************
 */
static void build_monitor_packet_calibr(uint8_t *packet, uint8_t channel_num)
{

	uint32_t ix, jx, num_remap = 0;
	uint8_t monitor[40];
	uint8_t ci[40];

	for (ix = 0; ix < (packetsize/2); ix++) {
		monitor[ix] = *packet++;
		ci[ix] = *packet++;
	}

	/* copy frame */
    /* Refer to Connection Memory */

    /* Phoenix tdm_port or channel_num over 223 will use SIU1 not SIU0 */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= channel_num) {
        num_remap = channel_num - PHOENIX_DB3_START_TDM7_PORT224;
        for (jx = 0; jx < 40; jx++) {
            SWTU0_SOURCEBUFFER[1][num_remap][jx] = monitor[jx];
            SWTU0_SOURCEBUFFER[1][num_remap + 1 ][jx] = ci[jx];
        }
    } else {
        for (jx = 0; jx < 40; jx++) {
            SWTU0_SOURCEBUFFER[0][channel_num][jx] = monitor[jx];
            SWTU0_SOURCEBUFFER[0][channel_num + 1][jx] = ci[jx];
        }
    }

    /************************************/
    /* clear out TDM desintation array  */
    /************************************/
    /* Remove below function because
     * All caller function already clear by himself
     * this function just set one port but duplicate clear all port
     *
     * Phoenix tdm_port or channel_num over 223 will use SIU1 not SIU0

    if (PHOENIX_DB3_START_TDM7_PORT224 <= channel_num) {
        for (ix = 0; ix < COLS; ix++) {
            for (jx = 0; jx < (ROWS*2); jx++) {
                SWTU0_DESTINATIONBUFFER[1][ix][jx] = 0xff;
            }
        }
    } else {
        for (ix = 0; ix < COLS; ix++) {
            for (jx = 0; jx < (ROWS*2); jx++) {
                SWTU0_DESTINATIONBUFFER[0][ix][jx] = 0xff;
            }
        }
    }
    */

}

/***********************************************************************
 *
 * Function: fill_codec_tx_buff_wr
 *
 * This function fills the xmit buffer with the codec 
 * write command in the required format.
 * buffer length : 80 bytes. (40 bytes each channel) 
 * 
 * Input : which_port, first_reg, data, int *ptr, 
 *         cibits
 *
 * Returns: PASSED 
 *
 **********************************************************************
 */
int 
fill_codec_tx_buff_wr (int tdm_port, int first_reg,  
			int data, uint16_t *ptr, int cibits)
{
    int ix, channel;

    /* Idle  */
    for (ix = 0; ix < 4; ix++) {        /* 00: 0xff03 */
        *ptr++ = SWAP16(0xff03);     /* 01: 0xff03 */
    }                                /* 02: 0xff03 */
                                     /* 03: 0xff03 */

    /* 								
     * Write HW address ch0: 0x8900, ch 1: 0x9100		
     */ 
    channel =  (tdm_port) % 2;

    if ((is_fxo(tdm_port) == TRUE) || (channel == 0)) {
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 | cibits); /* 04:0x9100|0x0010|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 | cibits);
    } else {
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 | cibits); /* 04:0x8900|0x0010|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 | cibits);
    }

    /* 
     * Write command, SI3XXX_WRITE = 0x0100 
     */
    *ptr++ = SWAP16(SI3XXX_WRITE | 0x03 | cibits); /* 06:0x0100|0x0003|cibits */
    *ptr++ = SWAP16(SI3XXX_WRITE | 0x02 | cibits); /* 07:0x0100|0x0002|cibits */

    /* 
     * Write first register number to be written
     */
    /* 08:0xXX00|0x03|cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x03 | cibits); 

    /* 09: 0xXX00 | 0x02 |cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x02 | cibits); 

    /* 
     * Fill in the value to be written in the register.
     */
    /* 10: 0xXX00 | 0x03 | cibits */
    *ptr++ = SWAP16(((data << 8) & 0xff00) | 0x03 | cibits); 	  
    /* 11: 0xXX00 | 0x02 | cibits */
    *ptr++ = SWAP16(((data << 8) & 0xff00) | 0x02 | cibits); 	 

    *ptr++ = SWAP16(0xff03);  /* EOM*/		  /* 12: 0xff03 */
    *ptr++ = SWAP16(0xff03);

    /*
     * Send the address again to read back from the codec.
     */

    channel = (tdm_port) % 2;
    if ((is_fxo(tdm_port) == TRUE) || (channel == 0)) {
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 |cibits); /* 14:0x9100|0x0002|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 |cibits); 
    } else {
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 |cibits); /* 14:0x8900|0x0002|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 |cibits); 
    }

    /* 
     * Read back from the codec 
     */
    *ptr++ = SWAP16(SI3XXX_READ | 0x03 | cibits); /* 16:0x8100|0x0003|cibits */
    *ptr++ = SWAP16(SI3XXX_READ | 0x02 | cibits); /* 17:0x8100|0x0002|cibits */

    /* 
     * Write first register number to be read
     */
    /* 18: 0xXX00 | 0x03 |cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x03 | cibits); 
    /* 19: 0xXX00 | 0x02 |cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x02 | cibits); 

    *ptr++ = SWAP16(0xff03);  /* EOM*/			/* 20: 0xff03 */
    *ptr++ = SWAP16(0xff03);

    /* Fill buffer for turnaround time to receive 
       response from Codec */
    *ptr++ = SWAP16(0xff01| cibits);		/* 22: 0xff01 | cibits */
    *ptr++ = SWAP16(0xff01| cibits);
 
    *ptr++ = SWAP16(0xff03| cibits);		/* 24: 0xff03 | cibits */
    *ptr++ = SWAP16(0xff01| cibits);		/* 25: 0xff01 | cibits */

    for (ix = 0; ix < 4; ix++) {
	    *ptr++ = SWAP16(0xff03| cibits);	/* 26: 0xff03 | cibits */
    }						                /* 27: 0xff03 | cibits */
						                    /* 28: 0xff03 | cibits */
    return (PASSED);				        /* 29: 0xff03 | cibits */

}

/***********************************************************************
 *
 * Function: fill_codec_tx_buff_rd
 *
 * This function fills the xmit buffer with the codec
 * read command in the required format.
 * buffer length : 80 bytes.
 *
 * Input : which_port, first_reg, data, int *ptr,
 *         cibits
 *
 * Returns: none
 *
 **********************************************************************
 */
int
fill_codec_tx_buff_rd (int tdm_port, int first_reg, int num_reg,
                         uint16_t *ptr, int cibits)
{
    int ix, channel;

    /* Idle  */
    for (ix = 0; ix < 4; ix++) {
        *ptr++ = SWAP16(0xff03);
    }
    /*
     *  Write HW address ch0: 0x8900, or ch 1: 0x9100
     */
    channel =  (tdm_port) % 2;

    if ((is_fxo(tdm_port) == TRUE) || (channel == 0)) {
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 |cibits);
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 |cibits);
    } else {
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 |cibits);
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 |cibits);
    }

    /*
     * Read back from the codec, SI3050_READ = 0x8100
     */
    *ptr++ = SWAP16(SI3XXX_READ | 0x03 | cibits);
    *ptr++ = SWAP16(SI3XXX_READ | 0x02 | cibits);

    /*
     * Write first register number to be read
     */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x03 | cibits);
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x02 | cibits);

    *ptr++ = SWAP16(0xff03);  /* EOM*/
    *ptr++ = SWAP16(0xff03);

    /* add 2 more for Motalvo */
    if (num_reg == 4) {
        *ptr++ = SWAP16(0xff03);  /* EOM*/
        *ptr++ = SWAP16(0xff03);
    }

    /* Fill buffer for turnaround time to receive
       response from Codec */


    *ptr++ = SWAP16(0xff01| cibits);
    *ptr++ = SWAP16(0xff01| cibits);

    for (ix = 0; ix < num_reg; ix++) {
        *ptr++ = SWAP16(0xff03| cibits);
        *ptr++ = SWAP16(0xff01| cibits);
    }
    for (ix = 0; ix < 4; ix++) {
        *ptr++ = SWAP16(0xff03| cibits);
    }

    return (PASSED);

}

/***********************************************************************
 *
 * Function: fill_codec_wr_buff_6
 *
 * This function fills the xmit buffer of mcbspx with the codec 
 * write command in the required format.
 * buffer length : 38 words = 76 bytes. but only use 64 bytes
 * 
 * Input : which_port, first_reg, data, int *ptr, 
 *         cibits
 *
 * Returns: none
 *
 **********************************************************************
 */

int fill_codec_wr_buff_6(int tdm_port, int first_reg, uint16_t *data, uint16_t *ptr, int cibits)
{
    int ix, channel;

    /* Idle  */
    for (ix = 0; ix < 4; ix++) {   /* 00: 0xff03 */
        *ptr++ = SWAP16(0xff03);   /* 01: 0xff03 */
    }                              /* 02: 0xff03 */
                                   /* 03: 0xff03 */
    /* 								
     * Write HW address ch0: 0x8900, ch 1: 0x9100		
     */ 
    channel = (tdm_port) % 2;
    if (channel == 0) {
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 | cibits); /* 04:0x9100|0x0010|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_0 | 2 | cibits);
    } else {
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 | cibits); /* 04:0x8900|0x0010|cibits */
        *ptr++ = SWAP16(HW_ADDR_CH_1 | 2 | cibits);
    }

    /* 
     * Write command, SI33XX0_WRITE = 0x0100 
     */
    *ptr++ = SWAP16(SI3XXX_WRITE | 0x03 | cibits); /* 06:0x0100|0x0003|cibits */
    *ptr++ = SWAP16(SI3XXX_WRITE | 0x02 | cibits); /* 07:0x0100|0x0002|cibits */

    /* 
     * Write first register number to be written
     */
    /* 08:0xXX00|0x03|cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x03 | cibits); 

    /* 09: 0xXX00 | 0x02 |cibits */
    *ptr++ = SWAP16(((first_reg << 8) & 0xff00) | 0x02 | cibits); 

    /* 
     * Fill in the value to be written in the register.
     */
    for (ix = 0; ix < 6; ix++) {
        /* 10: 0xXX00 | 0x03 | cibits */
        *ptr++ = SWAP16(((data[ix] ) & 0xff00) | 0x03 | cibits); 	  
        /* 11: 0xXX00 | 0x02 | cibits */
        *ptr++ = SWAP16(((data[ix] ) & 0xff00) | 0x02 | cibits); 	 

    }
    *ptr++ = SWAP16(0xff03);  /* EOM*/  		  /* 12: 0xff03 */
    *ptr++ = SWAP16(0xff03);
    *ptr++ = SWAP16(0xff03);  /* EOM*/			  /* 12: 0xff03 */
    *ptr++ = SWAP16(0xff03);

    return (PASSED);
}


/***********************************************************************
 *
 * Function: send_6pkt_to_codec
 *
 * This function sends read/write commands over Monitor channels.
 * 
 * Input :  which_port, 
 *	        first_reg, data, cibits, 
 *          channel_type
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int 
send_6pkt_to_codec (int which_port, int first_reg, uint16_t *data, 
		     int cibits, int channel_type)
{
    uint8_t *tx_buf_ptr;
    uint8_t *packet;

    /* 
     * We have BUFF_SIZE of 0x200, so watch out for size of data
     * transfer in the following. The block_size should not exceed 0x400
     */
    tx_buf_ptr= &wr_packet[0];

    /* use the original init_buffer or use memset */
    memset(&wr_packet[0], 0x0, sizeof(wr_packet));

    /* fill TX buffer with the packet to be sent to codec */

    fill_codec_wr_buff_6(which_port, first_reg, data,
			       (uint16_t*)tx_buf_ptr, cibits);

    packet = tx_buf_ptr;

    /* send the packet through tdm channels */
    /* parameter 1 is for build frame packet but we didn't use it */
    if (send_tdm_packet(packet,channel_type, which_port, 1)) {
        return (FAILED);
    } 
    return (PASSED);
}

/***********************************************************************
 *
 * Function: send_pkt_to_codec
 *
 * This function sends read/write commands over Monitor channels.
 * 
 * Input : which_port,
 *         first_reg, num_reg, *data, cibits, write
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
int send_pkt_to_codec (int which_port, 
        int which_reg, int num_reg, 
        int *data, int cibits, 
        int channel_type, int command, int which_siu)
{

    uint8_t *packet;
    char errstr[128];

    /* init read, write buffers */
    memset(&rd_packet[0], 0xff, sizeof(rd_packet));
    memset(&wr_packet[0], 0xff, sizeof(wr_packet));

     if (command == WRITE) {
          fill_codec_tx_buff_wr(which_port, which_reg, *data,
                (uint16_t *)wr_packet, cibits);
          packet = &wr_packet[0];
     } else if (command == READ) {
          fill_codec_tx_buff_rd(which_port, which_reg, num_reg,
                   (uint16_t *)rd_packet, cibits);
          packet = &rd_packet[0];
     }

     /* send the packet through tdm channels */
    if (send_tdm_packet(packet, channel_type, which_port, 1)) {
        cterr('f', 0,"\rsend tdm packet reg:%x\n\r",which_reg);
        sprintf(errstr, "\nsend tdm reg:%x\n",which_reg);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: send_6pkt_to_codec_cal
 *
 * This function sends read/write commands over Monitor channels.
 * 
 * Input :  which_port, 
 *	   first_reg, data, cibits, 
 *         channel_type
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
int 
send_6pkt_to_codec_cal (int *channel_num_arr, int max_fxs_ports,
             int first_reg, uint16_t *data, int cibits, int channel_type)
{
    uint8_t *tx_buf_ptr;
    uint8_t *packet;
    int ix, jx, tdm_port, channel_num;


	/*****************************/
	/* initialize source arrays  */
	/*****************************/
	for (ix = 0; ix < COLS; ix++) {
		for (jx = 0; jx < (2*ROWS); jx++) {
			SWTU0_SOURCEBUFFER[0][ix][jx] = 0x0;
            /* Add for Phoenix has SIU1 */
			SWTU0_SOURCEBUFFER[1][ix][jx] = 0x0;
		}
	}


    tdm_port = 0;
    /* 
     * We have BUFF_SIZE of 0x200, so watch out for size of data
     * transfer in the following. The block_size should not exceed 0x400
     */
    tx_buf_ptr= &wr_packet[0];

    /* use the original init_buffer or use memset */
    memset(&wr_packet[0], 0x0, sizeof(wr_packet));

    /* fill TX buffer with the packet to be sent to codec */

    fill_codec_wr_buff_6(tdm_port, first_reg, data,(uint16_t*)tx_buf_ptr, cibits);
    packet = &tx_buf_ptr[0];
    for (ix = 0; ix < max_fxs_ports; ix++) {
        if (ix % 2 == 0) {
            channel_num = channel_num_arr[ix];
        }
        if (channel_type == MONITOR) {
            build_monitor_packet_calibr(packet, channel_num);
        }		    
    }

    /************************************/
    /* clear out TDM desintation array  */
    /************************************/
    for (ix = 0; ix < COLS; ix++) {
		for (jx = 0; jx < (ROWS*2); jx++) {
            SWTU0_DESTINATIONBUFFER[0][ix][jx] = 0x0;
            /* Add for Phoenix has SIU1 */
            SWTU0_DESTINATIONBUFFER[1][ix][jx] = 0x0;
        }
    }

    /* send the packet through tdm channels */
    if (send_tdm_packet_calibr()) {
        return(FAILED);
    } 


    return PASSED;

}
/***********************************************************************
 *
 * Function: send_tdm_packet
 *
 * This function sends TDM packet
 * 
 * Input : which_port,
 *     first_reg, num_reg, *data, cibits, write
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
int send_tdm_packet(uint8_t *packet, int channel_type, uint16_t which_port,
                    uint16_t num_channel)
{
    uint32_t  no_ports;
    uint32_t *src_buf_base, *dst_buf_base;
    int32_t retval;
    int32_t ix, tdm_siu = 0, wastetime = SEND_TDM_PACKET_DELAY;
    uint8_t star_num_of_siu = 0,number_of_siu = 1;
    char errstr[128];

    codec_tdm_intr_received[0] = 0;
    codec_tdm_intr_received[1] = 0;


    /* enabling TDM interrupt */
    sp_InitInterruptController(); /* enable interrupt controller */

    /* Connect TDM_isr to TDM interrupt */
    sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_TDM), TYPE_INT_TDM, PRIORITY0, 
                    &tdm_isr);

    /* initialize UART */
    SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

    /* Phoenix will adopte SIU1 port when tdm_port over 224 */ 
    if (PHOENIX_DB3_START_TDM7_PORT224 <= which_port ) {
        no_ports = NUMBER_OF_PHOENIX_SIU;
        star_num_of_siu = NUM_OF_SIU1;
    } else {
        no_ports = number_of_siu;
        star_num_of_siu = NUM_OF_SIU0;
    }
    retval = 0;
    for (ix = star_num_of_siu; ix < no_ports; ix++) {
#ifdef SHOW_DEBUG
        PRINT_STR("  PORT");
        PRINT_DEC(ix);
        PRINT_STR(" channel initialization starts..\r");

        PRINT_STR("  - Initialize source memory space at ");
        PRINT_HEX((uint32_t)&SWTU0_SOURCEBUFFER[ix][0][0]);
        PRINT_STR(" .");
#endif
         
        if (channel_type == MONITOR) {
            build_monitor_packet(packet, which_port);
        } else if (channel_type == VOICE) {
            build_voice_packet(packet, which_port);
        } else if (channel_type == FRAME) {
            build_frame_packet(packet, num_channel);
        }

        /* all channels unmasked by default */
        sp_TdmDisableInChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);
        /* all channels unmasked by default */
        sp_TdmDisableOutChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);

        sp_TdmSIUConfExtLB_usingBB(TDM(ix), NUMBER_OF_CHANNEL, BIT_SIZE_8, CH_MODE);

        /* enable channels */
        sp_TdmEnableInChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);
        sp_TdmEnableOutChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);

        /* swtu_conf==TWO_DIMENSIONAL */ 
        /* configuration for new two-dimensional mode */
        sp_TdmSWTU2Dconfig(TDM(ix), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC, 
                           (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);

        /* address translation for DSS core */
        src_buf_base = (uint32_t*)&SWTU0_SOURCEBUFFER[ix][0][0];
        dst_buf_base = (uint32_t*)&SWTU0_DESTINATIONBUFFER[ix][0][0];

        /* swtu_conf == TWO_DIMENSIONAL */
        sp_Tdm2DInitChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1, 
                         N_BITS_PER_SAMPLE_PER_CH, NUMBER_OF_SAMPLE_BUFFER, 
                         src_buf_base, dst_buf_base, NOT_USING_UNIVERSAL_COUNTER, 
                         BUFFERING_OPTION);
    }

    /* Enable interrupt before we start TDM */
    for (ix = star_num_of_siu; ix < no_ports; ix++) {
        sp_TdmIntrInit(TDM(ix), TDM_INTR_SWTU_DST);
    }

    for (ix = star_num_of_siu; ix < no_ports; ix++) {
        sp_TdmInternalClkRun(TDM(ix));
        sp_TdmRun(TDM(ix));
    }

    if (PHOENIX_DB3_START_TDM7_PORT224 <= which_port ) {
        tdm_siu = NUM_OF_SIU1;
    } else {
        tdm_siu = NUM_OF_SIU0;
    }
    /* wait until core receives interrupt signals from 1 ports */
    /* SR Put timing detail here so the test does not hang */
    while (!(codec_tdm_intr_received[tdm_siu])) {
        if (wastetime == 0)
            break;
        /*
         * change sleep from millisecond to microsecond
         * It will speed up received
         */
        usleep(1);
        wastetime--;
    }

    if (wastetime == 0) {
        PRINT_STR(" No TDM interrupt received \r");
        retval = 1;
        PRINT_STR(" Send TDM packet FAILED");
        cterr('f', 0, "Send TDM Failed");

        sprintf(errstr, "\nTDM interrupt Error.\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        bsp_debug_printf("TDM STOP\n");
        sp_TdmStop(tdm_siu);

        return (retval);
    }

    if (retval) {
        PRINT_STR("\n\r Send TDM packet FAILED");
        cterr('f', 0, "Send TDM Failed");
    } 
    sp_TdmStop(tdm_siu);
    return (retval);
}

/***********************************************************************
 *
 * Function: send_pkt_to_codec_calibr
 *
 * This function sends read/write commands over Monitor channels.
 * 
 * Input : which_port,
 *     first_reg, num_reg, *data, cibits, write
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
int send_pkt_to_codec_calibr (int *channel_num_arr, int max_fxs_ports, int which_reg,
                int num_reg, int *data, int cibits, int channel_type, int command, int which_siu)
{
    uint8_t *packet;
    int ix, jx, tdm_port, channel_num;

    /* init read, write buffers */
    memset(&wr_packet[0], 0xff, sizeof(wr_packet));
    tdm_port = 0;                              /* even port channel*/
    if (command == WRITE) {
        fill_codec_tx_buff_wr(tdm_port, which_reg, *data, (uint16_t *)wr_packet, cibits);
        packet = &wr_packet[0];
    }
    for (ix = 0; ix < max_fxs_ports; ix++) {   /* even port need load patch */
        if (ix%2 == 0) {
            channel_num = channel_num_arr[ix];
        }

        if (channel_type == MONITOR) {
            build_monitor_packet_calibr(packet, channel_num);
        }
    }

	/************************************/
	/* clear out TDM desintation array  */
	/************************************/
    /* Phoenix tdm_port or channel_num over 223 will use SIU1 not SIU0 */
    for (ix = 0; ix < COLS; ix++) {
		for (jx = 0; jx < (ROWS*2); jx++) {
            SWTU0_DESTINATIONBUFFER[0][ix][jx] = 0x0;
            /* Add for Phoenix has SIU1 */
            SWTU0_DESTINATIONBUFFER[1][ix][jx] = 0x0;
		}
	}

     /* send the packet through tdm channels */
    if (send_tdm_packet_calibr()) {
        return (FAILED);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: send_tdm_packet_calibr
 *
 * This function sends TDM packet
 * 
 * Input : none
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int send_tdm_packet_calibr(void)
{
    uint32_t no_ports;
    uint32_t *src_buf_base, *dst_buf_base;
    int32_t retval;
    int32_t ix, wastetime = SEND_TDM_PACKET_DELAY;
    uint8_t number_of_siu = 1;
    char errstr[128];

    codec_tdm_intr_received[0] = 0;
    codec_tdm_intr_received[1] = 0;

    /* enabling TDM interrupt */
    sp_InitInterruptController(); /* enable interrupt controller */

    /* Connect TDM_isr to TDM interrupt */
    sp_SetInterrupt(HW_INT_NUM_TO_ID(INT_TDM), TYPE_INT_TDM, PRIORITY0, 
                    &tdm_isr);

    /* initialize UART */
    SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

    if(is_phoenix() && phoenix_has_dbx(BOARD_DB3_TEST)) {
        no_ports = NUMBER_OF_PHOENIX_SIU;
    } else {
        no_ports = number_of_siu;
    }
    retval = 0;
    for (ix = 0; ix < no_ports; ix++) {
#ifdef SHOW_DEBUG
        PRINT_STR("  PORT");
        PRINT_DEC(ix);
        PRINT_STR(" channel initialization starts..\r");

        PRINT_STR("  - Initialize source memory space at ");
        PRINT_HEX((uint32_t)&SWTU0_SOURCEBUFFER[ix][0][0]);
        PRINT_STR(" .");
#endif

        /* all channels unmasked by default */
        sp_TdmDisableInChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);
        /* all channels unmasked by default */
        sp_TdmDisableOutChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);

        sp_TdmSIUConfExtLB_usingBB(TDM(ix), NUMBER_OF_CHANNEL, BIT_SIZE_8, CH_MODE);

        /* enable channels */
        sp_TdmEnableInChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);
        sp_TdmEnableOutChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1);


        /* swtu_conf==TWO_DIMENSIONAL */ 
        /* configuration for new two-dimensional mode */
        sp_TdmSWTU2Dconfig(TDM(ix), AUTOLOAD_ON, SIGCON_LCOL2_LROW_INTC, 
                           (BUFFERING_OPTION+1)*NUMBER_OF_SAMPLE_BUFFER);

        /* address translation for DSS core */
        src_buf_base = (uint32_t*)&SWTU0_SOURCEBUFFER[ix][0][0];
        dst_buf_base = (uint32_t*)&SWTU0_DESTINATIONBUFFER[ix][0][0];

        /* swtu_conf == TWO_DIMENSIONAL */
        sp_Tdm2DInitChan(TDM(ix), 0, NUMBER_OF_CHANNEL-1, 
                         N_BITS_PER_SAMPLE_PER_CH, NUMBER_OF_SAMPLE_BUFFER, 
                         src_buf_base, dst_buf_base, NOT_USING_UNIVERSAL_COUNTER, 
                         BUFFERING_OPTION);
    }

    /* Enable interrupt before we start TDM */
    for (ix = 0; ix < no_ports; ix++) {
        sp_TdmIntrInit(TDM(ix), TDM_INTR_SWTU_DST);
    }

    for (ix = 0; ix < no_ports; ix++) {
        sp_TdmInternalClkRun(TDM(ix));
        sp_TdmRun(TDM(ix));

    }

    /* wait until core receives interrupt signals from 1 ports */
    /* SR Put timing detail here so the test does not hang */
    if (is_phoenix() && phoenix_has_dbx(BOARD_DB3_TEST)) {
        /* Check SIU0 and SIU1 both receive interrupt */
        while ((!(codec_tdm_intr_received[TDM(0)])) || (!(codec_tdm_intr_received[TDM(1)]))) {
            if (wastetime == 0)
                break;

            usleep(1);
            wastetime--;
        }
    } else {
        while (!(codec_tdm_intr_received[TDM(0)])) {
            if (wastetime == 0)
                break;

            usleep(1);
            wastetime--;
        }
    }

    if (wastetime == 0) {
        PRINT_STR(" No TDM interrupt received \r");
        retval = 1;
        PRINT_STR(" Send TDM packet FAILED");
        cterr('f', 0, "Send TDM Failed");

        sprintf(errstr, "\nTDM interrupt Error.\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        bsp_debug_printf("TDM STOP\n");
        if (is_phoenix() && phoenix_has_dbx(BOARD_DB3_TEST)) {
            sp_TdmStop(TDM(0));
            sp_TdmStop(TDM(1));
        } else {
            sp_TdmStop(TDM(0));
        }
        return (retval);
    }

    if (retval) {
        PRINT_STR("\n\r Send TDM packet FAILED");
        cterr('f', 0, "Send TDM Failed");
    }
    if (is_phoenix() && phoenix_has_dbx(BOARD_DB3_TEST)) {
        sp_TdmStop(TDM(0));
        sp_TdmStop(TDM(1));
    } else {
        sp_TdmStop(TDM(0));
    }


    return (retval);
}
/***********************************************************************
 *
 * Function: verify_si3xxx_lpbk_digital_data
 *
 * This function verifies the data received in the receive buffer
 * of the voice channel, after si32261 codec Digital loopback.
 * This isifor increase pattedern checking, somehow 5 data wrap
 * to the beginning of the rx_buf 
 * Input :  channel, port, data, siu 
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
ulong
verify_si3xxx_lpbk_digital_data (int channel, int which_port, uint16_t data)
{
    uint8_t data_tx, data_rx;
    int rx_size;
    uint8_t *rx_buf_ptr;
    int i, found = FALSE, num_of_chan, num_of_chan_remap;

    num_of_chan = get_chan(which_port, VOICE);
    /* Phoenix port over 244 need use SIU1 and remap */
    if (PHOENIX_DB3_START_TDM7_PORT224 <= num_of_chan ) {
        num_of_chan_remap = num_of_chan - PHOENIX_DB3_START_TDM7_PORT224;
        rx_buf_ptr = (uint8_t *)&SWTU0_DESTINATIONBUFFER[1][num_of_chan_remap][0];
    } else {
        rx_buf_ptr = (uint8_t *)&SWTU0_DESTINATIONBUFFER[0][num_of_chan][0];
    }
    if (channel == 0) {
        data_tx = (data & 0xff00) >> 8;
    } else {
        data_tx = (data & 0xff);
    }

    for (rx_size = 0; rx_size < 80 ; rx_size++) {
        data_rx = *rx_buf_ptr;
        if (data_rx != data_tx) {
            rx_buf_ptr++;
            continue;
        } else {
            found = TRUE;
            for (i = 0; i < 39; i++) { /* should have 40 */
                if (data_rx != data_tx) {
                    return (FAILED);
                }
                rx_buf_ptr++;
            }	
	        break;
        }
    }

    if (found == FALSE) {
        return(FAILED);
    }


    return (PASSED);
}


/***********************************************************************
 *
 * Function: si3050_reg_write
 *
 * This function writes CODEC/DAA registers.
 * Input : port number, register number, data
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 **********************************************************************
 */
int si3050_reg_write (int which_port, int which_reg, int data)
{
    int tdm_port;
    tdm_port = get_tdm_port(which_port, FXO_CODEC); 
    return (send_pkt_to_codec(tdm_port, which_reg, 1, &data, 0, MONITOR, 
                              WRITE, 0));

}

/***********************************************************************
 *
 * Function: si3xxx_reg_write
 *
 * This function writes CODEC/DAA registers.
 * Input : port number, register number, data
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 **********************************************************************
 */
int si3xxx_reg_write (int which_port, int which_reg, int data)
{
    int tdm_port;

    tdm_port = get_tdm_port(which_port, FXS_CODEC); 

    return (send_pkt_to_codec(tdm_port, which_reg, 1, &data, 0, MONITOR, 
                              WRITE, 0));

}

/***********************************************************************
 *
 * Function: si3xxx_reg_write_calibr
 *
 * This function writes CODEC/DAA registers.
 * Input : port number, register number, data
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 **********************************************************************
 */
int si3xxx_reg_write_calibr (int *channel_num_arr, int max_fxs_ports,
                int which_reg, int data)
{

    return (send_pkt_to_codec_calibr(&channel_num_arr[0],
                             max_fxs_ports,which_reg, 1, &data, 0, MONITOR, WRITE, 0));

}
/**********************************************************************
 *
 * Function: si3050_reg_read
 *
 * This function reads CODEC/DAA registers.
 * Input : port number, register number, pointer to data
 *
 * Returns: PASSED for no errors;
 *             FAILED for test failure.
 *
 **********************************************************************
 */
int si3050_reg_read (int which_port, int which_reg, int* data)
{
    int num_of_chan, port;
    char errstr[128];

    port = get_tdm_port(which_port, FXO_CODEC); 

    if (send_pkt_to_codec(port, which_reg, 1, data, 0, MONITOR, READ, 0)) {
        return (FAILED);
    }

    msleep(5);

    /* use siu0 */
    num_of_chan = get_chan(port, MONITOR);
    if (si3xxx_read_reg((uint8_t *)&SWTU0_DESTINATIONBUFFER[0][num_of_chan][0],
                        port, data) == FAILED) {
        bsp_debug_printf("Read si3xxx register fail\n");
        sprintf(errstr,"\nRead si3xxx register fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: si3xxx_reg_read
 *
 * This function reads CODEC/DAA registers.
 * Input : port number, register number, pointer to data
 *
 * Returns: PASSED for no errors;
 *             FAILED for test failure.
 *
 **********************************************************************
 */
int si3xxx_reg_read (int which_port, int which_reg, int* data)
{
    int num_of_chan, num_of_chan_remap, tdm_port;
    char errstr[128];

    tdm_port = get_tdm_port(which_port, FXS_CODEC); 

    if (send_pkt_to_codec(tdm_port, which_reg, 1, data, 0, MONITOR, READ, 0)) {
        return (FAILED);
    }
    msleep(1);

    /* use siu0 */
    num_of_chan = get_chan(tdm_port, MONITOR);
    /* Phoenix will adopte SIU1 port when tdm_port over 224 */ 
    if (PHOENIX_DB3_START_TDM7_PORT224 <= num_of_chan) {
        /* num_of_chan need remapping start from 0 on SIU1 */
        num_of_chan_remap = num_of_chan - PHOENIX_DB3_START_TDM7_PORT224;
        if(si3xxx_read_reg((uint8_t *)&SWTU0_DESTINATIONBUFFER[1][num_of_chan_remap][0],
                            tdm_port, data)==FAILED) {
            bsp_debug_printf("Read si3xxx register fail\n");
            sprintf(errstr, "\nRead si3xxx reg fail");
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    } else {
        if(si3xxx_read_reg((uint8_t *)&SWTU0_DESTINATIONBUFFER[0][num_of_chan][0],
                            tdm_port, data)==FAILED) {
            bsp_debug_printf("Read si3xxx register fail\n");
            sprintf(errstr, "\nRead si3xxx reg fail");
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: si3xxx_read_reg
 *
 * This function reads the codec registers
 * 
 * Input : buf_ptr, which_port 
 *
 * Returns: none
 *
 **********************************************************************
 */
ulong si3xxx_read_reg (uint8_t *rx_buf_ptr, int which_port, int *data)
{
    int buf_size = 0x40;
    int i, channel_a_b;

    if ((is_fxo(which_port) == TRUE) || ((which_port % 2) == 0)) {
        channel_a_b = 0x91; 
    } else {
        channel_a_b = 0x89;
    }

    /* search for first word of received data, by adjust the first few
	idle bytes in the transmit buffer */
    for (i = 0; i < buf_size; i++) {
        if  ((*(rx_buf_ptr + i) ) == channel_a_b ) {
            if  ((*(rx_buf_ptr + i + 1) ) == channel_a_b) {
                rx_buf_ptr += (i + 2);
                break;
            }
        }
    }

    if (i == buf_size) {
        return(FAILED);
    }

    *(data) = *(rx_buf_ptr);

    return (PASSED);
}


/***********************************************************************
 *
 * Function: si_write_ram_codec
 *
 * this one is for write RAM codec
 *
 * Input: which_port, location, data_hi, data_lo
 * Output: PASS/FAIL
 *
***********************************************************************
 */
int si_write_ram_codec (int which_port, unsigned int location, uint16_t val_hi, uint16_t val_lo)
{
    uint16_t temp_hi, temp_lo;
    uint16_t snd_data[6];
    int data, cibits = 0;
    int tdm_port;

    tdm_port = get_tdm_port(which_port, FXS_CODEC);

    temp_hi = temp_lo = 0;
    /*
     * write adress high first
     */
    temp_hi = location;
    temp_hi &= 0xFF00;
    temp_hi = temp_hi >> 3; /* it needs shift R 8, then shit L 5 */
    data = temp_hi;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[0] = SWAP16(temp_hi);
    } else {
        if (si3xxx_reg_write(which_port, RAM_ADDR_HI, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b0
     */
    temp_lo = val_lo;
    temp_lo = temp_lo << 3;
    temp_lo &= 0xFF;
    data = temp_lo;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[1] = SWAP16(temp_lo);
    } else {
        if (si3xxx_reg_write(which_port, RAM_DATA_B0, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b1
     */
    temp_lo = val_lo;
    temp_lo = temp_lo << 3;
    temp_lo &= 0xFF00;
    data = temp_lo >> 8;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[2] = SWAP16(temp_lo>>8);
    } else {
        if (si3xxx_reg_write(which_port, RAM_DATA_B1, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b2
     */
    temp_lo = val_lo;
    temp_lo &= 0xFF00;
    temp_lo = temp_lo >> 5; /* shift R 8, then shift L 3 */
    temp_lo &= 0xFF00;
    temp_hi = val_hi;
    temp_hi = temp_hi << 3;
    temp_lo = (temp_lo >> 8) | (temp_hi & 0xFF);
    data = temp_lo;


    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[3] = SWAP16(temp_lo);
    } else {
        if (si3xxx_reg_write(which_port, RAM_DATA_B2, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b3
     */
    temp_hi = val_hi;
    temp_hi = temp_hi << 3;
    temp_hi &= 0xFF00;
    data = temp_hi >> 8;


    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[4] = SWAP16(temp_hi>>8);
    } else {
        if (si3xxx_reg_write(which_port, RAM_DATA_B3, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write address low to latch it
     */
    temp_lo = location;
    temp_lo &= 0xFF;
    data = temp_lo;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[5] = SWAP16(temp_lo);

        if (send_6pkt_to_codec(tdm_port,
                               RAM_ADDR_HI,&snd_data[0],
                               cibits, MONITOR)) {
            return(FAILED);
        }
    } else {
        if (si3xxx_reg_write(which_port, RAM_ADDR_LO, data) == FAILED) {
            return(FAILED);
        }
    }

    return (PASSED);


}

/***********************************************************************
 *
 *si_write_ram_codec_calibr()
 *
 * this one is for write RAM codec
 *
 * Input: which_port, location, data_hi, data_lo
 * Output: PASS/FAIL
 *
***********************************************************************
 */
int si_write_ram_codec_calibr (int *channel_num_arr, int max_fxs_ports, 
                    uint location, uint16_t val_hi, uint16_t val_lo)
{
    uint16_t temp_hi, temp_lo;
    uint16_t snd_data[6];
    int data, cibits = 0;

    temp_hi = temp_lo = 0;
    /*
     * write adress high first
     */
    temp_hi = location;
    temp_hi &= 0xFF00;
    temp_hi = temp_hi >> 3; /* it needs shift R 8, then shit L 5 */
    data = temp_hi;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[0] = SWAP16(temp_hi);
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                     RAM_ADDR_HI, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b0
     */
    temp_lo = val_lo;
    temp_lo = temp_lo << 3;
    temp_lo &= 0xFF;
    data = temp_lo;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[1] = SWAP16(temp_lo);
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                                    RAM_DATA_B0, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b1
     */
    temp_lo = val_lo;
    temp_lo = temp_lo << 3;
    temp_lo &= 0xFF00;
    data = temp_lo >> 8;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[2] = SWAP16(temp_lo>>8);
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                                    RAM_DATA_B1, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b2
     */
    temp_lo = val_lo;
    temp_lo &= 0xFF00;
    temp_lo = temp_lo >> 5; /* shift R 8, then shift L 3 */
    temp_lo &= 0xFF00;
    temp_hi = val_hi;
    temp_hi = temp_hi << 3;
    temp_lo = (temp_lo >> 8) | (temp_hi & 0xFF);
    data = temp_lo;


    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[3] = SWAP16(temp_lo);
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                                    RAM_DATA_B2, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write data_b3
     */
    temp_hi = val_hi;
    temp_hi = temp_hi << 3;
    temp_hi &= 0xFF00;
    data = temp_hi >> 8;


    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[4] = SWAP16(temp_hi>>8);
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                                    RAM_DATA_B3, data) == FAILED) {
            return(FAILED);
        }
    }

    /*
     * write address low to latch it
     */
    temp_lo = location;
    temp_lo &= 0xFF;
    data = temp_lo;

    if ((location == SI3226x_PRAM_ADDR) ||
        (location == SI3226x_PRAM_DATA)) {
        snd_data[5] = SWAP16(temp_lo);

        if (send_6pkt_to_codec_cal(&channel_num_arr[0], max_fxs_ports,
                               RAM_ADDR_HI,&snd_data[0],cibits, MONITOR)) {
            return(FAILED);
        }
        
    } else {
        if (si3xxx_reg_write_calibr(&channel_num_arr[0], max_fxs_ports,
                              RAM_ADDR_LO, data) == FAILED) {
            return(FAILED);
        }
    }

    return (PASSED);


}


/***********************************************************************
 *
 * Function: si32xx_read_4reg
 *
 * This function reads the codec 4 registers
 * 
 * Input :  port, pointers to parm_1, parm_2, parm_3, and WhoAmI.
 *
 * Returns: 
 *
 **********************************************************************
 */
ulong 
si32xx_read_4reg (uint8_t *rx_buf_ptr, uint * parm_1, uint * parm_2, uint * parm_3,
		  uint * WhoAmI)
{
    int data_rx1;
    int buf_size = 0x40;
    int ix, channel_a_b = 0x91;

    /* search for first word of received data, by adjust the first few
	idle bytes in the transmit buffer */
    for (ix = 0; ix < buf_size; ix++) {
        if ( (*(rx_buf_ptr + ix) ) == channel_a_b ) {
            if ( (*(rx_buf_ptr + ix + 1) ) == channel_a_b ) {
                rx_buf_ptr += (ix + 2);
                break;
            }
        }
    }
    for (ix = 0; ix < 4; ix++) {
        data_rx1 = *(rx_buf_ptr + 2*ix) ;
        switch(ix) {
        case 0:
            *parm_1 = data_rx1;
            break;
        case 1:
            *parm_2 = data_rx1;
            break;
        case 2:
            *parm_3 = data_rx1;
            break;
        case 3:
            *WhoAmI = data_rx1;
            break;
        }
     }

    return (PASSED);
}



/***********************************************************************
 *
 * Function: si_read_ram_codec
 *
 * This function read a value from an  internal Ram.
 *
 * Input : which_port, location, *parm1, *parm2
 *
 * Returns: ram_value 
 *
 *
***********************************************************************
 */
int si_read_ram_codec (int which_port, uint location,
                       uint16_t *parm1, uint16_t *parm2)
{
    int wait1 = 0x20;
    volatile uint read_val = 1;
    uint temp_hi, temp_lo, temp, parm_2;

    temp_hi = temp_lo = 0;

    /* read RAMSTAT bit 0 to indicate previous
     * ram access is completed and ready
     */

    while ((wait1 != 0) && (read_val != RAM_ADDR_READY_TO_ACCESS)) {

        if (si3xxx_reg_read (which_port, RAMSTAT, (int *)&read_val) == FAILED) {
            cterr('f', 0, "%s RAM address %x error", __FUNCTION__, location);
            return (FAILED);
        } 
        --wait1;
    }

    if (wait1 == 0) {
        /* if failed, return both 0xFFFF */
        *parm1 = *parm2 = 0xFFFF;
        cterr('f', 0, "%s: RAMSTAT = %x. Timedout", __FUNCTION__, read_val);
        return (FAILED);
    }
    /*
     * write address to hi first
     */

    temp_hi = location;
    temp_hi &= 0x700;

    if (si3xxx_reg_write (which_port, RAM_ADDR_HI, temp_hi >> 3) == FAILED) {
        cterr('f', 0, "%s: RAM Hi %#x write failure", __FUNCTION__, location);
        return (FAILED);
    }

    /*
     * write address to low
     */
    temp_lo = location;
    temp_lo &= 0xFF;

    if (si3xxx_reg_write(which_port, RAM_ADDR_LO, temp_lo) == FAILED) {
        cterr('f', 0, "%s: RAM Lo %#x write failure", __FUNCTION__, temp_lo);
        return (FAILED);
    }

    /*
     * make sure the RAM can be access
     */
    wait1 = 0x20;
    while ((wait1 != 0) && (read_val != RAM_ADDR_READY_TO_ACCESS)) {

        if (si3xxx_reg_read(which_port, RAMSTAT, (int *)&read_val) == FAILED) {
            cterr('f', 0, "%s: RAMSTAT read %#x failure", __FUNCTION__,
                  location);
            return (FAILED);
        }
        --wait1;
    }

    if (wait1 == 0) {
        /* if failed, return both 0xFFFF */
        *parm1 = *parm2 = 0xFFFF;
        cterr('f', 0, "%s: Wrote RAM address. But timedout. RAMSTAT = %#x",
              __FUNCTION__, read_val);
        return (FAILED);
    }
    msleep(1);
    /*
     * clear the buffer first
     */
    temp_lo = temp_hi = 0;

    /* Add here to read the register 4 at a time for the 0x54f */

    if (si3xxx_reg_read(which_port, RAM_DATA_B0, (int *)&temp_lo) == FAILED) {
        cterr('f', 0, "%s: Unable to read B0", __FUNCTION__);
        return (FAILED);
    }
    /* check the value read */
    temp_lo = (temp_lo & 0xff)>>3;

    if (si3xxx_reg_read(which_port, RAM_DATA_B1, (int *)&temp) == FAILED) {
        cterr('f', 0, "%s: Unable to read B1", __FUNCTION__);
        return (FAILED);
    }

    /* check the value read */
    temp = temp & 0xFF;
    temp_lo |= (temp << 5);

    if (si3xxx_reg_read (which_port, RAM_DATA_B2, (int *)&parm_2) == FAILED) {
        cterr('f', 0, "%s: Unable to read B2", __FUNCTION__);
        return (FAILED);
    }

    /* check the value read */
    temp = parm_2 & 0x07;
    temp_lo |= (temp << 13); /* this is the correct value */
    temp_hi = (parm_2 & 0xff)>> 3;

    if (si3xxx_reg_read (which_port, RAM_DATA_B3, (int *)&parm_2) == FAILED) {
        cterr('f', 0, "%s: Unable to read B3", __FUNCTION__);
        return (FAILED);
    }

    /* check the value read */
    temp = parm_2 & 0xFF;
    temp_hi |= (temp << 5);

    /*
     * So we need to return temp_hi and temp_lo, but DSP5510
     */
    *parm1 = temp_hi;
    *parm2 = temp_lo;
    msleep(5);

    return (PASSED);
}



/******** History ********
$Log: si3xxx_utils.c,v $
Revision 1.4  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.2  2018/05/08 23:03:49  haohsu
Suport FXO Ring test

Revision 1.2.28.1  2018/02/06 09:26:17  haohsu
Code change for VG400

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.9  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.1.2.8  2017/04/17 06:08:46  olin2
Remove Enable PLL function

Revision 1.1.2.7  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.1.2.6  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.1.2.5  2017/02/09 06:41:05  olin2
Support voltage margin and fail over port utility

Revision 1.1.2.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.3  2017/01/05 06:06:33  olin2
Support FXS Ring and Calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield



$Endlog$
*/


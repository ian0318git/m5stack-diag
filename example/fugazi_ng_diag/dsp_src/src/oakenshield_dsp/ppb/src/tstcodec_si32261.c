/* $Id: tstcodec_si32261.c,v 1.4 2021/04/15 00:53:07 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/tstcodec_si32261.c,v $ 
 *------------------------------------------------------------------
 * tstcodec_si32261.c
 * Utility for SI32261
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
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "libgpio.h"
#include "fxs_test.h"
#include "si3226x_registers.h"
#include "si3226x.h"
#include "si3xxx_utils.h"
#include "si3226x_patch_C_FB_2014JUN18.h"
#include "si3226x_QCUK_constants.h"
#include "diag_fpga.h"


int si32261_codec_stop_ring(int);
int si32261_codec_set_ring(int);
int si32261_codec_set_onoff_hook_map(int);
int si32261_codec_digital_loopback(int);
int si32261_protected_mode(int);
int si32261_common_mode_calibration(int, uint *);
int si_write_reg_32261(int , uint, uint);
int si_write_reg_32261_calibr(int * , int , uint , int);
int si_read_reg_32261(int , uint , uint *);
int si_write_ram_32261(int , unsigned int , uint16_t , uint16_t);
int si_write_ram_32261_calbri(int , int , unsigned int , uint16_t , uint16_t);
int si_read_ram_32261(int , unsigned int , uint16_t *, uint16_t *);
uint8_t tst_packet[80];
extern int do_calibration_flag;



/**********************************************************************
 *
 * Function: si_read_reg_32261
 *
 * This function read a value to from Ram.
 *
 * Input : which_port, location, *where
 *
 * Returns: PASSED/FAILED
 *
 *
 **********************************************************************
 */
int si_read_reg_32261(int which_port, uint location, uint *value)
{
    return (si3xxx_reg_read (which_port, location, (int *)value));
}


/**********************************************************************
 *
 * Function: si_write_reg_32261
 *
 * This function write a value to internal Reg.
 *
 * Input : which_port, location, value
 *
 * Returns: PASSED/FAILED
 *
 *
 **********************************************************************
 */
int si_write_reg_32261(int tdm_port, uint location, uint value)
{
    int result = PASSED;

    result = si3xxx_reg_write(tdm_port, location, value);

    return (result);

}

/**********************************************************************
 *
 * Function: si_write_reg_32261_calibr
 *
 * This function write a value to internal Ram for calibration optimize.
 *
 * Input : which_port, location, value
 *
 * Returns: PASSED/FAILED
 *
 *
 **********************************************************************
 */
int si_write_reg_32261_calibr(int *channel_num_arr, int max_fxs_ports, 
                              uint location, int value)
{
    return (send_pkt_to_codec_calibr(&channel_num_arr[0], max_fxs_ports,
                   location, 1, &value, 0, MONITOR, WRITE, 0));
}



/**********************************************************************
 *
 * Function: si_read_ram_32261
 *
 * This function reads CODEC/DAA registers.
 * Input : which_port, reg, *data, *data_lo
 *
 * Returns: PASSED for no errors;
 *             FAILED for test failure.
 *
 **********************************************************************
 */
int si_read_ram_32261(int which_port, unsigned int reg, uint16_t *data, uint16_t *data_lo)
{
    int ret_val = PASSED;

    ret_val = si_read_ram_codec(which_port, reg, (uint16_t *)data,
                (uint16_t *)data_lo);

    return (ret_val);
}

/***********************************************************************
 *
 * Function: si_write_ram_32261
 *
 * This function writes CODEC/DAA ram.
 * Input : port number
 *
 * Returns: PASSED for no errors;
 *             FAILED for test failure.
 *
 **********************************************************************
 */
int si_write_ram_32261(int which_port, unsigned int which_reg, uint16_t data_hi, uint16_t data_lo)
{
    int ret_val = PASSED;

    ret_val = si_write_ram_codec(which_port, which_reg, data_hi, data_lo);

    return (ret_val);

}


/*********************************************************************
 *
 * si_write_ram_32261_wrap
 *
 * This function is for writing to Silab codec
 *
 * Input: which_port, location, register
 *
 * OUtput: PASSED/FAILED
 *
 **********************************************************************
 */
int si_write_ram_32261_wrap (int which_port, unsigned int location, unsigned int reg)
{

    if (si_write_ram_32261(which_port, location, (reg & 0xFFFF0000) >> 16, 
                          (reg & 0xFFFF))) {
        return (FAILED);
    }
    return (PASSED);

}

/***********************************************************************
 *
 * Function: si_write_ram_32261_calibr
 *
 * This function writes CODEC/DAA ram for calibration optimaize.
 * Input : port number
 *
 * Returns: PASSED for no errors;
 *             FAILED for test failure.
 *
 **********************************************************************
 */
int si_write_ram_32261_calibr(int *channel_num_arr, int max_fxs_ports,
                        unsigned int which_reg, uint16_t data_hi, uint16_t data_lo)
{
    int ret_val = PASSED;

    ret_val = si_write_ram_codec_calibr(&channel_num_arr[0],
                               max_fxs_ports,which_reg, data_hi, data_lo);


    return (ret_val);

}

/*********************************************************************
 *
 * Function: si32261_load_patch
 *
 * This function download the patch file
 * Input : which port
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 **********************************************************************
 */
int si32261_load_patch (int which_port)
{
    volatile uint16_t temp_hi, temp_lo;
    unsigned int loop, reg_data;
    int jmp_table_low = JMP_TABLE_LOW;
    int jmp_table_hi = JMP_TABLE_HIGH;

    bsp_debug_printf("\n\r Load Patch \r");

    si_write_reg_32261(which_port, JMPEN, 0); /* disable Patch RAM */

    /*
     * make sure JMPEN = 0
     */
    do {
        si_read_reg_32261(which_port, JMPEN, &reg_data);
        if ((reg_data & 0xff) != 0 ) {
           si_write_reg_32261(which_port, JMPEN, 0);
        }
    } while ((reg_data & 0xff) == 1);

    /* Zero out jump table in case previous values are still loaded */
    for (loop = 0; loop < 8; loop++) {
        /* Zero out jump table */
        si_write_reg_32261(which_port, jmp_table_low, 0);
        si_write_reg_32261(which_port, jmp_table_low + 1, 0);
        jmp_table_low+=2;
    }

    for (loop = 0; loop < 8; loop++) {
        /* Zero out jump table */
        si_write_ram_32261(which_port, jmp_table_hi, 0, 0);
        jmp_table_hi++;
    }

    if (si_write_ram_32261(which_port, SI3226x_PRAM_ADDR, 0, 0)) {
        return (FAILED);
    }

    /*
     * If the data is all 0, you have hit the end of the programmed
     * values and can stop loading.
     */

    for (loop = 0; loop < 1024; loop++) {
        temp_hi = (((patchData[loop] << 9) & 0xffff0000) >> 16);
        temp_lo = ((patchData[loop] << 9) & 0xffff);
        if ((temp_hi == 0) && (temp_lo == 0)) {
            loop = 1024;
        } else {
            if (si_write_ram_32261(which_port, SI3226x_PRAM_DATA, 
                                    temp_hi, temp_lo)) {
                return (FAILED);
            }
        }
    }

    si_write_reg_32261(which_port, RAM_ADDR_HI,0);

    /* Load the jump table with the passed values. */
    jmp_table_low = 82;
    for (loop = 0; loop < 8; loop++) {
        if (patchEntries[loop] != 0) {
            si_write_reg_32261(which_port,jmp_table_low,
                patchEntries[loop]&0xff);

            si_write_reg_32261(which_port,jmp_table_low+1,
                patchEntries[loop]>>8);
        }
        jmp_table_low+=2;
    }

    jmp_table_hi = 1597;
    for (loop = 0; loop < 8; loop++) {
        if (patchEntries[loop + 8] != 0){
            si_write_ram_32261(which_port, jmp_table_hi, 0,
                (patchEntries[loop + 8]&0x1fff));
        }
                jmp_table_hi++;
    }

    /**
     * Write patch support RAM locations (if any) 
     */
    for (loop = 0; loop < 128; loop++) {
        if(patchSupportAddr[loop] != 0) {
            si_write_ram_32261(which_port, patchSupportAddr[loop],
                ((patchSupportData[loop]&0x1fff0000) >> 16),
                (patchSupportData[loop]&0xffff));
        } else {
            loop = 128;
        }
    }

    /* leave the JMPEN 0 disable the jmp table */
    si_write_reg_32261(which_port, JMPEN, 1); /* enable the patch */
    si_read_reg_32261(which_port, JMPEN, &reg_data);
    if (((reg_data) & 0xff) == 0 ) {
       si_write_reg_32261(which_port, JMPEN, 1);
    }
    return (PASSED);
}

/*********************************************************************
 *
 * Function:phoenix_separ_dbx_port_translate 
 *
 * This function will skip MB/BDx port if user don't test it
 * Input : port = FXS port porint
 *         max_ports = max FXS port
 *         board_id = board ID 
 *
 * Returns: FXS port
 *
 **********************************************************************
 */
int phoenix_separ_dbx_port_translate (int *port, int max_ports, char board_id)
{
    if (board_id == PHOENIX_144FXS) {
        if ((*port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
            *port = FXS_PORT23; /* escape MB FXS port */
            return (TRUE);
        } else if ((FXS_PORT24 <= *port) && (*port < FXS_PORT48) &&
                   (!phoenix_has_dbx(BOARD_DB1_TEST))) {
            *port = FXS_PORT47;
            return (TRUE);
        } else if ((FXS_PORT48 <= *port) && (*port < FXS_PORT96) &&
                   (!phoenix_has_dbx(BOARD_DB2_TEST))) {
            *port = FXS_PORT95;
            return (TRUE);
        } else if ((FXS_PORT96 <= *port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
            *port = max_ports;
            return (TRUE);
        }
    } else { /* 132FXS and 84 FXS case */
        if ((*port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
            *port = FXS_PORT23;
            return (TRUE);
        } else if ((FXS_PORT24 <= *port) && (*port < FXS_PORT36) &&
                   (!phoenix_has_dbx(BOARD_DB1_TEST))) {
            *port = FXS_PORT35;
            return (TRUE);
        } else if ((FXS_PORT36 <= *port) && (*port < FXS_PORT84) &&
                   (!phoenix_has_dbx(BOARD_DB2_TEST))) {
            *port = FXS_PORT83;
            return (TRUE);
        } else if ((FXS_PORT84 <= *port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
            *port = max_ports;
            return (TRUE);
        }
    }

    return (FALSE);
}
/*********************************************************************
 *
 * Function: si32261_load_patch
 *
 * This function download the patch file
 * Input : which port
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 **********************************************************************
 */
int si32261_load_patch_cal (int max_fxs_ports)
{
    volatile uint16_t temp_hi, temp_lo;
    unsigned int loop, reg_data;
    int jmp_table_low = 82;
    int jmp_table_hi = 1597;
    int channel_num_arr[144];
    char board_id = 0xFF;
    int which_port, ix; 

    board_id = get_oak_id();
    if (build_channel_num_arr(&channel_num_arr[0],max_fxs_ports) == FAILED) {
        return(FAILED);
    }

    bsp_debug_printf("\n\r Load Patch from %d ~ %d\r",0, max_fxs_ports);

    /* disable Patch RAM*/
    si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports, JMPEN, 0);

    /*
     * make sure JMPEN = 0
     */
    if (is_phoenix()) {
        for (ix = 0; ix < max_fxs_ports; ix++) {
            /* Skip port if some MB/DBx not test */
            if (phoenix_separ_dbx_port_translate(&ix, max_fxs_ports, board_id) == TRUE){
                continue;
            }
            
            if (ix%2 == 0) {
                which_port = ix;
                do {
                    si_read_reg_32261(which_port, JMPEN, &reg_data);
                    if ((reg_data & 0xff) != 0 ) {
                        si_write_reg_32261(which_port, JMPEN, 0);
                    }
                } while ((reg_data & 0xff) == 1);
            }
        }
    } else {
        for (ix = 0; ix < max_fxs_ports; ix++) {
            if (ix%2 == 0) {
                which_port = ix;
                do {
                    si_read_reg_32261(which_port, JMPEN, &reg_data);
                    if ((reg_data & 0xff) != 0 ) {
                        si_write_reg_32261(which_port, JMPEN, 0);
                    }
                } while ((reg_data & 0xff) == 1);
            }
        }
    }

    /* Zero out jump table in case previous values are still loaded */
    for (loop = 0; loop < 8; loop++) {
        /* Zero out jump table */
        si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports, jmp_table_low, 0);
        si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports, jmp_table_low + 1, 0);
        jmp_table_low+=2;
    }

    for (loop = 0; loop < 8; loop++) {
        /* Zero out jump table */
        si_write_ram_32261_calibr(&channel_num_arr[0], max_fxs_ports,jmp_table_hi, 0, 0);
        jmp_table_hi++;
    }

    if (si_write_ram_32261_calibr(&channel_num_arr[0], max_fxs_ports,SI3226x_PRAM_ADDR, 0, 0)) {
        return (FAILED);
    }

    /*
     * If the data is all 0, you have hit the end of the programmed
     * values and can stop loading.
     */

    for (loop = 0; loop < 1024; loop++) {
        temp_hi = (((patchData[loop] << 9) & 0xffff0000) >> 16);
        temp_lo = ((patchData[loop] << 9) & 0xffff);
        if ((temp_hi == 0) && (temp_lo == 0)) {
            loop = 1024;
        } else {
            si_write_ram_32261_calibr(&channel_num_arr[0], max_fxs_ports,
                        SI3226x_PRAM_DATA,temp_hi, temp_lo);
        }
    }

    si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports, RAM_ADDR_HI, 0);

    /* Load the jump table with the passed values. */
    jmp_table_low = 82;
    for (loop = 0; loop < 8; loop++) {
        if (patchEntries[loop] != 0) {
            si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports,
                                      jmp_table_low, patchEntries[loop]&0xff);
            si_write_reg_32261_calibr(&channel_num_arr[0], max_fxs_ports,
                                      jmp_table_low+1, patchEntries[loop]>>8);
        }
                jmp_table_low+=2;
    }

    jmp_table_hi = 1597;
    for (loop = 0; loop < 8; loop++) {
        if (patchEntries[loop + 8] != 0) {
            si_write_ram_32261_calibr(&channel_num_arr[0], max_fxs_ports,
                      jmp_table_hi, 0,(patchEntries[loop + 8]&0x1fff));
        }
                jmp_table_hi++;
    }

    /**
     * Write patch support RAM locations (if any) 
     */
    for (loop = 0; loop < 128; loop++) {
        if (patchSupportAddr[loop] != 0) {
            si_write_ram_32261_calibr(&channel_num_arr[0], max_fxs_ports,
                      patchSupportAddr[loop],((patchSupportData[loop]&0x1fff0000) >> 16),
                      (patchSupportData[loop]&0xffff));
        } else {
            loop = 128;
        }
    }
    if (is_phoenix()) {
        for (ix = 0; ix < max_fxs_ports; ix++) {
            /* Skip port if some MB/DBx not test */
            if (phoenix_separ_dbx_port_translate(&ix, max_fxs_ports, board_id) == TRUE){
                continue;
            }
            
            if (ix%2 == 0) {
                which_port = ix;
                si_write_reg_32261(which_port, JMPEN, 1); //enable the patch
                si_read_reg_32261(which_port, JMPEN, &reg_data);
                if (((reg_data) &0xff) == 0 ) {
                    si_write_reg_32261(which_port, JMPEN, 1);
                }
            }
        }
    } else {
        /*leave the JMPEN 0 disable the jmp table */
        for (ix = 0; ix < max_fxs_ports; ix++) {
            if (ix%2 == 0) {
                which_port = ix;
                si_write_reg_32261(which_port, JMPEN, 1); //enable the patch
                si_read_reg_32261(which_port, JMPEN, &reg_data);
                if (((reg_data) &0xff) == 0 ) {
                    si_write_reg_32261(which_port, JMPEN, 1);
                }
            }
        }
    }
    return (PASSED);
}


/*
** Function: Si3226x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
*/
int Si3226x_PowerUpConverter (int which_port)
{
    int vbath,vbat;
    ramData data;
    int timer = 0;
    uint16_t data_hi, data_lo;
    unsigned int reg_data;
    char errstr[128];

    bsp_debug_printf("\n\r Si3226x_PowerUpConverter \r");

    /*
    ** Check to see if already powered, return if so
    */
    si_read_ram_32261(which_port, PD_DCDC, &data_hi, &data_lo);
    data = (((data_hi)<<16) | (data_lo));
    if(!(data & 0x100000)) {
        cterr('f', 0,  "Si3226x_PowerUpConverter-> check PD_DCDC faill, %x\n", data);
        sprintf(errstr, "\ncheck PD_DCDC faill\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return FAILED;   /* Return if already powered up */
    }

    /*
    ** Power up sequence 
    */
    if (Si3226x_General_Configuration.batType == BO_DCDC_TRACKING) {
        /*
        ** TRACKING CONVERTER SEQUENCE
        **
        ** - clear previous ov/uv lockout
        ** - powerup charge pump
        ** - delay
        ** - powerup digital dc/dc w/ OV clamping and shutdown
        ** - delay
        ** - verify no short circuits by looking for vbath/2
        ** - clear dcdc status
        ** - switch to analog converter with OV clamping only (no shutdown)
        ** - select analog dcdc and disable pwrsave
        ** - delay
        */

        si_write_ram_32261_wrap(which_port, DCDC_OITHRESH,Si3226x_General_Configuration.dcdc_oithresh_lo);
        si_write_reg_32261(which_port, LINEFEED,LF_OPEN);  /* Ensure open before powering converter */
        si_read_reg_32261(which_port, ENHANCE, &reg_data);      /* Read ENHANCE entry settings */
        si_write_reg_32261(which_port, ENHANCE,reg_data&0x07);  /* Disable powersave during bringup */
        si_write_ram_32261_wrap(which_port, PD_DCDC,0x700000L);   /* In case OV or UV previously occurred */
        /* Interim support for higher voltage PBB that uses gate drive circuit */
        si_write_ram_32261_wrap(which_port,DCDC_CPUMP,0x100000L);/* Turn on charge pump */
        msleep(100);
        si_write_ram_32261_wrap(which_port, PD_DCDC,0x600000L);
        msleep(100);
        si_read_ram_32261(which_port, VBATH_EXPECT, &data_hi, &data_lo);
        vbath = (((data_hi)<<16) | (data_lo));
        si_read_ram_32261(which_port, MADC_VBAT, &data_hi, &data_lo);
        vbat = (((data_hi)<<16) | (data_lo));
        if (vbat & 0x10000000L)
            vbat |= 0xF0000000L;

        if (vbat < (vbath / 2)) {
            bsp_debug_printf("Port: %d, Vbat = 0x%x\n",which_port, vbat);
            cterr('f', 0,  "Si3226x_PowerUpConverter-> check vbat/vbath fail\n");
            si_write_ram_32261_wrap(which_port, PD_DCDC, 0x300000L); /* shutdown converter */
            return FAILED;
        } else { /* Enable analog converter */
            si_write_ram_32261_wrap(which_port, DCDC_STATUS,0L);
            si_write_ram_32261_wrap(which_port, PD_DCDC,0x400000L);
            si_write_reg_32261(which_port, ENHANCE, reg_data);   /* restore ENHANCE setting */
            msleep(100);
        }

        /*
        ** - monitor vbat vs expected level (VBATH_EXPECT)
        */
        si_read_ram_32261(which_port, VBATH_EXPECT, &data_hi, &data_lo);
        vbath = (((data_hi)<<16) | (data_lo));
        timer = 0;
        do {
            si_read_ram_32261(which_port, MADC_VBAT, &data_hi, &data_lo);
            vbat = (((data_hi)<<16) | (data_lo));
            if(vbat & 0x10000000L) {
                vbat |= 0xF0000000L;
            }
            msleep(10);
        } while ((vbat < (vbath - COMP_5V))&&(timer++ < 2000));


        if (timer > 2000) {
            /* Error handling - shutdown converter, disable channel, set error tag */
            cterr('f', 0,  "Si3226x_PowerUpConverter-> 1. check vbat/vbath fail\n");
            sprintf(errstr, "\ncheck vbat/vbath fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            si_write_ram_32261_wrap(which_port, PD_DCDC, 0x300000L); /* shutdown converter */
            return (FAILED);
        }

        return PASSED;
    } else if ((Si3226x_General_Configuration.batType == BO_DCDC_TSS)||
               (Si3226x_General_Configuration.batType == BO_DCDC_TSS_ISO)||
                Si3226x_General_Configuration.batType == BO_DCDC_QSS) {
        /*
        ** FIXED RAIL CONVERTER SEQUENCE
        **
        ** - return if even channel
        ** - clear previous ov/uv lockout
        ** - powerup charge pump
        ** - delay
        ** - powerup converter
        ** - delay
        ** - verify no short circuits by looking for vbath/2
        ** - clear dcdc status
        ** - delay
        */

        if (which_port % 2 == 0) {  /* is even */
            return (PASSED);
        }

        si_write_reg_32261(which_port, LINEFEED,LF_OPEN);  /* Ensure open before powering converter */
        si_read_reg_32261(which_port,ENHANCE, &reg_data);      /* Read ENHANCE entry settings */
        si_write_reg_32261(which_port, ENHANCE,reg_data&0x07);  /* Disable powersave during bringup */
        si_write_ram_32261_wrap(which_port, PD_DCDC,0x700000L);   /* In case OV or UV previously occurred */
        si_write_ram_32261_wrap(which_port, DCDC_CPUMP,0x100000L);/* Turn on charge pump */
        msleep(100);
        si_write_ram_32261_wrap(which_port, PD_DCDC,0x600000L);  /* Start Converter */
        msleep(1000);

#ifdef TSS_IOS
        /* If isolated design, turn off charge pump and powerdown OCLO */
        if(Si3226x_General_Configuration.batType == BO_DCDC_TSS_ISO) {
            si_write_ram_32261_wrap(which_port, DCDC_CPUMP,0x0L);
            msleep(100);
            si_write_ram_32261_wrap(which_port, PD_OCLO,0x300000L);
            msleep(100);
        }
#endif

        si_read_ram_32261(which_port, VBATH_EXPECT, &data_hi, &data_lo);
        vbath = (((data_hi)<<16) | (data_lo));
        si_read_ram_32261(which_port, MADC_VBAT, &data_hi, &data_lo);
        vbat = (((data_hi)<<16) | (data_lo));
        if(vbat & 0x10000000L) {
            vbat |= 0xF0000000L;
        }
        if(vbath & 0x10000000L) {
            vbath |= 0xF0000000L;
        }

        if (vbat < (vbath / 2)) {
            cterr('f', 0,  "2. check vbat/vbath fail. vbat: %x, vbath: %x\n", vbat, vbath);
            si_write_ram_32261_wrap(which_port, PD_DCDC, 0x300000L); /* shutdown converter */
            msleep(100);
            si_write_ram_32261_wrap(which_port,DCDC_CPUMP,0x0L); /* shut off charge pump */

            return (FAILED);
        } else {
            si_write_ram_32261_wrap(which_port,DCDC_STATUS,0L);
            msleep(100);
        }

        /*
        ** - monitor vbat vs expected level (VBATH_EXPECT)
        */
        si_read_ram_32261(which_port, VBATH_EXPECT, &data_hi, &data_lo);
        vbath = (((data_hi)<<16) | (data_lo));
        do {
            si_read_ram_32261(which_port, MADC_VBAT, &data_hi, &data_lo);
            vbat = (((data_hi)<<16) | (data_lo));
            if(vbat & 0x10000000L)
                vbat |= 0xF0000000L;
            msleep(100);
        } while ((vbat < (vbath - COMP_5V))&&(timer++ < 200));  /* 2 sec timeout */

        if (timer > 200) {
            /* Error handling - shutdown converter, disable channel, set error tag */
            cterr('f', 0, "Si3226x_PowerUpConverter-> 2. vbath ecpext fail\n");
            sprintf(errstr, "\n2.vbath ecpext fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            si_write_ram_32261_wrap(which_port, PD_DCDC, 0x300000L); /* shutdown converter */
            return (FAILED);

        }

        /* Restore ENHANCE reg */
        si_write_reg_32261(which_port, ENHANCE,reg_data);

        return (PASSED);;
    } else {/* external battery - just verify presence */
        /*
        ** - monitor vbat vs expected level (VBATH_EXPECT)
        */
        si_read_ram_32261(which_port, VBATH_EXPECT, &data_hi, &data_lo);
        vbath = (((data_hi)<<16) | (data_lo));
        do {
            si_read_ram_32261(which_port,MADC_VBAT, &data_hi, &data_lo);
            vbat = (((data_hi)<<16) | (data_lo));
            if(vbat & 0x10000000L)
                vbat |= 0xF0000000L;
            msleep(100);
        } while ((vbat < (vbath - COMP_5V))&&(timer++ < 200));  /* 2 sec timeout */

        if (timer > 200) {
            cterr('f', 0, "Si3226x_PowerUpConverter-> 3. check vbat/vbath fail\n");
            sprintf(errstr, "\n3.check vbat/vbath fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            /* Error handling - shutdown converter, disable channel, set error tag */
           return (FAILED);
        }

    }
    return (PASSED);
}



/*
** Function: Si3226x_Init_with_Options
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate remaining items except madc & lb
**
** Input Parameters: which port
** fault: error code
**
** Return:
** error code
*/

int si3226x_init_with_options (int port) 
{
    /*
    ** This function will initialize the chipRev and chipType members in pProslic
    ** as well as load the initialization structures.
    */
    unsigned int pdn_tmp, reg_32261;
    int count, ix, which_port;
    uint16_t data, data_lo;
    int codec_channel = 2;
    char errstr[128];


    for (ix = 0; ix < codec_channel; ix++) {

        which_port = port + ix;
        if (si32261_protected_mode(which_port)==FAILED) {
            cterr('f', 0,  "si3226x_init_with_options->Set protect fail\n");
            sprintf(errstr, "\nprotect fail\\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            return(FAILED);
            msleep(10);
        }

        /*
        ** Load general parameters - includes all BOM dependencies
        **
        ** First qualify general parameters by identifying valid device key.  This
        ** will prevent inadvertent use of other device's preset files, which could
        ** lead to improper initialization and high current states.
        */

        /* Force pwrsave off and disable AUTO-tracking - set to user configured state after cal */
        si_write_reg_32261(which_port, ENHANCE, 0);
        si_write_reg_32261(which_port, AUTO, 0x2F);

        si_write_ram_32261_wrap(which_port, BAT_HYST,Si3226x_General_Configuration.bat_hyst);
        si_write_ram_32261_wrap(which_port,VBATR_EXPECT,Si3226x_General_Configuration.vbatr_expect);
        si_write_ram_32261_wrap(which_port,VBATH_EXPECT,Si3226x_General_Configuration.vbath_expect);
        si_write_ram_32261_wrap(which_port,PWRSAVE_TIMER,Si3226x_General_Configuration.pwrsave_timer);
        si_write_ram_32261_wrap(which_port,OFFHOOK_THRESH,Si3226x_General_Configuration.offhook_thresh);
        si_write_ram_32261_wrap(which_port,VBAT_TRACK_MIN,Si3226x_General_Configuration.vbat_track_min);
        si_write_ram_32261_wrap(which_port,VBAT_TRACK_MIN_RNG,Si3226x_General_Configuration.vbat_track_min_rng);

        si_write_ram_32261_wrap(which_port, THERM_DBI,Si3226x_General_Configuration.therm_dbi);

        si_write_ram_32261_wrap(which_port, DCDC_VERR,Si3226x_General_Configuration.dcdc_verr);
        si_write_ram_32261_wrap(which_port, DCDC_VERR_HYST,Si3226x_General_Configuration.dcdc_verr_hyst);
        si_write_ram_32261_wrap(which_port, DCDC_OITHRESH_LO,Si3226x_General_Configuration.dcdc_oithresh_lo);
        si_write_ram_32261_wrap(which_port, DCDC_OITHRESH_HI,Si3226x_General_Configuration.dcdc_oithresh_hi);
        si_write_ram_32261_wrap(which_port, PD_UVLO,Si3226x_General_Configuration.pd_uvlo);
        si_write_ram_32261_wrap(which_port, PD_OVLO,Si3226x_General_Configuration.pd_ovlo);
        si_write_ram_32261_wrap(which_port, PD_OCLO,Si3226x_General_Configuration.pd_oclo);

        si_write_ram_32261_wrap(which_port, DCDC_UVHYST,Si3226x_General_Configuration.dcdc_uvhyst);
        si_write_ram_32261_wrap(which_port, DCDC_UVTHRESH,Si3226x_General_Configuration.dcdc_uvthresh);
        si_write_ram_32261_wrap(which_port, DCDC_OVTHRESH,Si3226x_General_Configuration.dcdc_ovthresh);

        si_write_ram_32261_wrap(which_port, DCDC_UVPOL,Si3226x_General_Configuration.dcdc_uvpol);

        si_write_ram_32261_wrap(which_port, DCDC_VREF_CTRL,Si3226x_General_Configuration.dcdc_vref_ctrl);
        si_write_ram_32261_wrap(which_port, DCDC_RNGTYPE,Si3226x_General_Configuration.dcdc_rngtype);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_GAIN,Si3226x_General_Configuration.dcdc_ana_gain);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_TOFF,Si3226x_General_Configuration.dcdc_ana_toff);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_TONMIN,Si3226x_General_Configuration.dcdc_ana_tonmin);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_TONMAX,Si3226x_General_Configuration.dcdc_ana_tonmax);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_DSHIFT,Si3226x_General_Configuration.dcdc_ana_dshift);
        si_write_ram_32261_wrap(which_port, DCDC_ANA_LPOLY,Si3226x_General_Configuration.dcdc_ana_lpoly);

        si_write_ram_32261_wrap(which_port, COEF_P_HVIC,Si3226x_General_Configuration.coef_p_hvic);
        si_write_ram_32261_wrap(which_port, P_TH_HVIC,Si3226x_General_Configuration.p_th_hvic);

        si_write_ram_32261_wrap(which_port,SCALE_KAUDIO,Si3226x_General_Configuration.scale_kaudio);

        /* GC RAM locations that moved from RevB to RevC */
        si_write_ram_32261_wrap(which_port, LKG_OFHK_OFFSET,Si3226x_General_Configuration.lkg_ofhk_offset);
        si_write_ram_32261_wrap(which_port, LKG_LB_OFFSET,Si3226x_General_Configuration.lkg_lb_offset);
        si_write_ram_32261_wrap(which_port, VBATH_DELTA,Si3226x_General_Configuration.vbath_delta);
        si_write_ram_32261_wrap(which_port, UVTHRESH_MAX,Si3226x_General_Configuration.uvthresh_max);
        si_write_ram_32261_wrap(which_port, UVTHRESH_SCALE,Si3226x_General_Configuration.uvthresh_scale);
        si_write_ram_32261_wrap(which_port, UVTHRESH_BIAS,Si3226x_General_Configuration.uvthresh_bias);

        /* Hardcoded mods to default settings */
        si_read_reg_32261(which_port, GPIO_CFG1, &reg_32261);
        reg_32261 &= 0xF9;  /* Clear DIR for GPIO 1&2 */
        reg_32261 |= 0x60;  /* Set ANA mode for GPIO 1&2 */
        si_write_reg_32261(which_port, GPIO_CFG1, reg_32261); /* coarse sensors analog mode */
        si_write_reg_32261(which_port, PDN, 0x80); /* madc powered in open state */
        si_write_ram_32261_wrap(which_port,TXACHPF_A1_1,0x71EB851L); /* Fix HPF corner */
        si_write_ram_32261_wrap(which_port,ROW0_C2, 0x723F235L);   /* improved DTMF det */
        si_write_ram_32261_wrap(which_port,ROW1_C2, 0x57A9804L);   /* improved DTMF det */
        si_write_ram_32261_wrap(which_port,XTALK_TIMER,0x36000L); /* xtalk fix */
        si_write_ram_32261_wrap(which_port,DCDC_CPUMP_LP_MASK,0x1100000L); /* Charge pump mask */
        /* Smart VOV Default Settings - set here in case no ring preset is loaded */
        si_write_ram_32261_wrap(which_port,VOV_DCDC_SLOPE,0xFFFFFFL); /* dcdc overhead scale */
        si_write_ram_32261_wrap(which_port,VOV_DCDC_OS,0xA18937L); /* smart vov overhead offset*/
        si_write_ram_32261_wrap(which_port,VOV_RING_BAT_MAX,0xE49BA5L); /* smart vov max vov */

        si_write_ram_32261_wrap(which_port,VDIFFLPF,0x10038DL); /* vloop lpf 10hz */
        si_write_ram_32261_wrap(which_port,ILOOPLPF,0x4EDDB9L); /* iloop lpf*/
        si_write_ram_32261_wrap(which_port,ILONGLPF,0x806D6L); /* ilong lpf */
        si_write_ram_32261_wrap(which_port,VCMLPF,0x10059FL); /* 20pps pulse dialing */
        si_write_ram_32261_wrap(which_port,CM_SPEEDUP_TIMER,0xF0000L); /* 20pps pulse dialing */
        si_write_ram_32261_wrap(which_port,VCM_TH,0x106240L); /* 20pps pulse dialing */

        /* Prevent Ref Osc from powering down in PLL Freerun mode (pd_ref_osc) */
        si_read_ram_32261(which_port,PWRSAVE_CTRL_LO, &data, &data_lo);
        pdn_tmp = ((data<<16) | data_lo);
        si_write_ram_32261_wrap(which_port,PWRSAVE_CTRL_LO,pdn_tmp&0x0BFFFFFFL);/* clear b26 */

        /* Hardcoded mods for Tracking supplies */
        if (Si3226x_General_Configuration.batType == BO_DCDC_TRACKING) {
            si_write_ram_32261_wrap(which_port,DCDC_UV_DEBOUNCE, 0x200000L);
            si_write_ram_32261_wrap(which_port,DCDC_OV_DEBOUNCE, 0x0L);
            si_write_ram_32261_wrap(which_port,DCDC_OIMASK, 0xC00000L);
            si_write_ram_32261_wrap(which_port,VCM_HYST,0x206280L); /* 2v */

            if (Si3226x_General_Configuration.bomOpt == BO_DCDC_BUCK_BOOST) {
                si_write_ram_32261_wrap(which_port,DCDC_DCFF_ENABLE,0x10000000L);/* enable dcff drive */
                si_write_ram_32261_wrap(which_port,LPR_SCALE,0x2A00000L);/* scale for LPR amplitude */
                si_write_ram_32261_wrap(which_port,LPR_CM_OS,0x61EB80L); /* LPR cm offset */
                si_write_ram_32261_wrap(which_port,VBAT_IRQ_TH,0x51EB80L); /* thresh to 5v */
            } else {
                si_write_ram_32261_wrap(which_port,DCDC_DCFF_ENABLE,0x0L);/* disable dcff drive */
                si_write_ram_32261_wrap(which_port,LPR_SCALE,0x1F00000L);/* scale for LPR amplitude */
                si_write_ram_32261_wrap(which_port,LPR_CM_OS,0x51EB80L); /* LPR cm offset */
            }
        } else if (Si3226x_General_Configuration.batType == BO_DCDC_QSS) {
            si_write_ram_32261_wrap(which_port,VCM_HYST,0x306280L); /* 3v */
            si_write_ram_32261_wrap(which_port,LPR_SCALE,0x2A00000L);/* scale for LPR amplitude */
            si_write_ram_32261_wrap(which_port,LPR_CM_OS,0x61EB80L); /* LPR cm offset */
            si_write_ram_32261_wrap(which_port,DCDC_OIMASK, 0xA00000L);
            si_write_ram_32261_wrap(which_port,DCDC_UV_DEBOUNCE, 0x0L);
            si_write_ram_32261_wrap(which_port,DCDC_OV_DEBOUNCE, 0xD00000L);
            si_write_ram_32261_wrap(which_port,MADC_VDC_SCALE, 0xAE924B9L);
            si_write_ram_32261_wrap(which_port,VBATL_EXPECT, 0xF00000L); /* force vbatl 13v to keep cm recalc */
            si_write_ram_32261_wrap(which_port,PD_OFFLD_DAC,0x200000L);
            si_write_ram_32261_wrap(which_port,PD_OFFLD_GM,0x200000L);
            si_write_ram_32261_wrap(which_port,DCDC_PD_ANA, 0x300000);
            si_write_ram_32261_wrap(which_port,P_TH_OFFLOAD, 0x480CBFL); /* Large - not used in QSS */
            si_write_reg_32261(which_port, OFFLOAD,0x3); /* Enable offload and vbat_l */

         }  else { /* TSS, TSS_ISO, or EXTERNAL */
             si_write_ram_32261_wrap(which_port,DCDC_UV_DEBOUNCE, 0x0L);
             si_write_ram_32261_wrap(which_port,DCDC_OV_DEBOUNCE, 0xD00000L);
             si_write_ram_32261_wrap(which_port,DCDC_OIMASK, 0xA00000L);
             si_write_ram_32261_wrap(which_port,VCM_HYST,0x306280L); /* 3v */
             si_write_ram_32261_wrap(which_port,LPR_SCALE,0x2A00000L);/* scale for LPR amplitude */
             si_write_ram_32261_wrap(which_port,LPR_CM_OS,0x61EB80L); /* LPR cm offset */

             si_write_ram_32261_wrap(which_port,VBATL_EXPECT, 0xF00000L); /* force vbatl 13v to keep cm recalc */   
             si_write_ram_32261_wrap(which_port,MADC_VDC_SCALE, 0xAE924B9L);
             si_write_ram_32261_wrap(which_port,DCDC_PD_ANA, 0x300000);
             si_write_ram_32261_wrap(which_port,P_TH_OFFLOAD, 0x280CBFL); /* 1.1W @ 60C */
             si_write_ram_32261_wrap(which_port,PD_OFFLD_DAC,0x200000L);
             si_write_ram_32261_wrap(which_port,PD_OFFLD_GM,0x200000L);
             si_write_ram_32261_wrap(which_port,DCDC_CPUMP_LP_MASK,0x100000L); /* Charge pump mask */

             /* Setup power offloading for tracking switched supplies */
             if ((Si3226x_General_Configuration.batType == BO_DCDC_TSS) ||
                 (Si3226x_General_Configuration.batType == BO_DCDC_TSS_ISO)) {

                 si_write_reg_32261(which_port,OFFLOAD,0x3); /* Enable offload and vbat_l */
             } else {
                 si_write_reg_32261(which_port,OFFLOAD,0x13); /* Enable offload and vbat_l,
                                                                 disable fixed rail battery management. */
             }
         }


          /* DCDC Drive Polarity */
          si_write_ram_32261_wrap(which_port, DCDC_SWDRV_POL ,Si3226x_General_Configuration.dcdc_swdrv_pol);


         /*
          ** Calibrate (madc offset)
         */
         si_write_reg_32261(which_port, CALR0, 0);
         si_write_reg_32261(which_port, CALR1, 0x0);
         si_write_reg_32261(which_port, CALR2, 0x01);
         si_write_reg_32261(which_port, CALR3, 0x80);

         /* around 1 second */
         count = 20;
         do {
             si_read_reg_32261(which_port, CALR3, &reg_32261);
             msleep(10);
             count--;
         } while ((reg_32261 & 0x80) && (count > 0));

         if (count <= 0) {
             cterr('f', 0,  "Calibrate (madc offset) fail\n");
             sprintf(errstr, "\nmadc offset fail\n");
             strcat((char *)&(hd_if->errmsg), errstr);
             return (FAILED);
         }

    }


    for (ix = 0; ix < codec_channel; ix++) {

        which_port = port + ix;

        /*
        ** Bring up DC/DC converters sequentially to minimize
        ** peak power demand on VDC
        */

        if (Si3226x_PowerUpConverter(which_port) == FAILED) {
            cterr('f', 0,  "Power up fail\n");
            sprintf(errstr, "\nPwr up fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            return(FAILED);
        }

        msleep(10);
    }

    for (ix = 0; ix < codec_channel; ix++) {

        which_port = port + ix;

        /*
        ** Calibrate remaining cals (except madc, lb)
        */
        si_write_reg_32261(which_port, CALR0, 0);
        si_write_reg_32261(which_port, CALR1, 0xC0);
        si_write_reg_32261(which_port, CALR2, 0x18);
        si_write_reg_32261(which_port, CALR3, 0x80);

        /* around 1 second */
        count = 600;
        do {
            si_read_reg_32261(which_port, CALR3, &reg_32261);
            msleep(10);
            count--;
        } while ((reg_32261) & (count > 0));

        if (count <= 0) {
            cterr('f', 0, "Calibrate remaining cals fail\n");
            sprintf(errstr, "\ncals fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    }


    for (ix = 0; ix < codec_channel; ix++) {

        which_port = port + ix;

        si_write_reg_32261(which_port, ENHANCE,Si3226x_General_Configuration.enhance);
        si_write_reg_32261(which_port, AUTO,Si3226x_General_Configuration.autoRegister);
        si_write_reg_32261(which_port, ZCAL_EN,Si3226x_General_Configuration.zcal_en);
    }
    return(PASSED);
}



/**************************************************************
** Function: Si3226x_LBCal
**
** Description: 
** Run canned longitudinal balance calibration.  Each channel
** may be calibrated in parallel since there are no shared
** resources between si3226x devices.
**
** Input Parameters: which_port
**
** Return: 0
**************************************************************
*/
int Si3226x_LBCal (int which_port)
{
    unsigned char timeout = 0;
    unsigned int count = 600, reg_enhance, reg_linefeed, reg_data;

    si_read_reg_32261(which_port, ENHANCE, &reg_enhance);
    si_read_reg_32261(which_port, LINEFEED, &reg_linefeed);

    /* Disable powersave mode and set linefeed to fwd active */
    reg_enhance &= 0x07;
    si_write_reg_32261(which_port, ENHANCE, reg_enhance);
    si_write_reg_32261(which_port, LINEFEED, LF_FWD_ACTIVE);

    si_write_reg_32261(which_port, CALR0, CAL_LB_ALL); /* enable LB cal */
    si_write_reg_32261(which_port, CALR3, 0x80); /* start cal */

    si_read_reg_32261(which_port, CALR3, &reg_data);

    while (((reg_data) & 0x80) && (count > 0)) {
        msleep(10);
        si_read_reg_32261(which_port, CALR3, &reg_data);
        count--;
    }

    if (count <= 0) {
        si_write_reg_32261(which_port, LINEFEED, LF_OPEN);
        timeout = 1;
    } else {
        si_write_reg_32261(which_port, LINEFEED, reg_linefeed);
    }

    si_write_reg_32261(which_port, ENHANCE, reg_enhance);

    if (timeout != 0) {
        return (FAILED);
    } else {
        return (PASSED);
    }

}


/*
** Function: PROSLIC_DCFeedSetupCfg
**
** Description: 
** configure dc feed
*/
int Si3226x_DCFeedSetupCfg (int port) 
{
    unsigned char preset = 0;
    unsigned int reg_data;

    si_read_reg_32261(port, LINEFEED, &reg_data);
    si_write_reg_32261(port,LINEFEED,0);
    si_write_ram_32261_wrap(port,SLOPE_VLIM,Si3226x_DCfeed_Presets[preset].slope_vlim);
    si_write_ram_32261_wrap(port,SLOPE_RFEED,Si3226x_DCfeed_Presets[preset].slope_rfeed);
    si_write_ram_32261_wrap(port,SLOPE_ILIM,Si3226x_DCfeed_Presets[preset].slope_ilim);
    si_write_ram_32261_wrap(port,SLOPE_DELTA1,Si3226x_DCfeed_Presets[preset].delta1);
    si_write_ram_32261_wrap(port,SLOPE_DELTA2,Si3226x_DCfeed_Presets[preset].delta2);
    si_write_ram_32261_wrap(port,V_VLIM,Si3226x_DCfeed_Presets[preset].v_vlim);
    si_write_ram_32261_wrap(port,V_RFEED,Si3226x_DCfeed_Presets[preset].v_rfeed);
    si_write_ram_32261_wrap(port,V_ILIM,Si3226x_DCfeed_Presets[preset].v_ilim);
    si_write_ram_32261_wrap(port,CONST_RFEED,Si3226x_DCfeed_Presets[preset].const_rfeed);
    si_write_ram_32261_wrap(port,CONST_ILIM,Si3226x_DCfeed_Presets[preset].const_ilim);
    si_write_ram_32261_wrap(port,I_VLIM,Si3226x_DCfeed_Presets[preset].i_vlim);
    si_write_ram_32261_wrap(port,LCRONHK,Si3226x_DCfeed_Presets[preset].lcronhk);
    si_write_ram_32261_wrap(port,LCROFFHK,Si3226x_DCfeed_Presets[preset].lcroffhk);
    si_write_ram_32261_wrap(port,LCRDBI,Si3226x_DCfeed_Presets[preset].lcrdbi);
    si_write_ram_32261_wrap(port,LONGHITH,Si3226x_DCfeed_Presets[preset].longhith);
    si_write_ram_32261_wrap(port,LONGLOTH,Si3226x_DCfeed_Presets[preset].longloth);
    si_write_ram_32261_wrap(port,LONGDBI,Si3226x_DCfeed_Presets[preset].longdbi);
    si_write_ram_32261_wrap(port,LCRMASK,Si3226x_DCfeed_Presets[preset].lcrmask);
    si_write_ram_32261_wrap(port,LCRMASK_POLREV,Si3226x_DCfeed_Presets[preset].lcrmask_polrev);
    si_write_ram_32261_wrap(port,LCRMASK_STATE,Si3226x_DCfeed_Presets[preset].lcrmask_state);
    si_write_ram_32261_wrap(port,LCRMASK_LINECAP,Si3226x_DCfeed_Presets[preset].lcrmask_linecap);
    si_write_ram_32261_wrap(port,VCM_OH,Si3226x_DCfeed_Presets[preset].vcm_oh);
    si_write_ram_32261_wrap(port,VOV_BAT,Si3226x_DCfeed_Presets[preset].vov_bat);
    si_write_ram_32261_wrap(port,VOV_GND,Si3226x_DCfeed_Presets[preset].vov_gnd);

    si_write_reg_32261(port,LINEFEED,reg_data);

    return (PASSED);
}

/*
** Function: Si3226x_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     ring preset
**
** Returns:
** 0
*/

int Si3226x_RingSetup (int port)
{

    int preset = 0;

    si_write_ram_32261_wrap(port,RTPER,Si3226x_Ring_Presets[preset].rtper);
    si_write_ram_32261_wrap(port,RINGFR,Si3226x_Ring_Presets[preset].freq);
    si_write_ram_32261_wrap(port,RINGAMP,Si3226x_Ring_Presets[preset].amp);
    si_write_ram_32261_wrap(port,RINGPHAS,Si3226x_Ring_Presets[preset].phas);
    si_write_ram_32261_wrap(port,RINGOF,Si3226x_Ring_Presets[preset].offset);
    si_write_ram_32261_wrap(port,SLOPE_RING,Si3226x_Ring_Presets[preset].slope_ring);
    si_write_ram_32261_wrap(port,IRING_LIM,Si3226x_Ring_Presets[preset].iring_lim);
    si_write_ram_32261_wrap(port,RTACTH,Si3226x_Ring_Presets[preset].rtacth);
    si_write_ram_32261_wrap(port,RTDCTH,Si3226x_Ring_Presets[preset].rtdcth);
    si_write_ram_32261_wrap(port,RTACDB,Si3226x_Ring_Presets[preset].rtacdb);
    si_write_ram_32261_wrap(port,RTDCDB,Si3226x_Ring_Presets[preset].rtdcdb);
    si_write_ram_32261_wrap(port,VOV_RING_BAT,Si3226x_Ring_Presets[preset].vov_ring_bat);
    si_write_ram_32261_wrap(port,VOV_RING_GND,Si3226x_Ring_Presets[preset].vov_ring_gnd);

    /* Always limit VBATR_EXPECT to the general configuration maximum */
#ifndef NOCLAMP_VBATR
    if (Si3226x_Ring_Presets[preset].vbatr_expect > Si3226x_General_Configuration.vbatr_expect) {
        si_write_ram_32261_wrap(port,VBATR_EXPECT,Si3226x_General_Configuration.vbatr_expect);
    } else {
        si_write_ram_32261_wrap(port,VBATR_EXPECT,Si3226x_Ring_Presets[preset].vbatr_expect);
    }
#else
        si_write_ram_32261_wrap(port,VBATR_EXPECT,Si3226x_Ring_Presets[preset].vbatr_expect);
#endif

    si_write_reg_32261(port,RINGTALO,Si3226x_Ring_Presets[preset].talo);
    si_write_reg_32261(port,RINGTAHI,Si3226x_Ring_Presets[preset].tahi);
    si_write_reg_32261(port,RINGTILO,Si3226x_Ring_Presets[preset].tilo);
    si_write_reg_32261(port,RINGTIHI,Si3226x_Ring_Presets[preset].tihi);

    si_write_ram_32261_wrap(port,DCDC_VREF_MIN_RNG,Si3226x_Ring_Presets[preset].vbat_track_min_rng);
    si_write_reg_32261(port,RINGCON,Si3226x_Ring_Presets[preset].ringcon);
    si_write_reg_32261(port,USERSTAT,Si3226x_Ring_Presets[preset].userstat);
    si_write_ram_32261_wrap(port,VCM_RING,Si3226x_Ring_Presets[preset].vcm_ring);
    si_write_ram_32261_wrap(port,VCM_RING_FIXED,Si3226x_Ring_Presets[preset].vcm_ring_fixed);
    si_write_ram_32261_wrap(port,DELTA_VCM,Si3226x_Ring_Presets[preset].delta_vcm);


    /* Clamp IRING_LIM to 60mA max if QSS */
    if (Si3226x_General_Configuration.batType == BO_DCDC_QSS) {
        /*
        ** Parameter Limits
        */
        #define QSS_IRING_LIM_MAX               0x8B9786L   /* 60mA */
        si_write_ram_32261_wrap(port,IRING_LIM,QSS_IRING_LIM_MAX);
    }

    /* Automatically adjust DCDC_RNGTYPE */
    if (Si3226x_General_Configuration.bomOpt == BO_DCDC_BUCK_BOOST) {
        si_write_ram_32261_wrap(port,DCDC_RNGTYPE,0x0L);  /* Fixed */
    } else {
        si_write_ram_32261_wrap(port,DCDC_RNGTYPE,Si3226x_Ring_Presets[preset].dcdc_rngtype);
    }

    return (PASSED);
}

static int Si3226x_dbgSetGain (int gain, int impedance_preset, int tx_rx_sel)
{
    int errVal = 0;
    int i;
    int gain_pga, gain_eq;
    const ProSLIC_GainScaleLookup gainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    { 
        {-30, 32},
        {-29, 35},
        {-28, 40},
        {-27, 45},
        {-26, 50},
        {-25, 56},
        {-24, 63},
        {-23, 71},
        {-22, 79},
        {-21, 89},
        {-20, 100},
        {-19, 112},
        {-18, 126},
        {-17, 141},
        {-16, 158}, 
        {-15, 178}, 
        {-14, 200}, 
        {-13, 224}, 
        {-12, 251}, 
        {-11, 282}, 
        {-10, 316}, 
        {-9, 355}, 
        {-8, 398}, 
        {-7, 447}, 
        {-6, 501},
        {-5, 562},
        {-4, 631},
        {-3, 708},
        {-2, 794},
        {-1, 891},
        {0, 1000},
        {1, 1122},
        {2, 1259},
        {3, 1413},
        {4, 1585},
        {5, 1778},
        {6, 1995},
        {0xff,0}  /* terminator */
    }; 
 
/* 
** 5.4.0 - Removed relative gain scaling. to support automatic adjustment based on
**         gain plan provided in txgain_db and rxgain_db.  It is presumed that all
**         coefficients were generated for 0dB/0dB gain and the txgain_db and rxgain_db
**         parameters will be used to scale the gain using the existing gain provisioning
**         infrastructure when the zsynth preset is loaded.  This function will ignore 
**         the txgain_db and rxgain_db parameters and scale absolute gain presuming a
**         0dB/0dB coefficient set.
*/
/*
** 6.0.0 - Modifying where gain/attenuation is placed to minimize clipping.
**
**         RX Path:   -30dB < gain <  0dB -   All in RXACGAIN
**                      0dB < gain <  6dB -   All in RXACEQ
**
**         TX Path:   -30dB < gain <  0dB -   All in TXACEQ
**                      0dB < gain <  6dB -   All in TXACGAIN
*/
    /* Test against max gain */
    if (gain > EXTENDED_GAIN_MAX) {
		errVal = RC_GAIN_OUT_OF_RANGE;
		gain = EXTENDED_GAIN_MAX; /* Clamp to maximum */
	}

	/* Test against min gain */
    if (gain < GAIN_MIN) {
		errVal = RC_GAIN_OUT_OF_RANGE;
		gain = GAIN_MIN; /* Clamp to minimum */
	}

	/* Distribute gain */
	if (gain == 0) {
		gain_pga = 0;
		gain_eq = 0;
	} else if(gain > GAIN_MAX) {
		if (tx_rx_sel == TXACGAIN_SEL) {
			gain_pga = GAIN_MAX;
			gain_eq = gain - GAIN_MAX;
		} else {
			gain_pga = gain - GAIN_MAX;
			gain_eq = GAIN_MAX;
		}
	} else if (gain > 0) {
		if (tx_rx_sel == TXACGAIN_SEL) {
			gain_pga = gain;
			gain_eq  = 0;
		} else {
			gain_pga = 0;
			gain_eq = gain;
		}
	} else {
		if (tx_rx_sel == TXACGAIN_SEL) {
			gain_pga = 0;
			gain_eq  = gain;
		} else {
			gain_pga = gain;
			gain_eq = 0;
		}

	}

    /* 
	** Lookup PGA Appopriate PGA Gain
	*/
    i=0;
    do {
        if (gainScaleTable[i].gain >= gain_pga) {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if (gainScaleTable[i].gain == 0xff) {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if (tx_rx_sel == TXACGAIN_SEL) {
        Si3226x_audioGain_Presets[0].acgain = 
                (Si3226x_Impedance_Presets[impedance_preset].txgain/1000)*gainScaleTable[i].scale;
    } else {
        Si3226x_audioGain_Presets[1].acgain =
                (Si3226x_Impedance_Presets[impedance_preset].rxgain/1000)*gainScaleTable[i].scale;
    }

    /* 
	** Lookup EQ Gain
	*/
    i = 0;
    do {
        if (gainScaleTable[i].gain >= gain_eq)  {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if (gainScaleTable[i].gain == 0xff) {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if(tx_rx_sel == TXACGAIN_SEL) {
        /*sign extend negative numbers*/
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

        Si3226x_audioGain_Presets[0].aceq_c0 =
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[0].aceq_c1 =
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[0].aceq_c2 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[0].aceq_c3 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000)*gainScaleTable[i].scale;
    } else {
        /*sign extend negative numbers*/
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L) {
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
        }
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L) {
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
        }
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L) {
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
        }
        if (Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L) {
            Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;
        }

        Si3226x_audioGain_Presets[1].aceq_c0 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[1].aceq_c1 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[1].aceq_c2 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*gainScaleTable[i].scale;
        Si3226x_audioGain_Presets[1].aceq_c3 = 
                ((int)Si3226x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*gainScaleTable[i].scale;
    }


    return (errVal);
}


/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3226x_TXAudioGainSetup (int which_port, int preset) 
{
    si_write_ram_32261_wrap(which_port,TXACGAIN,Si3226x_audioGain_Presets[preset].acgain);
    si_write_ram_32261_wrap(which_port,TXACEQ_C0,Si3226x_audioGain_Presets[preset].aceq_c0);
    si_write_ram_32261_wrap(which_port,TXACEQ_C1,Si3226x_audioGain_Presets[preset].aceq_c1);
    si_write_ram_32261_wrap(which_port,TXACEQ_C2,Si3226x_audioGain_Presets[preset].aceq_c2);
    si_write_ram_32261_wrap(which_port,TXACEQ_C3,Si3226x_audioGain_Presets[preset].aceq_c3);
    return 0;
}




/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3226x_RXAudioGainSetup (int which_port, int preset)
{
    si_write_ram_32261_wrap(which_port,RXACGAIN_SAVE,Si3226x_audioGain_Presets[preset].acgain);
    si_write_ram_32261_wrap(which_port,RXACGAIN,Si3226x_audioGain_Presets[preset].acgain);
    si_write_ram_32261_wrap(which_port,RXACEQ_C0,Si3226x_audioGain_Presets[preset].aceq_c0);
    si_write_ram_32261_wrap(which_port,RXACEQ_C1,Si3226x_audioGain_Presets[preset].aceq_c1);
    si_write_ram_32261_wrap(which_port,RXACEQ_C2,Si3226x_audioGain_Presets[preset].aceq_c2);
    si_write_ram_32261_wrap(which_port,RXACEQ_C3,Si3226x_audioGain_Presets[preset].aceq_c3);
    return 0;
}



/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedence synthesis
*/
int Si3226x_ZsynthSetup (int port) 
{

    unsigned char preset = 0;
    unsigned int cal_en = 0, reg_data;
    unsigned short timer = 500;

    si_read_reg_32261(port,LINEFEED, &reg_data);
    si_write_reg_32261(port,LINEFEED,0);
    /*
    ** Load provided coefficients - these are presumed to be 0dB/0dB
    */
    si_write_ram_32261_wrap(port,TXACEQ_C0,Si3226x_Impedance_Presets[preset].audioEQ.txaceq_c0);
    si_write_ram_32261_wrap(port,TXACEQ_C1,Si3226x_Impedance_Presets[preset].audioEQ.txaceq_c1);
    si_write_ram_32261_wrap(port,TXACEQ_C2,Si3226x_Impedance_Presets[preset].audioEQ.txaceq_c2);
    si_write_ram_32261_wrap(port,TXACEQ_C3,Si3226x_Impedance_Presets[preset].audioEQ.txaceq_c3);
    si_write_ram_32261_wrap(port,RXACEQ_C0,Si3226x_Impedance_Presets[preset].audioEQ.rxaceq_c0);
    si_write_ram_32261_wrap(port,RXACEQ_C1,Si3226x_Impedance_Presets[preset].audioEQ.rxaceq_c1);
    si_write_ram_32261_wrap(port,RXACEQ_C2,Si3226x_Impedance_Presets[preset].audioEQ.rxaceq_c2);
    si_write_ram_32261_wrap(port,RXACEQ_C3,Si3226x_Impedance_Presets[preset].audioEQ.rxaceq_c3);
    si_write_ram_32261_wrap(port,ECFIR_C2,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c2);
    si_write_ram_32261_wrap(port,ECFIR_C3,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c3);
    si_write_ram_32261_wrap(port,ECFIR_C4,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c4);
    si_write_ram_32261_wrap(port,ECFIR_C5,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c5);
    si_write_ram_32261_wrap(port,ECFIR_C6,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c6);
    si_write_ram_32261_wrap(port,ECFIR_C7,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c7);
    si_write_ram_32261_wrap(port,ECFIR_C8,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c8);
    si_write_ram_32261_wrap(port,ECFIR_C9,Si3226x_Impedance_Presets[preset].hybrid.ecfir_c9);
    si_write_ram_32261_wrap(port,ECIIR_B0,Si3226x_Impedance_Presets[preset].hybrid.ecfir_b0);
    si_write_ram_32261_wrap(port,ECIIR_B1,Si3226x_Impedance_Presets[preset].hybrid.ecfir_b1);
    si_write_ram_32261_wrap(port,ECIIR_A1,Si3226x_Impedance_Presets[preset].hybrid.ecfir_a1);
    si_write_ram_32261_wrap(port,ECIIR_A2,Si3226x_Impedance_Presets[preset].hybrid.ecfir_a2);
    si_write_ram_32261_wrap(port,ZSYNTH_A1,Si3226x_Impedance_Presets[preset].zsynth.zsynth_a1);
    si_write_ram_32261_wrap(port,ZSYNTH_A2,Si3226x_Impedance_Presets[preset].zsynth.zsynth_a2);
    si_write_ram_32261_wrap(port,ZSYNTH_B1,Si3226x_Impedance_Presets[preset].zsynth.zsynth_b1);
    si_write_ram_32261_wrap(port,ZSYNTH_B0,Si3226x_Impedance_Presets[preset].zsynth.zsynth_b0);
    si_write_ram_32261_wrap(port,ZSYNTH_B2,Si3226x_Impedance_Presets[preset].zsynth.zsynth_b2);
    si_write_reg_32261(port,RA,Si3226x_Impedance_Presets[preset].zsynth.ra);
    si_write_ram_32261_wrap(port,TXACGAIN,Si3226x_Impedance_Presets[preset].txgain);
    si_write_ram_32261_wrap(port,RXACGAIN_SAVE,Si3226x_Impedance_Presets[preset].rxgain);
    si_write_ram_32261_wrap(port,RXACGAIN,Si3226x_Impedance_Presets[preset].rxgain);
    si_write_ram_32261_wrap(port,RXACHPF_B0_1,Si3226x_Impedance_Presets[preset].rxachpf_b0_1);
    si_write_ram_32261_wrap(port,RXACHPF_B1_1,Si3226x_Impedance_Presets[preset].rxachpf_b1_1);
    si_write_ram_32261_wrap(port,RXACHPF_A1_1,Si3226x_Impedance_Presets[preset].rxachpf_a1_1);

    /*
    ** Scale based on desired gain plan
    */
    Si3226x_dbgSetGain(Si3226x_Impedance_Presets[preset].txgain_db,preset,TXACGAIN_SEL);
    Si3226x_dbgSetGain(Si3226x_Impedance_Presets[preset].rxgain_db,preset,RXACGAIN_SEL);

    Si3226x_TXAudioGainSetup(port, TXACGAIN_SEL);
    Si3226x_RXAudioGainSetup(port, RXACGAIN_SEL);
    /* 
    ** Perform Zcal in case OHT used (eg. no offhook event to trigger auto Zcal) 
    */
    si_write_reg_32261(port,CALR0,0x00);   
    si_write_reg_32261(port,CALR1,0x40);   
    si_write_reg_32261(port,CALR2,0x00); 
    si_write_reg_32261(port,CALR3,0x80);  /* start cal */

    /* Wait for zcal to finish */
    do {
        si_read_reg_32261(port,CALR3, &cal_en);
        msleep(100);
        timer--;
    } while((cal_en&0x80)&&(timer>0));  
     
    si_write_reg_32261(port, LINEFEED, reg_data);

    if (timer > 0) {
        return PASSED;
    } else {
        return FAILED;
    }

}



/*********************************************************************
 *
 * si32261_protected_mode()
 *
 * user access mode 
 *
 * Input: which_port
 *
 * OUtput: PASSED/FAILED
 *
 **********************************************************************
 */
int si32261_protected_mode (int which_port)
{
    int secret[4] = {0x02, 0x08, 0x0e, 0x0};
    int ix;
    int protect;
    char errstr[128];

    bsp_debug_printf("\n\r Protect Mode \r");
    if (si_read_reg_32261(which_port, USERMODE_ENABLE, (uint *)&protect) == FAILED) {
        cterr('f', 0, "%s Unable to read Protected mode", __FUNCTION__);
        sprintf(errstr, "\nUnable to read Protected mode\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    if (protect & 0x1) {
        return (PASSED);
    }

    for (ix = 0; ix < 4; ix++) {
        if (si_write_reg_32261(which_port, USERMODE_ENABLE, secret[ix]) == FAILED) {
            cterr('f', 0, "%s Unable to write %#x %d'th protect byte",
                  __FUNCTION__, secret[ix], ix);
            sprintf(errstr, "\nUnable to write %#x %d'th protect byte\n",secret[ix], ix);
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
    }

    /*
     * check again
     */
    if (si_read_reg_32261(which_port, USERMODE_ENABLE, (uint *)&protect) == FAILED) {
        cterr('f', 0, "%s Unable to read Protected mode after writes",
              __FUNCTION__);
        sprintf(errstr, "\nUnable read Protected mode after writes\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    if (protect & 0x1) {
        return (PASSED);
    } else {
        /* If the write fails the first time. the second write without
         * the read verify should not return PASSED */
        cterr('f', 0, "%s Unable to set protect mode", __FUNCTION__);
        sprintf(errstr, "\nUnable to set protect mode\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }


    return (PASSED);

}

/***********************************************************************
 *
 * Function: si32261_send_lpbk_data
 *
 * This function  sends read/write commands over monitor channel 
 * 
 * Input : which_port, data, channel_type
 *         which_siu 
 *
 * Returns: none
 *
 **********************************************************************
 */
void fill_si32261_test_buf (uint16_t *buf_ptr, int block_size, uint16_t data)
{
    int size;

    for (size=0; size<(block_size/2); size++,buf_ptr++) {
        *buf_ptr = SWAP16(data);
    }
}



/*
 *
 * Function: si32261_init
 *
 * This function initialize SI32261 chip regs.
 * Input : which_port,
 *
 * Returns: PASSED for no errors;
 *          FAILED for test failure.
 *
 */
int si32261_init(int which_port)
{
    int i;
    int read_val;

    /* clear & setup interrupt */


    for (i = 0; i < 4; i++) {
        if (si3xxx_reg_write(which_port, IRQ1+i, 0x0) == FAILED) {
            return (FAILED);
        }
    }

    /* Set linefeed to forward active */
    if (si3xxx_reg_write(which_port, LINEFEED, SI32261_FORWARD_ACTIVE) == FAILED) {
        return (FAILED);
    }

    /* check RAM clear status */
    if (si3xxx_reg_read(which_port, MSTRSTAT, &read_val) == FAILED) {
        return (FAILED);
    }

    if (!(read_val & SRAM_CLEAR_COMPLETED)) {
        cterr('f', 0, "RAM is still not clear yet! \n");
        return (FAILED);
    }

    /*
     * enable the PCM of register 11 bit 4
     * original default is 5,
     */
    return (si3xxx_reg_write(which_port, PCMMODE, 0x15) );
}



/***********************************************************************
 *
 * Function: si32261_codec_digital_loopback
 *
 * This function executes Digital loopback through si32261 codec chip.
 * 
 * Input : which_port
 *
 * Returns: PASS/FAIL
 *
 **********************************************************************
 */
int si32261_codec_digital_loopback (which_port)
{
    int channel, data, tdm_port;
    uchar board_id;
    char errstr[128];
    
    si32261_init(which_port);

    /* Setup voice port connection */
    board_id = get_oak_id(); 
    /* 
     * Set Codec in Digital loopback mode by setting bit 1 of 
     * Control register 43.
     */
    if (si3xxx_reg_write (which_port, LOOPBACK, 0x1) == FAILED) {
        cterr('f', 0,"\rControl register 43 set 1 fail\n\r");
        sprintf(errstr, "\nlpbk reg 43 set 1 fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return(FAILED);
    }


    channel =  (which_port) % 2;
    if(channel == 0) {   
        data = 0x5500;
    } else if (channel == 1) {
        data = 0xAAAA;
    }

    /* fill in with test data */
    fill_si32261_test_buf((uint16_t *)&tst_packet[0], sizeof(tst_packet), data);


    tdm_port = get_tdm_port(which_port, FXS_CODEC);
    /* send the packet through tdm channels */
    if (send_tdm_packet(&tst_packet[0], VOICE, tdm_port, 1)) {
        cterr('f', 0,"\r***send tdm fail\n\r");
        sprintf(errstr, "\nsend tdm packet fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /* verify data */
    if (verify_si3xxx_lpbk_digital_data(channel, tdm_port, (uint16_t)data)) {
        cterr('f', 0,"\rverify lpbk fail\n\r");
        sprintf(errstr, "\nverify lpbk fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /* 
     * Reset Codec in Digital loopback mode by resetting bit 0 of 
     * Control register 43, set back to init value 0.
     */
    if (si3xxx_reg_write(which_port, LOOPBACK, 0) == FAILED) {
        cterr('f', 0,"\rControl register 43 set 0 fail\n\r");
        sprintf(errstr, "\nRegister 43 set 0 fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return(FAILED);
    }

    return (PASSED);
   
}

/*********************************************************************
 *
 * Function: phoenix_codec_to_port_translate 
 *
 * This function will transfer code to port
 * Input : codec = codec
 *         board_id = board ID 
 *
 * Returns: port
 *
 **********************************************************************
 */
int phoenix_codec_to_port_translate (int codec, char board_id)
{
    int port;

    if (board_id == PHOENIX_144FXS) {
        if (codec < 8) {
            port = codec * 2;
        } else if ((8 <= codec) && (codec < 16)) {
            port = (codec - 2) * 2;
        } else if ((16 <= codec) && (codec < 24)) {
            port = (codec - 4) * 2;
        } else if ((24 <= codec) && (codec < 32)) {
            port = (codec - 6) * 2;
        } else {
            port = (codec - 8) * 2;
        }
    } else { /* 132FXS_6FXO and 84FXS_6FXO sku */
        if (codec < 8) {
            port = codec * 2;
        } else if ((8 <= codec) && (codec < 16)) {
            port = (codec - 2) * 2;
        } else if ((16 <= codec) && (codec < 24)) {
            port = (codec - 4) * 2;
        } else {
            port = (codec - 14) * 2;
        }
    }

    return (port);
}
/*********************************************************************
 *
 * si32261_codec_set_onoff_hook_map()
 *
 * set ON-OFF hook mapping with SLIC LCRRTP
 *     LCRRTP reg 34 = 0x22, bit1 LCR
 *     LCR Loop Closure Status Bit.
 *     0 = Loop closure not detected (on-hook)
 *     1 = Loop closure detected (off-hook)
 * Input: which codec
 *
 * OUtput: PASSED/FAILED
 *
 **********************************************************************
 */
int si32261_codec_set_onoff_hook_map(int codec)
{
    int port, ix;
    uchar board_id = 0xFF;

    board_id = get_oak_id();

    /* Transfer codec to port number */
    if (is_phoenix()) {
        port = phoenix_codec_to_port_translate(codec, board_id);
    } else {
        port = codec * 2;
    }
    if (si3226x_init_with_options(port)) {
        return (FAILED);
    }
    bsp_debug_printf("\r\n Check Codec Reg LCRRTP 34 = 0x22, bit1.");
    bsp_debug_printf("\r\n 0 = Loop closure not detected (on-hook)");
    bsp_debug_printf("\r\n 1 = Loop closure detected (off-hook)\n");

    for (ix = 0; ix < ONOFF_HOOK_MAP_PORT; ix++) {
        port = port + ix;
        bsp_debug_printf("\r\n Set ON/OFF hook mapping with port: %d\n", port);

        /* Set linefeed to forward active */
        si_write_reg_32261 (port, LINEFEED , 1);
        msleep(1000);
    }
     return (PASSED);
}

/*********************************************************************
 *
 * si32261_codec_set_ring()
 *
 * set ring
 *
 * Input: which codec
 *
 * OUtput: PASSED/FAILED
 *
 **********************************************************************
 */
int si32261_codec_set_ring(int codec)
{
    int port, ix;
    char errstr[128];
    uchar board_id = 0xFF;

    board_id = get_oak_id();

    /* Transfer codec to port number */
    if (is_phoenix()) {
        port = phoenix_codec_to_port_translate(codec, board_id);
    } else {
        port = codec * 2;
    }
    if (si3226x_init_with_options(port)) {
        return (FAILED);
    }

    for (ix = 0; ix < 2; ix++) {

        port = port + ix;

        bsp_debug_printf("\r\n Set Codec ring port: %d\n", port);

        Si3226x_DCFeedSetupCfg(port);
        bsp_debug_printf("\r\n Si3226x_DCFeedSetupCfg \n");
        Si3226x_RingSetup(port);
        bsp_debug_printf("\r\n Si3226x_RingSetup \n");
        bsp_debug_printf("\r\n Si3226x_ZsynthSetup \n");
        if (Si3226x_ZsynthSetup(port)) {
            cterr('f', 0,  "Set Ring, Si3226x_ZsynthSetup fail\n");
            sprintf(errstr, "\nSi32261ZsynthSetup fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }

    
        /* Set linefeed to forward active */
        si_write_reg_32261 (port, LINEFEED , 1);

        msleep(1000);

        /* Oakenshield    *(uint*)0x893=LINEFEED;   */
        si_write_reg_32261(port, LINEFEED, LF_RINGING);

        msleep(1000);
    }
     return (PASSED);
}



/*********************************************************************
 *
 * si32261_codec_stop_ring()
 *
 * stop ring
 *
 * Input: port
 *
 * OUtput: PASSED/FAILED
 *
 **********************************************************************
 */
int
si32261_codec_stop_ring(int which_port)
{
    return (si_write_reg_32261 (which_port, LINEFEED , 0));
}




/*
 *
 * Function: si32261_common_mode_calibration
 *
 * This function calibrates the si32261 chip.
 *
 * Input : which_port, *ret_data, mode
 *
 * Returns: PASS/FAIL
 *
 */
int si32261_common_mode_calibration (int which_port, uint * ret_data)
{
    uint32_t calibword;
    uint16_t calibdata[8]; /* 1458, 1459, 1476, 1477 4 bytes each */
    uint16_t *bufmsg_p = &calibdata[0];

    /* si3226x_init_with_options will initialize one codec once */
    bsp_debug_printf("\r Do SI32261 initialization\r");
    if (which_port % 2 == 0) {
        if (si3226x_init_with_options(which_port) == FAILED) {
            cterr('f', 0, "si3226x_init_with_options failed.");
            return (FAILED);
        }
    }

    msleep(1000);

    PRINT_STR("\rWaiting for LB calibration to complete");

    if (Si3226x_LBCal(which_port) == FAILED) {
        cterr('f', 0, "Si3226x_LBCal timeout");
    }

    msleep(100); 
    bsp_debug_printf("\n\r Getting port %d calibration data \r", which_port);
    si_read_ram_32261(which_port, 1476, bufmsg_p + 4, bufmsg_p + 5);
    si_read_ram_32261(which_port, 1477, bufmsg_p + 6, bufmsg_p + 7);


    /* Compress the received calibration data into 4 bytes */
    /* Per DSP codecdrv.c - 1458 and 1459 [23:16], 1476 and 1477 [25:18] */
    calibword = (((uint32_t)calibdata[0] << 24) |
         (((uint32_t)calibdata[2] & 0xFF) << 16) |
         ((((uint32_t)calibdata[4] >> 2) & 0xFF) << 8) |
         (((uint32_t)calibdata[6] >> 2) & 0xFF));
    *ret_data = calibword;

    /* Write to the flash will be handled in the caller */
    bsp_debug_printf("\n\r calibdata %x %x %x %x %x %x %x %x\n",
        calibdata[0], calibdata[1], calibdata[2], calibdata[3],
        calibdata[4], calibdata[5], calibdata[6], calibdata[7]);
    bsp_debug_printf("\n\r Calibration data = %x \n\n", *ret_data);

    if (si_write_reg_32261(which_port, LINEFEED, 2) == FAILED) {
        return (FAILED);
    }


    return (PASSED);
}



/*
 *
 * Function: si32261_common_mode_calibration_wo_result
 *
 * This function calibrates the si32261 chip.
 *
 * Input : dummy
 *
 * Returns: PASS/FAIL
 *
 */
int si32261_common_mode_calibration_wo_result (int dummy)
{
    int which_port, which_port_num, max_fxs_port, port, port_num;
    char errstr[128];
    
    do_calibration_flag = TRUE;
    char board_id = 0xFF;
    max_fxs_port = get_fxs_port_num();
 
    /* si3226x_load_patch will load four codec(eight port) once */
    bsp_debug_printf("\r Do SI32261 Load Patch\r");
    board_id = get_oak_id();
    if (is_phoenix()) {
        for (port = 0; port < max_fxs_port; port++ ) {
            /* Skip port if some MB/DBx not test */
            if (phoenix_separ_dbx_port_translate(&port, max_fxs_port, board_id) == TRUE){
                continue;
            }

            if (si32261_protected_mode(port)==FAILED) {
                cterr('f', 0,  "si3226x_init_with_options->Set protect fail\n");
                return (FAILED);
            }
            msleep(10);
        }    
    } else { /* Not phoenix did not include DBx */
        if (board_id == VG400_2FXS_2FXO) {
            port_num = VG_2FXS_STR_PORT;
        } else {
            port_num = 0;
        }
        for (port = port_num; port < max_fxs_port; port++) {
            if (si32261_protected_mode(port)==FAILED) {
                cterr('f', 0,  "si3226x_init_with_options->Set protect fail\n");
                return (FAILED);
            }
                msleep(10);
        }
    }

    /* load patch */
    if (si32261_load_patch_cal(max_fxs_port)) {
        cterr('f', 0, "si3226x_init_with_options->load_patch fail\n");
        sprintf(errstr, "\nload patch fail\n");
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }
    bsp_debug_printf("Do SI32261 initialization\n");
    /* si3226x_init_with_options will initialize one codec once */
    if (board_id == VG400_2FXS_2FXO) {
        which_port_num = VG_2FXS_STR_PORT;
    } else {
        which_port_num = 0;
    }
    for (which_port = which_port_num; which_port < max_fxs_port; which_port++) {
        if (is_phoenix()) {
            /* Skip port if some MB/DBx not test */
            if (phoenix_separ_dbx_port_translate(&which_port, max_fxs_port, 
                        board_id) == TRUE){
                continue;
            }
        }

        bsp_debug_printf("\n\r Port: %d \r", which_port);

        if (which_port % 2 == 0) {
            if (si3226x_init_with_options(which_port) == FAILED) {
                cterr('f', 0, "si3226x_init_with_options failed.");
                sprintf(errstr, "\ninit_options failed\n");
                strcat((char *)&(hd_if->errmsg), errstr);
                return (FAILED);
            }
        }

        msleep(100);

        if (Si3226x_LBCal(which_port) == FAILED) {
            cterr('f', 0, "Si3226x_LBCal timeout");
            sprintf(errstr, "\nLBCal timeout\n");
            strcat((char *)&(hd_if->errmsg), errstr);
        }
    }

    do_calibration_flag = FALSE;

    return (PASSED);
}


/*
 *
 * Function: si32261_save_cal_data
 *
 * This function collects FXS calibration result on specific port
 * 
 * Input : None
 *
 * Returns: PASS/FAIL
 *
 */
int si32261_save_cal_data (void)
{
    int which_port, max_fxs_port, which_port_num, dbx_max_fxs_port;
    char calib_data_str[20];
    uint16_t calibdata[8]; /* 1458, 1459, 1476, 1477 4 bytes each */
    char save_data[80], cal_num[10];
    uint32_t calibword = 0;
    uint16_t *bufmsg_p = &calibdata[0];
    char board_id = 0xFF;

    memset(save_data, 0 , sizeof(save_data));
    memset(calib_data_str, 0 , sizeof(calib_data_str));

    max_fxs_port = get_fxs_port_num();
    /* Only work for Phoenix */
    dbx_max_fxs_port = max_fxs_port + 1;
    
    board_id = get_oak_id();
    if (board_id == VG400_2FXS_2FXO) {
        which_port_num = VG_2FXS_STR_PORT;
    } else {
        which_port_num = 0;
    }
    /* 
     * Setup Phoenix only DBx calibration and ending port not % 8
     * 132FXS_6FXO DB1 and DB2
     * 84FXS_6FXO DB1
     */
    if(is_phoenix()) {
        if(board_id == PHOENIX_132FXS_6FXO) {
            if ((!phoenix_has_dbx(BOARD_DB2_TEST)) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                /* DB1 end port at 36 */
                dbx_max_fxs_port = FXS_PORT36;
            } else if (!phoenix_has_dbx(BOARD_DB3_TEST)) {
                /* DB2 end port at 84 */
                dbx_max_fxs_port = FXS_PORT84;
            }
        } else if (board_id == PHOENIX_84FXS_6FXO) {
            if (!phoenix_has_dbx(BOARD_DB2_TEST)) {
                /* DB1 end port at 36 */
                dbx_max_fxs_port = FXS_PORT36;
            }
        }
    }

    for (which_port = which_port_num; which_port < max_fxs_port; which_port++) {
        if (is_phoenix()) {
            /* Skip port if some MB/DBx not test */
            if (phoenix_separ_dbx_port_translate(&which_port, max_fxs_port,
                        board_id) == TRUE){
                continue;
            }
        }

        bsp_debug_printf("\n\r Port: %d \r", which_port);
        si_read_ram_32261(which_port, 1476, bufmsg_p + 4, bufmsg_p + 5);
        si_read_ram_32261(which_port, 1477, bufmsg_p + 6, bufmsg_p + 7);


        /* Compress the received calibration data into 4 bytes */
        /* Per DSP codecdrv.c - 1458 and 1459 [23:16], 1476 and 1477 [25:18] */
        calibword = (((uint32_t)calibdata[0] << 24) |
                    (((uint32_t)calibdata[2] & 0xFF) << 16) |
                    ((((uint32_t)calibdata[4] >> 2) & 0xFF) << 8) |
                    (((uint32_t)calibdata[6] >> 2) & 0xFF));

        calibword = (0x10100000 | (calibword & 0xFFFF));

        /* Write to the flash will be handled in the caller */
        bsp_debug_printf("\n\r calibdata %x %x %x %x %x %x %x %x\n",
            calibdata[0], calibdata[1], calibdata[2], calibdata[3],
            calibdata[4], calibdata[5], calibdata[6], calibdata[7]);
        bsp_debug_printf("\n\r Calibration data = %x \n\n", calibword);


        if (si_write_reg_32261(which_port, LINEFEED, 2) == FAILED) {
            return (FAILED);
        }

        sprintf(&calib_data_str[0], " %x", (int)calibword);
        strcat(&save_data[0], &calib_data_str[0]);

        if (which_port % 8 == 7) {
            sprintf(&cal_num[0], "calib_%d", ((which_port+1)/8));

            bsp_debug_printf("\r\n %s= %s\n", cal_num, save_data);
            /* Send calibration value back to host side */
            sprintf((char *)&(hd_if->bufmsg), "%s=%s", cal_num, save_data);

            sp_SetGPIODataHigh(0x40);
            env_get();
            env_set_string(cal_num, save_data);
            env_sync(0);
            sp_SetGPIODataLow(0x40);

            memset(save_data, 0 , sizeof(save_data));
        } else if ((which_port % max_fxs_port) == (max_fxs_port -1)) {
            sprintf(&cal_num[0], "calib_%d", (((which_port+1)/8)+1));

            bsp_debug_printf("\r\n %s= %s\n", cal_num, save_data);
            /* Send calibration value back to host side */
            sprintf((char *)&(hd_if->bufmsg), "%s=%s", cal_num, save_data);

            sp_SetGPIODataHigh(0x40);
            env_get();
            env_set_string(cal_num, save_data);
            env_sync(0);
            sp_SetGPIODataLow(0x40);

            memset(save_data, 0 , sizeof(save_data));
        /* Phoenix has special case for signal DBx calibration case */ 
        } else if ((which_port % dbx_max_fxs_port) == (dbx_max_fxs_port -1)) {
            sprintf(&cal_num[0], "calib_%d", (((which_port+1)/8)+1));

            bsp_debug_printf("\r\n %s= %s\n", cal_num, save_data);
            /* Send calibration value back to host side */
            sprintf((char *)&(hd_if->bufmsg), "%s=%s", cal_num, save_data);

            sp_SetGPIODataHigh(0x40);
            env_get();
            env_set_string(cal_num, save_data);
            env_sync(0);
            sp_SetGPIODataLow(0x40);

            memset(save_data, 0 , sizeof(save_data));
        } 
        memset(calib_data_str, 0 , sizeof(calib_data_str));

    }


    return (PASSED);
}


/*
 *
 * Function: si32261_collect_cal_result
 *
 * This function collects FXS calibration result on specific port
 * 
 * Input : None
 *
 * Returns: PASS/FAIL
 *
 */
int si32261_collect_cal_result (void)
{
    uint16_t *bufmsg_p = (uint16_t *)&(hd_if->bufmsg);
    int which_port = hd_if->param1;

    si_read_ram_32261(which_port, 1458, bufmsg_p, bufmsg_p + 1);
    si_read_ram_32261(which_port, 1459, bufmsg_p + 2, bufmsg_p + 3);
    si_read_ram_32261(which_port, 1476, bufmsg_p + 4, bufmsg_p + 5);
    si_read_ram_32261(which_port, 1477, bufmsg_p + 6, bufmsg_p + 7);


    return (PASSED);
}




/******** History ********
$Log: tstcodec_si32261.c,v $
Revision 1.4  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3.64.4  2020/12/01 02:38:42  hondwang

CSCvw60842:
    Fix DSP single daught board calibration issue
Fixed by:
   1. Add program call for single daught board rnu calibration
Server: sjc-ads-9168

Revision 1.3  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.3  2018/05/08 23:03:49  haohsu
Suport FXO Ring test

Revision 1.2.28.2  2018/04/13 03:18:01  haohsu
Code Change for Vg400 (return calibration value, skip FXO loopback in FXO SKus)

Revision 1.2.28.1  2018/03/23 10:07:14  haohsu
Code change for calibration data saving

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.9  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.1.2.8  2017/04/17 06:08:47  olin2
Remove Enable PLL function

Revision 1.1.2.7  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.1.2.6  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.1.2.5  2017/03/02 08:13:59  harrchan
Delete limits value check on LB calibration for the Si3226x.

Revision 1.1.2.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.3  2017/01/05 06:06:34  olin2
Support FXS Ring and Calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:39  olin2
Initial commit code for Oakenshield




$Endlog$
*/


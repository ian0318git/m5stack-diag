/* $Id: tstcodec_si3050.c,v 1.2 2017/07/28 07:58:51 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/tstcodec_si3050.c,v $
 *------------------------------------------------------------------
 * tstcodec_si3050.c
 *      Oakenshield project: test with codec SI3050 
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c)2017 by Cisco Systems, Inc.
 *
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


/*===================================================================*
 *                             Globals                               *
 *===================================================================*/
static si3050_init_t si3050_init_tbl[] = {
    {SI3050_CONTROL_1,		0x80}, /* Register 1 */
    {SI3050_DAA_CONTROL_2,	0x00}, /* Register 6 */
    {SI3050_CONTROL_2,		0x07}, /* Register 2 */
    {SI3050_DC_TERM_CTRL,	0xC0}, /* Register 26 */
    {SI3050_GCI_CTL,		0x05}, /* Register 42 */
    {SI3050_RING_V_CTL_1,	0x09}, /* Register 22 */
    {SI3050_RING_V_CTL_2,	0x17}, /* Register 23 */
    {SI3050_RING_V_CTL_3,	0x8D}, /* Register 24 */
    {0xFF, 0xFF},
};

uint8_t test_packet[80];


/***********************************************************************
 *
 * Function: si3050_codec_init
 *
 * Description: Initialize the Si3050 Codec's for operation
 *
 * Input : which_port
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int si3050_codec_init (int which_port)
{
    si3050_init_t *tbl_p = &si3050_init_tbl[0];

    while (tbl_p->reg != 0xFF) {
        if (si3050_reg_write(which_port, tbl_p->reg, tbl_p->data) == FAILED) {
            cterr('f', 0, "%s: Failed to write %#x to register %d",
                     __FUNCTION__, tbl_p->data, tbl_p->reg);
            return (FAILED);
        }
        tbl_p++;
    }

    return (PASSED);
}


/***********************************************************************
 *
 * Function: fill_si3050_test_buf
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
void fill_si3050_test_buf (uint16_t *buf_ptr, int block_size, uint16_t data)
{
    int size;

    for (size = 0; size < (block_size/2); size++, buf_ptr++) {
        *buf_ptr = SWAP16(data);
    }
}


/***********************************************************************
 *
 * Function: si3050_codec_digital_loopback
 *
 * Description: This function executes Digital loopback through si3050 codec chip.
 *
 * Input : which_port
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
int si3050_codec_digital_loopback(int which_port)
{
    int channel, data, tdm_port;
    char errstr[128];

    /*
     * read line side chip status reg to detect frame signal
     */
    if (si3050_reg_read(which_port, LINE_SIDE_CHIP_STATUS_REG, &data) == FAILED) {
           cterr('f', 0, "%s: Unable to read lineside chip status register @ %#x",
                 __FUNCTION__, LINE_SIDE_CHIP_STATUS_REG);

           sprintf(errstr, "\nUnable read lineside reg@ %#x\n",
                             LINE_SIDE_CHIP_STATUS_REG);
           strcat((char *)&(hd_if->errmsg), errstr);
           return(FAILED);
    }

    /* check the value read */
    if (!(data & FRAME_DETECT)) {
        cterr('f', 0, "%s: ISOcap link frame lock is not established, reg = %x",
                       __FUNCTION__, data);
        sprintf(errstr,"\nISOcap link frame lock,reg = %x\n",
                        data);
        strcat((char *)&(hd_if->errmsg), errstr);
        return(FAILED);
    }

    /*
     * Set Codec in Digital loopback mode by setting bit 0 of
     * Control register 10.
     */
    if (si3050_reg_write(which_port, DAA_CONTROL_3_REG,
                         DIGITAL_DATA_LOOPBACK) == FAILED) {
        cterr('f', 0, "%s: Unable set loopback reg %#x",
                       __FUNCTION__, DAA_CONTROL_3_REG);
        sprintf(errstr,"\nUnable set loopback reg %#x\n",
                        DAA_CONTROL_3_REG);
        strcat((char *)&(hd_if->errmsg), errstr);
        return(FAILED);
    }


    /*
     * Send a data pattern
     */
    channel =  (which_port) % 2;
    if(channel == 0) {   
        data = 0x5500;
    } else if (channel == 1) {
        data = 0x0055;
    }

    /* fill in with test data */
    fill_si3050_test_buf((uint16_t *)&test_packet[0], sizeof(test_packet),
                            data);

    tdm_port = get_tdm_port(which_port, FXO_CODEC);
    /* send the packet through tdm channels */
    if (send_tdm_packet(&test_packet[0], VOICE, tdm_port, 1)) {
        return (FAILED);
    }

    /* verify data */
    if (verify_si3xxx_lpbk_digital_data(channel, tdm_port, (uint16_t)data)) {
        return (FAILED);
    }

    /* disable the digital loopback set up in the DAA_CONTROL_3_REG */
    if (si3050_reg_write(which_port, DAA_CONTROL_3_REG, 0) == FAILED) {
        cterr('f', 0, "%s: Unable to reset the loopback at register %#x",
                       __FUNCTION__, DAA_CONTROL_3_REG);
        sprintf(errstr,"\nUnable reset lpbk reg %#x\n",
                        DAA_CONTROL_3_REG);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /* Per Si3050 data sheet. Page 22. Section 5.8 Exception Handling -
     * Commnunication with the line-side device takes less than 10 ms to
     * establish.
     */
    msleep(15);

    /*
     * read line side chip status reg to detect frame signal
     */
    if (si3050_reg_read(which_port, LINE_SIDE_CHIP_STATUS_REG, &data) == 
                        FAILED) {
        cterr('f', 0, "%s: Lineside chip status register @ %#x read failed",
              __FUNCTION__, LINE_SIDE_CHIP_STATUS_REG);

        sprintf(errstr,"\nLineside reg @ %#x read fail\n",
                       LINE_SIDE_CHIP_STATUS_REG);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
	}

    /* check the value read */
    bsp_debug_printf("\ndata is %d\n",data);
    if (!(data & FRAME_DETECT)) {
        cterr('f', 0, "ISOcap link frame lock is not established after test. %#x", data);
        sprintf(errstr,"\nISOcap link frame lock not established after test. %#x\n", data);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /*
     * read line side chip device ID
     */
    if (si3050_reg_read(which_port, SYSTEM_SIDE_CHIP_REV_REG,
                        &data) == FAILED) {
        cterr('f', 0, "%s: Unable to read lineside chip Device ID @ %#x",
              __FUNCTION__, SYSTEM_SIDE_CHIP_REV_REG);
        sprintf(errstr,"\nUnable read lineside Device ID @ %#x\n",
               SYSTEM_SIDE_CHIP_REV_REG);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /* check the value read */
    if ((data & 0xf0) != SILAB_ID_3019) {
        cterr('f', 0, "%s: System side chip REV, reg %#x = 0x%2x",
              __FUNCTION__, SYSTEM_SIDE_CHIP_REV_REG, data);
        sprintf(errstr,"\nSystem side chip REV,reg %#x = 0x%2x\n",
               SYSTEM_SIDE_CHIP_REV_REG, data);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    return (PASSED);
}

/************* History ************
 * $Log: tstcodec_si3050.c,v $
 * Revision 1.2  2017/07/28 07:58:51  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1.2.5  2017/03/30 10:25:50  harrchan
 * Add fpga upgrade utility
 *
 * Revision 1.1.2.4  2017/03/09 07:23:34  harrchan
 * Support oakenshield double wide case
 *
 * Revision 1.1.2.3  2017/01/17 05:07:06  olin2
 * Clean up debug code
 *
 * Revision 1.1.2.2  2016/12/23 06:56:04  olin2
 * Support FXS/FXO loopback test
 *
 * Revision 1.1.2.1  2016/12/14 04:57:38  olin2
 * Initial commit code for Oakenshield
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

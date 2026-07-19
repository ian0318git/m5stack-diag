/* $Id: patriot_test.c,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_test.c
 *
 * Description: This file is for test functions
 *
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "patriot_main.h"
#include "common_utils.h"
#include "ds3170.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_etsec.h"
#include "p1021_immap.h"
#include "p1021_espi.h"
#include "patriot_intr.h"

extern uchar err_msg[];
extern signed int p1021_fd;
extern fe_packet_t *tx_packet_p;
extern int param;
extern unsigned char to_host_param[10];
uchar *fpga_data;
uchar *fw_data;

ulong uncached_data;

extern unsigned char patriot_fpga_prom[];
extern unsigned long int patriot_fpga_prom_size;

extern unsigned char param_arr[6];
extern unsigned char param_cmd_menu;

extern mac_addr_t module_mac_addr;
extern mac_addr_t host_mac_addr;

extern boolean n2g_flag;

/*
 * Kentrox bandwidth table for T3
 * These values come from the HW functional spec.
 * These values are to be programmed into the FPGA register for desired
 * rates.
 * the first value is for 1000 Kbits/sec, 2nd value is for 1500 Kbits/sec
 * 3rd is for 2000 Kbits/sec .... the last is for 35000 Kbits/sec.
 * NOTE: value for 1000 Kbits/sec is not given in the spec., thus value
 * for 1500 Kbits/sec is used for 1000 Kbits/sec.
 */
ulong patriot_t3_ken_tbl[] = {
    0x5ff167,
    0x5FF167, 0x47F167, 0x2A714C, 0x2FF167, 0x337181, 0x23F167, 0x1FF167,
    0x15314C, 0x0F313C, 0x17F167, 0x13F15D, 0x19B181, 0x153172, 0x11F167,
    0x063126, 0x0FF167, 0x12317C, 0x0A914C, 0x06112E, 0x07913C, 0x0D316D,
    0x0BF167, 0x043126, 0x09F15D, 0x039123, 0x0CD181, 0x035123, 0x0A9172,
    0x07114F, 0x08F167, 0x07915A, 0x031126, 0x07B161, 0x07F167, 0x03312B,
    0x09117C, 0x04D144, 0x05414C, 0x017116, 0x06115C, 0x066163, 0x079178,
    0x07F181, 0x06916D, 0x04E153, 0x05F167, 0x05315C, 0x021126, 0x031139,
    0x04F15D, 0x05B16D, 0x01C123, 0x03C14B, 0x066181, 0x03D14F, 0x035146,
    0x04415B, 0x054172, 0x01511E, 0x03814F, 0x030145, 0x047267, 0x03424D,
    0x03C25A, 0x015321, 0x018426, 0x03A65B, 0x03D761,
};


/*
 * Kentrox bandwidth table for E3
 * These values come from the HW functional spec.
 * These values are to be programmed into the FPGA register for desired
 * rates.
 * the first value is for 1000 Kbits/sec, 2nd value is for 1500 Kbits/sec
 * 3rd is for 2000 Kbits/sec .... the last is for 24500 Kbits/sec.
 */
ulong patriot_e3_ken_tbl[] = {
    0x03673E, 0x05173E, 0x03639E, 0x0473CE, 0x05139E, 0x02C1AE, 0x0361CE,
    0x02C14E, 0x0471E6, 0x06928E, 0x0511CE, 0x07325E, 0x02C0D6, 0x0671d6,
    0x0360E6, 0x05D176, 0x02C0A6, 0x07319E, 0x0470F2, 0x016046, 0x069146,
    0x057102, 0x0510E6, 0x020056, 0x07312E, 0x021052, 0x02C06A, 0x03607E,
    0x0670EA, 0x05C0CA, 0x036072, 0x0790FA, 0x05D0BA, 0x03706A, 0x02C052,
    0x07F0EA, 0x0730CE, 0x03B066, 0x047078, 0x044070, 0x016022, 0x019026,
    0x1690A2, 0x237052, 0x357080, 0x65D086, 0x751072, 0x74D06A,
};

/* T3 Digital Link | Bandwidth | Timeslot | Hex
 *                 |   44210   |   146    | 0x92
 *                 |   34010   |   113    | 0x71
 *                 |   20000   |   66     | 0x42
 *                 |   10000   |   33     | 0x21
 *                 |     300   |    0     | 0x0
 */
ulong patriot_t3_dig_link_tbl[] = {
    0x0, 0x21, 0x42, 0x71, 0x92,
};

/* T3 Larscom      | Bandwidth | Timeslot | Hex
 *                 |   44210   |   13     | 0xd
 *                 |   34010   |   10     | 0xa
 *                 |   20000   |   6      | 0x6
 *                 |   10000   |   3      | 0x3
 *                 |    3100   |   0      | 0x0
 */
ulong patriot_t3_larscom_tbl[] = {
    0x0, 0x3, 0x6, 0xa, 0xd,
};

/* T3 Adtran       | Bandwidth | Timeslot | Hex
 *                 |   44210   |   587    | 0x24b
 *                 |   34010   |   452    | 0x1c4
 *                 |   20000   |   266    | 0x10a
 *                 |   10000   |   133    | 0x85
 *                 |      75   |     0    | 0x0
 */
ulong patriot_t3_adtran_tbl[] = {
    0x0, 0x85, 0x10a, 0x1c4, 0x24b,
};

/* T3 Verilink     | Bandwidth | Timeslot | Hex
 *                 |   44210   |   27     | 0x1b
 *                 |   34010   |   22     | 0x16
 *                 |   20000   |   13     | 0xd
 *                 |   10000   |    6     | 0x6
 *                 |    1500   |    0     | 0x0
 */
ulong patriot_t3_verilink_tbl[] = {
    0x0, 0x6, 0xd, 0x16, 0x1b,
};

/* E3 Digital Link | Bandwidth | Timeslot | Hex
 *                 |   34010   |   95     | 0x5f
 *                 |   20000   |   56     | 0x38
 *                 |   10000   |   28     | 0x1c
 *                 |     358   |    1     | 0x1
 */
ulong patriot_e3_dig_link_tbl[] = {
    0x1, 0x1c, 0x38, 0x5f,
};

static reg_info_t patriot_te3_fpga_regs[] = {
    {"LED Control                  ",
                                0x00, SAVE_RESTORE, {BW_8BITS}, 0x1F, 0x3F},
    {"Port Type Select             ",
                                0x01, READ_ONLY,  {BW_8BITS}, 0x79, 0x00},
    {"Framer GPIO                  ",
                                0x02, READ_ONLY,  {BW_8BITS}, 0xFF, 0x00},
    {"Framer GPIO OE               ",
                                0x03, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub Mode Sel              ",
                                0x06, READ_ONLY,  {BW_8BITS}, 0x3F, 0x00},
    {"T3 Sub BandWidth Sel1        ",
                                0x07, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub BandWidth Sel2        ",
                                0x08, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub BandWidth Sel3        ",
                                0x09, READ_WRITE, {BW_8BITS}, 0x7F, 0x00},
    {"E3 Sub Mode Sel              ",
                                0x0a, READ_ONLY,  {BW_8BITS}, 0x17, 0x00},
    {"E3 Sub BandWidth Sel1        ",
                                0x0b, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"E3 Sub BandWidth Sel2        ",
                                0x0c, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"E3 Sub BandWidth Sel3        ",
                                0x0d, READ_WRITE, {BW_8BITS}, 0x7F, 0x00},
    {"TDM FPGA Rev. Reg            ",
                                0x0e, READ_ONLY,  {BW_8BITS}, 0xFF, 0x00},
    {"Scratch Pad Reg             ",
                                0x0f, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"Interrupt cause reg          ",
                                0x10, READ_ONLY,  {BW_8BITS}, 0x03, 0x00},
    // FPGA multiboot registers
    {"FPGA Reconfig Ctrl Reg                                      ",
     0x20, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Reconfig Status Reg                                    ",
     0x21, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 0(LS Byte)       ",
     0x24, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 1                ",
     0x25, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 2                ",
     0x26, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 3(MS Byte)       ",
     0x27, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 0(LS Byte)     ",
     0x28, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 1              ",
     0x29, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 2              ",
     0x2a, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 3(MS Byte)     ",
     0x2b, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Flag.Byte 0(LS Byte)         ",
     0x2c, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 0(LS Byte) ",
     0x30, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 1          ",
     0x31, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 2          ",
     0x32, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 3(MS Byte) ",
     0x33, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 0(LS Byte)            ",
     0x34, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 1                     ",
     0x35, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Bytte 2                    ",
     0x36, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 3(MS Byte)            ",
     0x37, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot State History Reg.Byte 0(LS Byte)          ",
     0x38, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot State History Reg.Byte 1(MS Byte)          ",
     0x39, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 0(LS Byte)                ",
     0x3c, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 1                         ",
     0x3d, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 2                         ",
     0x3e, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 3(MS Byte)                ",
     0x3f, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 0(LS Byte)           ",
     0x40, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 1                    ",
     0x41, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 2                    ",
     0x42, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 3(MS Byte)           ",
     0x43, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"end",                     0xff, 0, {0}, 0x0, 0x0},
};



/**********************************************************************
 *
 * Function: patriot_read_fpga_version
 * This function read FPGA version
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_read_fpga_version(void)
{
    uchar version;

    if (patriot_fpga_get_version(&version)) {
	return (TO_HOST_READ_FPGA_VERSION_FAIL);
    }
    
    to_host_param[0] = version;
    printf("\npatriot_read_fpga_version completed\n");fflush(0);
    return (TO_HOST_READ_FPGA_VERSION_OK);
    
}



/**********************************************************************
 *
 * Function: patriot_fpga_download_to_fpga
 * This function download FPGA
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

int
patriot_fpga_download_to_fpga(void)
{

    int fpga_size = param;
    int i, j, temp;
    unsigned char fpga_ver;
    
    /* Set up the GPIO pins on the CPU */
    /* PB29, PB30, PB31, PC0 are OUTPUT, PB8 is input */
        /* Configure pin direction and  function */
    
    /* PB29 as output DIN */

    /* clear direction bits for PB29 */
    REGB->im_gur.cpdir2b &= ~(MPC8500_CPDIR2_INOUT(29));
    /* PB29 as output */
    REGB->im_gur.cpdir2b |=  MPC8500_CPDIR2_OUT(29);
    /* clear function bits for PB29 and set it as GPIO*/
    REGB->im_gur.cppar2b &= ~(MPC8500_CPPAR2(29, 0x3));

    /* PB30 as output DCLK*/

    /* clear direction bits for PB30 */
    REGB->im_gur.cpdir2b &= ~(MPC8500_CPDIR2_INOUT(30));
    /* PB29 as output */
    REGB->im_gur.cpdir2b |=  MPC8500_CPDIR2_OUT(30);
    /* clear function bits for PB30 and set it as GPIO*/
    REGB->im_gur.cppar2b &= ~(MPC8500_CPPAR2(30, 0x3));


    /* PB31 as output PROGRAM_B */

    /* clear direction bits for PB31 */
    REGB->im_gur.cpdir2b &= ~(MPC8500_CPDIR2_INOUT(31));
    /* PB29 as output */
    REGB->im_gur.cpdir2b |=  MPC8500_CPDIR2_OUT(31);
    /* clear function bits for PB31 and set it as GPIO*/
    REGB->im_gur.cppar2b &= ~(MPC8500_CPPAR2(31, 0x3));    


    /* PC0 as output INIT_B, set it to be open-drain first */

    REGB->im_gur.cpodrc |= 0x80000000;
    /* clear direction bits for PC0 */
    REGB->im_gur.cpdir1c &= ~(MPC8500_CPDIR1_INOUT(0));
    /* PC0 as output */
    REGB->im_gur.cpdir1c |=  MPC8500_CPDIR1_OUT(0);
    /* clear function bits for PC0 and set it as GPIO*/
    REGB->im_gur.cppar1c &= ~(MPC8500_CPPAR1(0, 0x3));


    /* PB8 as input DONE */

    /* clear direction bits for PB8 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(8));
    /* PB8 as input */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_IN(8);
    /* clear function bits for PB8 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(8, 0x3));

    /* Check the DONE pin.  If it is asserted, set to 1, the
     * configuration is complete.  Stop here
     */
    if (REGB->im_gur.cpddatb & 0x00800000) {
        printf("\nDONE pin is high. Stop download FPGA");fflush(0);
	patriot_fpga_reset();
	platform_cpu_i2c_init();
	patriot_fpga_get_version(&fpga_ver);
        return (TO_HOST_FPGA_DOWNLOAD_TO_FPGA_OK);
    }
#ifdef DEBUG
    printf("\nREGB->im_gur.cpddatb = 0x%08x", REGB->im_gur.cpddatb);fflush(0);
#endif
    
    /* 1.	Assert the PROGRAM pin & the INIT pin.  Assertion of the
       Programming pin should be for a minimum of 500 ns. */
    /* Active low */
    REGB->im_gur.cpddatb &= ~0x00000001;
    REGB->im_gur.cpddatc &= ~0x80000000;
    wastetime(10);

    /* 2.	Negate the PROGRAM pin */    
    REGB->im_gur.cpddatb |= 0x00000001;

    /* 3.	Release the INIT pin (The MODE pins should be stable by now).
     set it to be input */
    /* clear direction bits for PC0 */
    REGB->im_gur.cpdir1c &= ~(MPC8500_CPDIR1_INOUT(0));
    /* PC0 as Input */
    REGB->im_gur.cpdir1c |=  MPC8500_CPDIR1_IN(0);

    /* 4.	Set the CCLK pin low. */
    REGB->im_gur.cpddatb &= ~0x00000002;
#ifdef DEBUG    
     printf("\npatriot_fpga_size = 0x%08x", patriot_fpga_size);
#endif
     printf("\nPlease wait for FPGA download\n");

     for (i = 0; i < patriot_fpga_prom_size; i++) {
	 if (i % 0x1000 == 0) {
	     printf(". ");fflush(stdout);
	 }
        for (j = 0; j < 8; j++) {

            /* 5. Place the data, 1 bit at a time, on the DIN pin (relative to
             * the FPGA) MSB first
             */
	    
	    if ((patriot_fpga_prom[i] << j) & 0x80) {
                REGB->im_gur.cpddatb |= 0x00000004;
            } else {
                REGB->im_gur.cpddatb &= ~0x00000004;
            }
	    wastetime(3);
            /* 6.	Set the CCLK pin high.  Wait a minimum of 16ns.  */
            REGB->im_gur.cpddatb |= 0x00000002;
	    wastetime(3);

            /* 7.	Set the CCLK pin low.  Wait a minimum of 16ns. */
            REGB->im_gur.cpddatb &= ~0x00000002;
	    

            /* 8.	Repeat steps 5 ' 8 until the bitstream is loaded. */
        }
     }

     msleep(100); 

     /* 9.Check the INIT pin to determine if the configuration failed
     *   to load.  If it did, the FPGA download was corrupted, bad CRC.
     *   Note:  0 = CRC error, 1 = No CRC error
     */
    if ((REGB->im_gur.cpddatc & 0x80000000) == 0) {
        sprintf(err_msg, "\n%s, [#%d]:INIT pin is not high, fail to download "
        		"FPGA", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_FPGA_DOWNLOAD_TO_FPGA_FAIL);
    }

    for (i = 0; i < 8; i++) {
        /* 10.	Set the CCLK pin high.  Wait a minimum of 16ns.  Due to
         * 33Mhz max clock frequency.
         */
        REGB->im_gur.cpddatb |= 0x00000002;
        wastetime(5);

        /* 11.	Set the CCLK pin low.  Wait a minimum of 16ns. */
        REGB->im_gur.cpddatb &= ~0x00000002;
        wastetime(5);
        /* 12.	Repeat steps 10 ' 11 a total of 8 times, i.e. 7 more times */
    }

    /* HW suggests to delay for 10ms */
    msleep(500);
    /* 13. Check the DONE pin.  If it is asserted, set to 1, the
     * configuration is complete.  If it is negated, the FPGA has failed
     * to load, re-start from step 1.
     */
    if ((REGB->im_gur.cpddatb & 0x00800000) == 0) {
        sprintf(err_msg, "\n%s, [#%d]:DONE pin is not high. Fail to download "
        		"FPGA", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_FPGA_DOWNLOAD_TO_FPGA_FAIL);
    }

    /* 14.	To initialize the device we need to "clock" it several more
       times. */
    for (i = 0; i < 8; i++) {
        /* 15. Set the CCLK pin high.  Wait a minimum of 16ns.  Due to
         * 33Mhz max clock frequency. */
        REGB->im_gur.cpddatb |= 0x00000002;
        wastetime(5);

        /* 16.	Set the CCLK pin low.  Wait a minimum of 16ns. */
        REGB->im_gur.cpddatb &= ~0x00000002;
        wastetime(5);
        /* 17.	Repeat steps 14 ' 15 a total of 8 times, i.e. 7 more times */
    }
    /* 18.	The FPGA should be configured now. */

    patriot_fpga_reset();
    platform_cpu_i2c_init();
    patriot_fpga_get_version(&fpga_ver);
    
    return (TO_HOST_FPGA_DOWNLOAD_TO_FPGA_OK);
}


/**********************************************************************
 *
 * Function: patriot_fpga_reg_test
 * This function test FPGA registers
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_reg_test(void)
{
    printf("\npatriot_fpga_reg_test\n");


    if (register_tests(patriot_te3_fpga_regs, I2C_BUS)) {
    	sprintf(err_msg, "\n%s, [#%d]:FPGA Register test fail",
    			__FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_FPGA_REG_TEST_FAIL);
    }

    printf("\npatriot_fpga_reg_test completed\n");fflush(0);
    return (TO_HOST_FPGA_REG_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_dump_fpga_reg
 * This function dump FPGA registers
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void
patriot_dump_fpga_reg(void)
{
    printf("\npatriot_dump_fpga_reg\n");
    register_display(patriot_te3_fpga_regs, I2C_BUS);
    printf("\n");
}


/**********************************************************************
 *
 * Function: patriot_fpga_register_alter
 *
 * This function provide for fpga utilites to alter fpga register
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void patriot_fpga_register_alter(void)
{
    printf("\npatriot_fpga_register_alter");
    register_alter(patriot_te3_fpga_regs, I2C_BUS);
}


/**********************************************************************
 *
 * Function: patriot_cpu_alive_test
 * This function test to make sure the CPU is alive and can response
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_cpu_alive_test(void)
{

    printf("\npatriot_cpu_alive_test completed\n");fflush(0);
    /* For this test, only need to send back the ACK */
    return (TO_HOST_CPU_ALIVE_TEST_OK);
    
}



/**********************************************************************
 *
 * Function: patriot_memory_test
 * This function test memory on Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_memory_test(void)
{
    printf("\npatriot_memory_test\n");

    if (module_mem_test()) {
        sprintf(err_msg, "\n%s, [#%d]:Memory test fail",
               __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_MEMORY_TEST_FAIL);
    }

    printf("\npatriot_memory_test completed\n");fflush(0);
    return (TO_HOST_MEMORY_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_spi_prom_test
 * This function test SPI PROM on Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_spi_prom_test(void)
{
    int i, j, num_of_pages, addr_offset, spi_num, cs;
    uchar *write_buf;
    uchar read_buf[SPI_PROM_TEST_SIZE];
    
    printf("\npatriot_spi_prom_test\n");
    if (patriot_spi_prom_init()) {
    	sprintf(err_msg, "\n%s, [#%d]:Fail spi prom init", __FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_SPI_PROM_TEST_FAIL);
    }
    
    spi_num = param;
    
    if (spi_num == 0) {
        cs = ESPI_CS0;
    } else {
        cs = ESPI_CS2;
    }    

    write_buf = (uchar *)malloc(SPI_PROM_TEST_SIZE);
    if (write_buf == NULL) {
    	sprintf(err_msg, "\n%s, [#%d]:Malloc write buffer Failed \n", __FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
    	return (TO_HOST_SPI_PROM_TEST_FAIL);
    }
    /* Test the last sector */
    /* fill with incremental pattern */

    num_of_pages = SPI_PROM_TEST_SIZE/SPI_PROM_PAGE_SIZE;
#ifdef DEBUG    
    printf("\nnum_of_pages = %d", num_of_pages);
#endif    
    for (i = 0; i < num_of_pages; i++) {
        for (j = 0; j < SPI_PROM_PAGE_SIZE; j++) {
            write_buf[j + i*SPI_PROM_PAGE_SIZE] = j;
        }
    }

    printf("Erase sector at %#.8x...", LAST_SECTOR_ADDR);
    /* Need to erease a sector of 64K */
    if (spi_prom_erase_if (LAST_SECTOR_ADDR,
        ERASE_64K_BLOCK, cs)) {
    	sprintf(err_msg, "\n%s, [#%d]:Fail to erase a sector of 64K", __FUNCTION__, __LINE__);
    	print_err(TRUE, err_msg, LVL_0);
        free(write_buf);
        return (TO_HOST_SPI_PROM_TEST_FAIL);
    }

    printf("\nProgram and verify sector at 0x%08x...", LAST_SECTOR_ADDR);
    addr_offset = LAST_SECTOR_ADDR;

    for (i = 0; i < num_of_pages; i++) {
	if (spi_prom_write_multi_bytes(addr_offset,
				       &write_buf[i * SPI_PROM_PAGE_SIZE], cs,
				       SPI_PROM_PAGE_SIZE)) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to write multi bytes to spi prom", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_SPI_PROM_TEST_FAIL);
        }
	addr_offset += SPI_PROM_PAGE_SIZE;
    }

    memset(&read_buf[0], 0, SPI_PROM_TEST_SIZE);
    
    addr_offset = LAST_SECTOR_ADDR;

    for (i = 0; i < SPI_PROM_TEST_SIZE; i++) {
        if (spi_prom_read_if(addr_offset, &read_buf[i], cs)) {
            sprintf(err_msg, "\n%s, [#%d]:Fail to read spi prom", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG	
	printf("\nread_buf[%d] = 0x%02x", i, read_buf[i]);fflush(0);
#endif	
        if (read_buf[i] != write_buf[i]) {
            sprintf(err_msg, "\n%s, [#%d]:Fail SPI PROM test, expect = 0x%02x, get = 0x%02x", __FUNCTION__, __LINE__,
                   write_buf[i], read_buf[i]);
            print_err(TRUE, err_msg, LVL_0);
            fflush(0);
            return (TO_HOST_SPI_PROM_TEST_FAIL);
        }
        addr_offset++;
    }

    printf("\npatriot_spi_prom_test\n");fflush(0);
    return (TO_HOST_SPI_PROM_TEST_OK);
    
}


/**********************************************************************
 *
 * Function: patriot_ds3170_reg_test
 * This function test Maxim DS3170 registers
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ds3170_reg_test(void)
{
    printf("\npatriot_ds3170_reg_test\n");
    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    /* Reset then take it out of reset */
    patriot_ds3170_reset();
    /* Shutdown PMUXCR bit 2 to zero to become GPIO instead of SDHC_WP */
    REGB->im_gur.pmuxcr &= (~0x20000000);
    return (ds3170_register_test());
}


/**********************************************************************
 *
 * Function: patriot_clear_e3_ais_test
 *
 * This function tests E3 AIS thru the LIU.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_e3_ais_test(void)
{
    uchar expected, ix = 0;
    uchar tmp_val = 0;
    uchar temp = 0;
    uchar lpbk_mode = NOTUSED;
    uchar bypass_subrate = BYPASS_SUB;
    uchar bypass_fpga = FALSE;

    printf("\npatriot_clear_e3_ais_test\n");
    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
    /* Configure Framer with Mode E3 */
    /* Set Diagnostics Loopback */
    if (patriot_conf_ds3170_frmr(CR4_LBM(0x7), MODE_E3, lpbk_mode,
    		bypass_subrate, bypass_fpga)) {
        sprintf(err_msg, "\n%s, [#%d]:patriot_conf_ds3170_frmr(), fail.\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }

    /* NO PAIS, NO LAIS
     * Automatic AIS when Diagnostic Loopback Enable (DLB) */
    if (ds3170_read(&temp, CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_H Reg read fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }

    temp &= CR1_PAIS(0x7);
    temp &= CR1_LAIS(0x3);

    if (ds3170_write(temp, CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_H Reg write fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, CR1_ADDR_H)) {
        printf("\n CR1_ADDR_H Reg read fail\n");
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
    printf("\n%s, [#%d]: CR1_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* LIU ON, JA OFF */
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_H Reg read fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }

    /* Tx Line IO signal disable for normal operation (non-liu)
     * Line Mode select LIU enable and JA is OFF
     */
    temp &= ~CR2_TLEN;
    temp &= CR2_LM(0x0);
    temp |= CR2_LM(0x1);

    if (ds3170_write (temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_LM(1) write reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
#ifdef DEBUG
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        printf("\nCR2_ADDR_H Reg read fail\n");
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
    printf("\n%s, [#%d]: CR2_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Set Framer mode for E3 G.751 FRAMED */
 	if (ds3170_read(&temp, CR2_ADDR_L)) {
 		sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n",
 				__FUNCTION__, __LINE__);
 		print_err(TRUE, err_msg, LVL_0);
 		return (TO_HOST_E3_AIS_TEST_FAIL);
 	}
 	temp |= CR2_FM(0x2);

 	if(ds3170_write(temp, CR2_ADDR_L)) {
 		sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg write fail\n",
 				__FUNCTION__, __LINE__);
 		print_err(TRUE, err_msg, LVL_0);
 		return (TO_HOST_E3_AIS_TEST_FAIL);
 	}

#ifdef DEBUG
 	if (ds3170_read(&temp, CR2_ADDR_L)) {
 		printf("\n CR2_ADDR_L Reg read fail\n");
 		return (TO_HOST_E3_AIS_TEST_FAIL);
 	}
 	printf("\n%s, [#%d]: CR2_ADDR_L contents = 0x%02x", __FUNCTION__,
 	   __LINE__, temp);
#endif

    /* Enable AIS E3G751.TCR
     */
    if (ds3170_read (&temp, G751_TCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:G751_TCR_ADDR_L read reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }

    temp |= G751_TCR_TAIS;

    if (ds3170_write (temp, G751_TCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:G751_TCR_TAIS write reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
#ifdef DEBUG
    if (ds3170_read (&temp, G751_TCR_ADDR_L)) {
        printf("\n G751_TCR_ADDR_L read reg fail\n");
        return (TO_HOST_E3_AIS_TEST_FAIL);
    }
    printf("\n%s, [#%d]: G751_TCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Set expected DS3170 E3 G751 Register values */
    expected = G751_RSR1_AIS | G751_RSR1_LOF; /* E3 G.751 */

    for (ix=0; ix<4; ix++) {
	/* Read the status register from Framer */
	if (ds3170_read (&tmp_val, G751_RSR1_ADDR_L)) {
	    sprintf(err_msg, "\n%s, [#%d]:G751_RSR1_ADDR_L read reg fail\n",
	    		__FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_E3_AIS_TEST_FAIL);
	}
#ifdef DEBUG
	printf("\n%s, [#%d]: G751_RSR1_ADDR_L contents = 0x%02x , ix=%d",
	       __FUNCTION__,__LINE__, tmp_val, ix);
#endif
	if (((tmp_val & expected) != expected) && (ix == 3)){
	    sprintf(err_msg, "\n%s, [#%d]:LIU AIS, Didn't Receive AIS Status "
	    		"in DS3170 Framer. Expected %#x, Got %#x.\n",
	    		__FUNCTION__, __LINE__,
		   expected, (tmp_val & expected));
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_E3_AIS_TEST_FAIL);
	} else {
	    continue;
	}
    }
    printf("\n %s test complete \n", __FUNCTION__);fflush(0);
    
    return (TO_HOST_E3_AIS_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_intr_test
 *
 * This function tests the Clear T3 Internal loopbacks with interrupt
 * enabled.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_intr_test(void)
{
    uchar temp, temp1, temp2 = 0;
    uchar lpbk_mode = NOTUSED;
    uchar bypass_subrate = BYPASS_SUB;
    uchar bypass_fpga = TRUE;
    int framer_intr = 0, framer_intr_bit = 0, ret_val;
    sm_patriot_framer_intr_iface_t framer_intr_iface;
    patriot_msg_t msg;
    uchar intr_msg[120];

    printf("\npatriot_clear_t3_intr_test\n");

    memset((uchar *)&framer_intr_iface, 0,sizeof(sm_patriot_framer_intr_iface_t));
    memset((uchar *)&msg, 0, sizeof(patriot_msg_t));
    memset((uchar *)&intr_msg, 0, sizeof(uchar));

    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    /* Set the Framer on T3 Mode, Analog loopback */
    if (patriot_conf_ds3170_frmr(CR4_LBM(1), MODE_T3, lpbk_mode,
    		bypass_subrate, bypass_fpga)) {
        sprintf(err_msg, "\n%s, [#%d]:patriot_conf_ds3170_frmr(), fail.\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    ret_val = ioctl(p1021_fd, IOCTL_CLEAR_FRAMER_INTR, (unsigned long)&msg);

    /* Check OOF, SEF, OOMF T3.RSR1 */
	if (ds3170_read(&temp, T3_RSR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
               "T3_RSR1_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
		return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
	}

#ifdef DEBUG
	printf("\n%s, [#%d]: T3_RSR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
    if (temp & (T3_RSR1_OOF|T3_RSR1_SEF|T3_RSR1_OOMF)) {
    	printf("\nOOF, SEF, OOMF status register detected\n");
    }
    /* Clear on read T3.RSRL1 */
    if (ds3170_read(&temp, T3_RSRL1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
               "T3_RSRL1_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: T3_RSRL1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Check OOF, SEF, OOMF (0xC2) */
    if(temp & (T3_RSRL1_OOFL|T3_RSRL1_SEFL|T3_RSRL1_OOMFL)) {
    	printf("\nOOFL, SEFL, OOMFL Latched detected\n");
        if (ds3170_read(&temp, T3_RSRIE1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
                   "T3_RSRIE1_ADDR_L.\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
        }
        
        if (temp & (T3_RSRIE1_OOFIE|T3_RSRIE1_SEFIE|T3_RSRIE1_OOMFIE)) {
        	printf("\nOOF, SEF, OOMF Interupt enable detected\n");
        	/* Disable the Interupt OOF, SEF, OOMF */
			temp &= ~T3_RSRIE1_OOFIE;
			temp &= ~T3_RSRIE1_SEFIE;
			temp &= ~T3_RSRIE1_OOMFIE;
	        if (ds3170_write(temp, T3_RSRIE1_ADDR_L)) {
                sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
                   "T3_RSRIE1_ADDR_L.\n", __FUNCTION__, __LINE__);
                print_err(TRUE, err_msg, LVL_0);
	            return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
	        }
        }

        if (ds3170_read(&temp, T3_RSRL1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
                   "T3_RSRL1_ADDR_L.\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
        }
#ifdef DEBUG
        printf("\n%s, [#%d]: T3_RSRL1_ADDR_L contents = 0x%02x", __FUNCTION__,
    	   __LINE__, temp);
#endif
        /* Make sure the OOF, SEF, OOMF is cleared */
        if(temp & (T3_RSRL1_OOFL|T3_RSRL1_SEFL|T3_RSRL1_OOMFL)) {
        	printf("\nDouble Check OOFL, SEFL, OOMFL detected\n");
        }
    }

    /* Enable Interrupt */
    if (ds3170_read(&temp, ISRIE_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
               "ISRIE_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }
    temp |= ISRIE_PISRIE | ISRIE_GSRIE;

    /* Global status register interrupt status interrupt enable
     * Port interrupt status register interrupt enable */
    if (ds3170_write(temp, ISRIE_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:write failed to reg "
               "ISRIE_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    /* Out of frame interrupt enable */
    if (ds3170_read(&temp,T3_RSRIE1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
              "T3_RSRIE1_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    temp |= T3_RSRIE1_OOFIE;
    if (ds3170_write(temp, T3_RSRIE1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:write failed to reg "
              "T3_RSRIE1_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    /* Force the FRSYNC to force the bit 1(00F) to trigger the interrupt */
    if (ds3170_read(&temp, T3_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
               "T3_RCR_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    temp |= T3_RCR_FRSYNC;
    if (ds3170_write(temp, T3_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:write failed to reg "
               "T3_RCR_ADDR_L.\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    /* Check if the interrupt handler being called in ds3170_intr_hndlr()
     * increase the framer_intr.
     */
    msleep(3000);
    ret_val = ioctl(p1021_fd, IOCTL_GET_FRAMER_INTR, (unsigned long)&msg);
#ifdef DEBUG
    printf("\nFRAMER msg.data[0] = %d\n", msg.data[0]);
    printf("\nFRAMER msg.data[1] = %d\n", msg.data[1]);
#endif
    framer_intr = msg.data[0];
    framer_intr_bit = msg.data[1];

    if (!framer_intr) {
        sprintf(err_msg, "%s, [#%d]:Clear T3 Interrupt, Didn't get interrupt for "
                "Cause %#.8x, framer_intr_bit %#.8x",__FUNCTION__, __LINE__, framer_intr, framer_intr_bit);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    } else {
        if (framer_intr_bit != CHECK_ALL_INT) {
            if ((framer_intr_bit & CHECK_ISR_PISR) == 0) {
                sprintf(intr_msg, "CHECK_ISR_PISR");
            }
            if ((framer_intr_bit & CHECK_ISR_FMSR) == 0) {
                sprintf(eos(intr_msg), "CHECK_ISR_FMSR");
            }
            if ((framer_intr_bit & CHECK_OOFIE_SEFIE_OOMFIE) == 0) {
                sprintf(eos(intr_msg), "CHECK_OOFIE_SEFIE_OOMFIE");
            }
            if ((framer_intr_bit & CHECK_OOFL_SEFL_OOMFL) == 0) {
                sprintf(eos(intr_msg), "CHECK_OOFL_SEFL_OOMFL");
            }
            sprintf(err_msg, "%s, [#%d]:Clear T3 Interrupt, get interrupt for "
                   "Cause %#.8x, but \n%s not occur.",
                   __FUNCTION__, __LINE__, framer_intr, intr_msg);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
        } else {
            printf("\nAll Framer Interrupt occur !");
        }
    }

    /* Clear the FRSYNC the bit to 0(00F) */
    if (ds3170_read(&temp, T3_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:read failed to reg "
               "T3_RCR_ADDR_L.\n",__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    temp &= ~T3_RCR_FRSYNC;
    if (ds3170_write(temp, T3_RCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:write failed to reg "
               "T3_RCR_ADDR_L.\n",__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_INTR_TEST_FAIL);
    }

    printf("\npatriot_clear_t3_intr_test\n");fflush(0);
    return (TO_HOST_CLR_T3_INTR_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_bert_test
 *
 * This function tests the Clear T3 Bert. The pattern is generated in
 * the DS3170 framer and looped back at the LIU to the framer.
 * THIS TEST REQUIRES A CABLE TO BE ATTACHED TO WORK CORRECTLY.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_bert_test(void)
{
    uchar temp, temp1, temp2 = 0;
    uchar oos, oosl, bel;
    ulong i = 0;
    uchar lpbk_mode = NOTUSED;
    uchar bypass_subrate = BYPASS_SUB;
    uchar bypass_fpga = FALSE;

    printf("\npatriot_clear_t3_bert_test\n");
    /*
     * LIU Bert Loopback test should not be run without a cable.
     * At the time of bring-up on this test was modified
     * from running between the BERT chip and the LIU to be run
     * from the BERT chip out the LIU and out the cable.
     *
     * BERT CHIP <----> LIU <-----> CABLE
     *
     * Test will hence fail unless cable is installed.
     * So check for external loopback flag ON before allowing
     * test execution.
     */

    /* Initialize */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:DS3170 Init fail\n", __FUNCTION__,
        		__LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }
    /* Set the Framer on T3 Mode */
    if (patriot_conf_ds3170_frmr(CR4_LBM(0x0), MODE_T3, lpbk_mode,
    		bypass_subrate, bypass_fpga)) {
        sprintf(err_msg, "\n%s, [#%d]:patriot_conf_ds3170_frmr(), fail.\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }
    /* Configure FPGA to specified subrate and bandwidth if applicable.
       if an invalid subrate is specified, it configuresￊￊthe FPGA to
       clear T3 mode if the mode is different from the FPGA's current
       mode, the FPGA 's DSU block will be reset
    */
    if (patriot_t3_set_subrate(CLEAR, 0)) {
        sprintf(err_msg, "\n%s, [#%d]:set subrate fail\n", __FUNCTION__,
        		__LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    /* Loop time enable for transmit using the receive clock */
    if (ds3170_read(&temp, CR3_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read CR3_ADDR_L reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= CR3_LOOPT;

    if (ds3170_write(temp, CR3_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write CR3_ADDR_L reg fail\n",
				__FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
		return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
	}

#ifdef DEBUG
    if (ds3170_read(&temp, CR3_ADDR_L)) {
        printf("\nT3 Bert Test, read CR3_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: CR3_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /*  Set the Framer BERT Register enable
     *  The BERT must be enabled before the pattern is loaded
     *  for the pattern load operation to take affect.
     */
    if (ds3170_read(&temp, CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read CR1_ADDR_H reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= CR1_BENA;

    if (ds3170_write(temp, CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write CR1_ADDR_H reg fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, CR1_ADDR_H)) {
        printf("\nT3 Bert Test, read CR1_ADDR_H reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: CR1_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* DS3170  Specs Table 10-31
     * Set Pseudo-Random Pattern Generation  2^11-1 O.153 */
    if (ds3170_read(&temp, BERT_PCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_PCR_ADDR_L reg "
        		"fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= BERT_PCR_PLF(0x0A);
    temp &= ~(BERT_PCR_PTS | BERT_PCR_QRSS);

    if (ds3170_write(temp, BERT_PCR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_PCR_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_PCR_ADDR_L)) {
        printf("\nT3 Bert Test, read BERT_PCR_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_PCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (ds3170_read(&temp, BERT_PCR_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_PCR_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= BERT_PCR_PTF(0x08);

    if (ds3170_write(temp, BERT_PCR_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_PCR_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_PCR_ADDR_H)) {
        printf("\nT3 Bert Test, read BERT_PCR_ADDR_H reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_PCR_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (ds3170_read(&temp, BERT_SPR2_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_SPR2_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= 0xff;

    if (ds3170_write(temp, BERT_SPR2_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_SPR2_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_SPR2_ADDR_L)) {
        printf("\nT3 Bert Test, read BERT_SPR2_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_SPR2_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (ds3170_read(&temp, BERT_SPR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_SPR2_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= 0xff;

    if (ds3170_write(temp, BERT_SPR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_SPR2_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_SPR2_ADDR_H)) {
        printf("\nT3 Bert Test, read BERT_SPR2_ADDR_H reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_SPR2_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (ds3170_read(&temp, BERT_SPR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_SPR1_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= 0xff;

    if (ds3170_write(temp, BERT_SPR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_SPR1_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_SPR1_ADDR_L)) {
        printf("\nT3 Bert Test, read BERT_SPR1_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_SPR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (ds3170_read(&temp, BERT_SPR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_SPR1_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= 0xff;

    if (ds3170_write(temp, BERT_SPR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_SPR1_ADDR_H reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_SPR1_ADDR_H)) {
        printf("\nT3 Bert Test, read BERT_SPR1_ADDR_H reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_SPR1_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* After Configure above bit, the pattern must be loaded into the BERT
     * This is accomplished via a zero-to-one transition on BERT.CR.TNPL
     * and BERT.CR.RNPL
     */
    if (ds3170_read(&temp, BERT_CR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_CR_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp |= (BERT_CR_TNPL  | BERT_CR_RNPL);

    if (ds3170_write(temp, BERT_CR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, write BERT_CR_ADDR_L reg"
        		" fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_CR_ADDR_L)) {
        printf("\nT3 Bert Test, read BERT_CR_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_CR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Read Status Register Latched for OOS and OOSL */
    if (ds3170_read(&temp, BERT_SR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Read OOS and BEC fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: BERT_SR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    oos = temp & BERT_SR_OOS;

    /* Read status register latched for OOSL */
    if (ds3170_read(&temp, BERT_SRL_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Read OOSL fail\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: BERT_SRL_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    oosl = temp & BERT_SRL_OOSL;

    /* Checking receive pattern generator is sync to the incoming pattern
     * also checking the out of sync changes status latched.
     */
    i = 100;
    while (--i && !(oos & BERT_SR_OOS) && (oosl & BERT_SRL_OOSL)) {
    	msleep(1);
        /* Read Status Register Latched for OOS and OOSL */
        if (ds3170_read(&temp, BERT_SR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Read OOS and BEC "
            		"fail\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
        }
        oos = temp & BERT_SR_OOS;

        /* Read status register latched for OOSL */
        if (ds3170_read(&temp, BERT_SRL_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Read OOSL fail\n",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
        }
        oosl = temp & BERT_SRL_OOSL;
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: oos contents = 0x%02x, ix=%d", __FUNCTION__,
	   __LINE__, oos, i);
    printf("\n%s, [#%d]: oosl contents = 0x%02x", __FUNCTION__,
	   __LINE__, oosl);
#endif

    if(!i) {
    	sprintf(err_msg, "\n%s, [#%d]:T3 BERT Test, Unable to SYNC or "
    			"find a pattern. Status oos = 0x%02x, oosl = 0x%02x\n",
    			__FUNCTION__, __LINE__, oos, oosl);
    	print_err(TRUE, err_msg, LVL_0);
    } else {
        /* reset the bit error counter */
        if (ds3170_read(&temp, BERT_CR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, read BERT_CR_ADDR_L"
            		" reg fail\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
        }

        temp &= ~BERT_CR_PMUM;
        temp |= BERT_CR_LPMU;

        if (ds3170_write(temp, BERT_CR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, reset the bit error"
            		" reg fail\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
        }

        /* clear previous status */
        if (ds3170_read(&temp, BERT_SR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Read OOS and "
            		"BEC fail\n", __FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
        }

        temp |= 0x00;

        if (ds3170_write(temp, BERT_SR_ADDR_L)) {
			sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, "
					"write BERT_SR_ADDR_L fail\n", __FUNCTION__, __LINE__);
			print_err(TRUE, err_msg, LVL_0);
			return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
		}

    }

    msleep (20);  /* test  time */

    /* reset the bit error counter */
    if (ds3170_read(&temp, BERT_CR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, "
        		"read BERT_CR_ADDR_L reg fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    temp &= ~BERT_CR_PMUM;
    temp |= BERT_CR_LPMU;

    if (ds3170_write(temp, BERT_CR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, "
        		"reset the bit error reg fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    if (ds3170_read(&temp, BERT_CR_ADDR_L)) {
        printf("\nT3 Bert Test, read BERT_CR_ADDR_L reg fail\n");
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n%s, [#%d]: BERT_CR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Read Status Register Latched for OOS , OOSL and BEL */
    if (ds3170_read(&temp, BERT_SR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, "
        		"Read OOS and BEC fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: BERT_SR_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    oos = temp & BERT_SR_OOS;

    /* Read status register latched for OOSL */
    if (ds3170_read(&temp, BERT_SRL_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, "
        		"Read OOSL fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

#ifdef DEBUG
    printf("\n%s, [#%d]: BERT_SRL_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    oosl = temp & BERT_SRL_OOSL;
    bel = temp & BERT_SRL_BEL;

    /* Check the bit error count */
    if (bel & BERT_SRL_BEL) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Bit error is detected"
               ", status %#x\n", __FUNCTION__, __LINE__, bel);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    if (oosl & BERT_SRL_OOSL) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Receive OOS bit is "
        		"changes state, status %#x\n", __FUNCTION__, __LINE__, oosl);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    /* Check the pattern if out of sync */
    if (oos & BERT_SR_OOS) {
        sprintf(err_msg, "\n%s, [#%d]:T3 Bert Test, Rx pattern generator "
        		"is not sync to the incoming pattern, status %#x\n", oos);
        print_err(TRUE, err_msg, LVL_0);
        return (TO_HOST_CLR_T3_BERT_TEST_FAIL);
    }

    printf("\n %s Testing Done \n", __FUNCTION__);fflush(0);
    return (TO_HOST_CLR_T3_BERT_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_fs_lpbk_test
 *
 * This function tests the CPU loopback
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fs_lpbk_test(void)
{
    int frame_num = 0, frame_size, i, ret_val, etsec_num;
    tsec_info_struct_t *tsec_p;
    tsec_bd_t  *tx_bd, *rx_bd, *rx_bd_vir_addr;
    unsigned char *rd_ptr, type, lpbk_op;
    unsigned char read_buf[1600];
    fe_packet_t *rx_pak;
    unsigned short pk_size;
    
    printf("\npatriot_fs_lpbk_test\n");

    if (n2g_flag) {
	etsec_num = ETSEC2;
    } else {
	etsec_num = ETSEC3;
    }
    
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);

    while (frame_num < NUM_RX_BD) {
        if (patriot_receive_frames(etsec_num, tsec_p, POLL_MODE) == PASSED) {
	    frame_num++;
            rx_bd = (tsec_bd_t *)etsec_get_rxbd(tsec_p);
	    rx_bd_vir_addr = (tsec_bd_t *)vir_addr((ulong)rx_bd);
            rd_ptr = (unsigned char *)vir_addr((ulong)rx_bd_vir_addr->buf_ptr);

            memset((uchar *)read_buf, 0, 1600);

            for (i = 0; i < rx_bd_vir_addr->length; i++) {
                read_buf[i] = *rd_ptr;
                rd_ptr++;
            }

	    memset((uchar *)rd_ptr, 0, MAX_RX_BUF);
	    
	    /* mark frame as processed */
	    rx_bd_vir_addr->status &= ~PQUICC_BDSTAT_RX_RO1;
	    rx_bd_vir_addr->status |= PQUICC_BDSTAT_RX_EMPTY;
	    /*
	     * if wrap occurs, we must re-initialize the tx and rx
	     * buffer descriptors so that we can Tx/Rx more frames
	     */
	    ret_val = check_for_bd_wrap(etsec_num);

	    /* If fail to reinitialize the tx & rx BD's, quit */
	    if (ret_val ==  FAILED) {
		memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
		tx_packet_p->data[0] = TO_HOST_REINIT_TX_RX_FAIL;
		if (patriot_send_frames(etsec_num, tsec_p, POLL_MODE)) {
		    sprintf(err_msg, "\n%s, [#%d]:Failed to send frames to host\n",
		    		__FUNCTION__, __LINE__);
		    print_err(TRUE, err_msg, LVL_0);
		    break;
		}
		break;
	    }
	    
            rx_pak = (fe_packet_t *)&read_buf[0];
            type = rx_pak->data[0];

            if (type != PATRIOT_DATA) {
                sprintf(err_msg, "\n%s, [#%d]:Wrong type, must be data type\n",
                		__FUNCTION__, __LINE__);
                print_err(TRUE, err_msg, LVL_0);
                return (TO_HOST_FREESCALE_LPBK_TEST_FAIL);
            }
            lpbk_op = rx_pak->data[1];

            if (lpbk_op != PATRIOT_CPU_LPBK) {
                sprintf(err_msg, "\n%s, [#%d]:Wrong loopback option, must "
                		"be loopback\n", __FUNCTION__, __LINE__);
                print_err(TRUE, err_msg, LVL_0);
                return (TO_HOST_FREESCALE_LPBK_TEST_FAIL);
            }
            pk_size = (rx_pak->data[2] << 8) | rx_pak->data[3];

            /* Clean up the tx packet */
            memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
            tx_packet_p->data[0] = PATRIOT_DATA;
            tx_packet_p->data[1] = PATRIOT_CPU_LPBK;
            tx_packet_p->data[2] = rx_pak->data[2];
            tx_packet_p->data[3] = rx_pak->data[3];
            /* Copy the data over */
            memcpy((char *)&(tx_packet_p->data[4]), (char *)&(rx_pak->data[4]),
                pk_size);
            /* Send back the data packet */
            if (patriot_send_frames(etsec_num, tsec_p, INTR_MODE)) {
                sprintf(err_msg, "\n%s, [#%d]:Failed to send frames to host\n",
                		__FUNCTION__, __LINE__);
                print_err(TRUE, err_msg, LVL_0);
                return (TO_HOST_FREESCALE_LPBK_TEST_FAIL);
            }
        }
    }
    printf("\npatriot_fs_lpbk_test completed\n");fflush(0);
    return (TO_HOST_FREESCALE_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_fpga_lpbk_test
 *
 * This function is a wrapper for Clear T3 Internal loopbacks.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_lpbk_test(void)
{
    printf("\npatriot_fpga_lpbk_test\n");
    if(patriot_clear_te3_test(MODE_T3, FPGA_LPBK)) {
        return (TO_HOST_FPGA_LPBK_TEST_FAIL);
    }
    printf("\npatriot_fpga_lpbk_test completed\n");fflush(0);
    return (TO_HOST_FPGA_LPBK_TEST_OK);
}



/**********************************************************************
 *
 * Function: patriot_clear_t3_int_lpbk_test
 *
 * This function is a wrapper for Clear T3 Internal loopbacks.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_int_lpbk_test(void)
{
    printf("\npatriot_clear_t3_int_lpbk_test\n");
    if(patriot_clear_te3_test(MODE_T3, INT_LPBK)) {
        return (TO_HOST_CLR_T3_LPBK_TEST_FAIL);
    }

    printf("\npatriot_clear_t3_int_lpbk_test completed\n");fflush(0);
    return (TO_HOST_CLR_T3_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_ext_lpbk_test
 *
 * This function is a wrapper for Clear T3 External loopbacks.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_ext_lpbk_test(void)
{
    printf("\npatriot_clear_t3_ext_lpbk_test\n");
    if(patriot_clear_te3_test(MODE_T3, EXT_LPBK)) {
        return (TO_HOST_CLR_T3_EX_LPBK_TEST_FAIL);
    }
    printf("\npatriot_clear_t3_ext_lpbk_test completed\n");fflush(0);
    return (TO_HOST_CLR_T3_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_t3_int_lpbk_test
 *
 * This function performs T3 subrates internal loopback.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_int_lpbk_test(void)
{
    printf("\npatriot_clear_t3_subrate_int_lpbk_test\n");
    if (patriot_clear_te3_subrate_test(MODE_T3, KENTROX,
        patriot_t3_ken_tbl[KENTROX_10K], INT_LPBK))
        return (TO_HOST_SUB_T3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, DIGITAL_LINK,
        (DIG_LINK_MAX_TS/2), INT_LPBK))
        return (TO_HOST_SUB_T3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, LARSCOM,
        (LARSCOM_MAX_TS/2), INT_LPBK))
        return (TO_HOST_SUB_T3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, ADTRAN,
        (ADTRAN_MAX_TS/2), INT_LPBK))
        return (TO_HOST_SUB_T3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, VERILINK,
        (VERILINK_MAX_TS/2), INT_LPBK))
        return (TO_HOST_SUB_T3_LPBK_TEST_FAIL);

    return (TO_HOST_SUB_T3_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_t3_ext_lpbk_test
 *
 * This function performs T3 subrates external loopback.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_ext_lpbk_test(void)
{
    printf("\npatriot_clear_t3_subrate_ext_lpbk_test\n");
    if (patriot_clear_te3_subrate_test(MODE_T3, KENTROX,
        patriot_t3_ken_tbl[KENTROX_10K], EXT_LPBK))
        return (TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, DIGITAL_LINK,
        (DIG_LINK_MAX_TS/2), EXT_LPBK))
        return (TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, LARSCOM,
        (LARSCOM_MAX_TS/2), EXT_LPBK))
        return (TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, ADTRAN,
        (ADTRAN_MAX_TS/2), EXT_LPBK))
        return (TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_T3, VERILINK,
        (VERILINK_MAX_TS/2), EXT_LPBK))
        return (TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL);

    return (TO_HOST_SUB_T3_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_t3_individual_int_lpbk_test
 *
 * This function tests T3 subrates. (Internal Loopback)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_individual_int_lpbk_test(void)
{
    static ulong i = 0;

    printf("\npatriot_subrate_t3_individual_int_lpbk_test\n");

    if (param_cmd_menu == PATRIOT_MENU) {
		i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Larscom,"
							  "      3-Adtran, 4-Verilink", 0, 0, 0x4);
    } else {
    	i = param_arr[2];
    }
    switch(i) {
    case 0:
        if (patriot_clear_t3_kentrox_lpbk(INT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL);
        }
        break;
    case 1:
        if (patriot_clear_t3_dig_link_lpbk(INT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL);
        }
        break;
    case 2:
        if (patriot_clear_t3_larscom_lpbk(INT_LPBK)) {
			return (TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL);
		}
        break;
    case 3:
        if (patriot_clear_t3_adtran_lpbk(INT_LPBK)) {
			return (TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL);
		}
        break;
    case 4:
        if (patriot_clear_t3_verilink_lpbk(INT_LPBK)) {
			return (TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL);
		}
        break;
	}

    return (TO_HOST_SUB_T3_IND_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_t3_individual_ext_lpbk_test
 *
 * This function tests T3 subrates. (External Loopback)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_individual_ext_lpbk_test(void)
{
    static ulong i = 0;

    printf("\npatriot_subrate_t3_individual_ext_lpbk_test\n");


    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Larscom,"
                              "      3-Adtran, 4-Verilink", 0, 0, 0x4);
    } else {
    	i = param_arr[2];
    }

    switch(i) {
    case 0:
        if (patriot_clear_t3_kentrox_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 1:
        if (patriot_clear_t3_dig_link_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 2:
        if (patriot_clear_t3_larscom_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 3:
        if (patriot_clear_t3_adtran_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 4:
        if (patriot_clear_t3_verilink_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    }

    return (TO_HOST_SUB_T3_IND_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_clear_e3_lpbk_test
 *
 * This function is a wrapper for Clear E3 internal loopbacks.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_e3_int_lpbk_test(void)
{

    printf("\npatriot_clear_e3_int_lpbk_test\n");
    if(patriot_clear_te3_test(MODE_E3, INT_LPBK)) {
        return (TO_HOST_CLR_E3_LPBK_TEST_FAIL);
    }
    printf("\npatriot_clear_e3_int_lpbk_test completed\n");fflush(0);
    return (TO_HOST_CLR_E3_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_clear_e3_ext_lpbk_test
 *
 * This function is a wrapper for Clear E3 external loopbacks.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_e3_ext_lpbk_test(void)
{

    printf("\npatriot_clear_e3_ext_lpbk_test\n");
    if(patriot_clear_te3_test(MODE_E3, EXT_LPBK)) {
        return (TO_HOST_CLR_E3_EX_LPBK_TEST_FAIL);
    }
    printf("\npatriot_clear_e3_ext_lpbk_test completed\n");fflush(0);
    return (TO_HOST_CLR_E3_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_e3_int_lpbk_test
 *
 * This function performs E3 subrates internal loopback.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_int_lpbk_test(void)
{
    printf("\npatriot_clear_e3_subrate_int_lpbk_test\n");
    if (patriot_clear_te3_subrate_test(MODE_E3, KENTROX,
        (patriot_e3_ken_tbl[KENTROX_10K]), INT_LPBK))
        return (TO_HOST_SUB_E3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_E3, DIGITAL_LINK,
        (DIG_LINK_MAX_TS/2), INT_LPBK))
        return (TO_HOST_SUB_E3_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_E3, UNFRM_E3, 0, INT_LPBK))
        return (TO_HOST_SUB_E3_LPBK_TEST_FAIL);

    return (TO_HOST_SUB_E3_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_e3_ext_lpbk_test
 *
 * This function performs E3 subrates external loopback.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_ext_lpbk_test(void)
{
    printf("\npatriot_clear_e3_subrate_ext_lpbk_test\n");
    if (patriot_clear_te3_subrate_test(MODE_E3, KENTROX,
        (patriot_e3_ken_tbl[KENTROX_10K]), EXT_LPBK))
        return (TO_HOST_SUB_E3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_E3, DIGITAL_LINK,
        (DIG_LINK_MAX_TS/2), EXT_LPBK))
        return (TO_HOST_SUB_E3_EX_LPBK_TEST_FAIL);

    if (patriot_clear_te3_subrate_test(MODE_E3, UNFRM_E3, 0, EXT_LPBK))
        return (TO_HOST_SUB_E3_EX_LPBK_TEST_FAIL);

    return (TO_HOST_SUB_E3_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_e3_individual_int_lpbk_test
 *
 * This function tests E3 subrates. In continuous mode, it only prompts
 * for the input once. (Internal Loopback)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_individual_int_lpbk_test(void)
{

    static uchar i = 0;

    printf("\npatriot_subrate_e3_individual_int_lpbk_test\n");

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Unframed", 0,
                               0, 0x2);
    } else {
    	i = param_arr[2];
    }
    switch(i) {
    case 0:
        if (patriot_clear_e3_kentrox_lpbk(INT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_LPBK_TEST_FAIL);
        }
        break;
    case 1:
        if (patriot_clear_e3_dig_link_lpbk(INT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_LPBK_TEST_FAIL);
        }
        break;
    case 2:
        if (patriot_clear_te3_subrate_test(MODE_E3, UNFRM_E3,
                                           0, INT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_LPBK_TEST_FAIL);
        }
        break;
    }

    return (TO_HOST_SUB_E3_IND_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_subrate_e3_individual_ext_lpbk_test
 *
 * This function tests E3 subrates. In continuous mode, it only prompts
 * for the input once. (External Loopback)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_individual_ext_lpbk_test(void)
{

    static uchar i = 0;

    printf("\npatriot_subrate_e3_individual_ext_lpbk_test\n");

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Unframed", 0,
                               0, 0x2);
    } else {
    	i = param_arr[2];
    }
    switch(i) {
    case 0:
        if (patriot_clear_e3_kentrox_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 1:
        if (patriot_clear_e3_dig_link_lpbk(EXT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    case 2:
        if (patriot_clear_te3_subrate_test(MODE_E3, UNFRM_E3,
                                           0, EXT_LPBK)) {
            return (TO_HOST_SUB_E3_IND_EX_LPBK_TEST_FAIL);
        }
        break;
    }

    return (TO_HOST_SUB_E3_IND_EX_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: check_status
 *
 * This function check status of ds3170 receiver reg for ex. OOF (out of frame)
 *
 * Input : Mode - T3 / E3
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
check_status(uchar mode)
{
    uchar statuslh, statusll = 0;
    uchar maskh, maskl = 0;
    uchar statush, statusl = 0;

    if (mode == MODE_T3) {
        /* Read the latched status */
        if (ds3170_read(&statuslh, T3_RSRL1_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSRL1_ADDR_H fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (ds3170_read(&statusll, T3_RSRL1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSRL1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        /* Read the realtime status */
        if (ds3170_read(&statush, T3_RSR1_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSR1_ADDR_H fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (ds3170_read(&statusl, T3_RSR1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSR1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        /* Read the mask */
        if (ds3170_read(&maskh, T3_RSRIE1_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSRIE1_ADDR_H fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        if (ds3170_read(&maskl, T3_RSRIE1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read T3_RSRIE1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        /* check only unmasked bits */
        statuslh &= maskh;
        statusll &= maskl;

        /* Check OOFL latched status register */
        if (statusll & T3_RSRL1_OOFL) {
            /* When 0, the rcv framer is not in an out of frame (OOF) condition.
             * When 1, the rcv framer is in an out of frame (OOF) condition.*/
            if (statusl & T3_RSR1_OOF) {
                sprintf(err_msg, "\n%s, [#%d]:T3 is in OOF condition\n"
                		, __FUNCTION__, __LINE__);
                print_err(FALSE, err_msg, LVL_1);
                return (FAILED);
            }
        }

        /* clear the latched status */
        if (ds3170_write(statuslh, T3_RSRL1_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:write T3_RSRL1_ADDR_H fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        if (ds3170_write(statusll, T3_RSRL1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:write T3_RSRL1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

    } else { /* E3 G751*/
        /* Read the latched status */
        if (ds3170_read(&statusll, G751_RSRL1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read G751_RSRL1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        /* Read the realtime status */
        if (ds3170_read(&statusl, G751_RSR1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read G751_RSR1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        /* Read the mask */
        if (ds3170_read(&maskl, G751_RSRIE1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:read G751_RSRIE1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

        /* check only unmasked bits */
        statusll &= maskl;

        /* Check OOFL latched status register */
        if (statusll & G751_RSRL1_OOFL) {
            /* When 0, the rcv framer is not in an out of frame (OOF) condition.
             * When 1, the rcv framer is in an out of frame (OOF) condition.*/
            if (statusl & G751_RSR1_OOF) {
                sprintf(err_msg, "\n%s, [#%d]: E3G751 is in OOF condition\n"
                		, __FUNCTION__, __LINE__);
                print_err(FALSE, err_msg, LVL_1);
                return (FAILED);
            }
        }

        /* clear the latched status */
        if (ds3170_write(statusll, G751_RSRL1_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:write G751_RSRL1_ADDR_L fail.\n"
            		, __FUNCTION__, __LINE__);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }

    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_te3_test
 *
 * This function tests the Clear T3 or E3 loopbacks. It performs a framer's
 * loopback, a LIU loopback, and an external connector loopbacks.
 *
 * Input : mode - T3 or E3 mode
 *         lpbk_mode - Internal or External lpbk
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int
patriot_clear_te3_test(uchar mode, uchar lpbk_mode)
{
    uchar str[20], *str_p;
    uchar temp = 0;
    uchar bypass_subrate = BYPASS_SUB;
    uchar bypass_fpga = FALSE;

    str_p = (uchar *)str;

    printf("\npatriot_clear_te3_test\n");

    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    /* configure DS3170 framer to enable loopback in mode E3 / T3 */
    if (patriot_conf_ds3170_frmr(CR4_LBM(7), mode, lpbk_mode, bypass_subrate,
    		bypass_fpga)) {
        sprintf(err_msg, "\n%s, [#%d]:patriot_conf_ds3170_frmr(), fail.\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    if (mode == MODE_T3) {
        if (patriot_t3_set_subrate(CLEAR, 0)) {
            sprintf(err_msg, "\n%s, [#%d]:t3 set subrate fail\n",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        str_p = "T3";
    } else {
        if (patriot_e3_set_subrate(CLEAR, 0)) {
            sprintf(err_msg, "\n%s, [#%d]:e3 set subrate fail\n",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        str_p = "E3";
    }

    printf("Clear %s DS3170 Framer", str_p);
 
    if (lpbk_mode == EXT_LPBK) {
         /* Disable Analog Loopback
          * set external loopbacks
          */
        printf(" External loopback\n");
        if(ds3170_read(&temp, CR4_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:TE3 Test read reg CR4_ADDR_H fail\n",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return(FAILED);
        }

        temp &= ~CR4_LBM(6);
        temp &= ~CR4_LBM(1);

        if (ds3170_write(temp, CR4_ADDR_H)) {
        	sprintf(err_msg, "\n%s, [#%d]:TE3 Ext. Lpbk Test write reg CR4_ADDR_H fail\n",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, CR4_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read loopback mode reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:CR4_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
        /* execute External DS3170 tdm loopback */
        if (patriot_ucc_lpbk_test(PATRIOT_UCC_PASS)) {
            sprintf(err_msg, "\n%s, [#%d]:DS3170 external loopback fail",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
    } else {
        printf(" Internal loopback\n");
        /* set internal Loopbacks
         * 0x001 = Enable Analog Loopback
         */
        if(ds3170_read(&temp, CR4_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:TE3 Internal Lpbk Test read "
            		"reg CR4_ADDR_H fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return(FAILED);
        }

        temp &= ~CR4_LBM(6);
        temp |= CR4_LBM(1);
        if (ds3170_write(temp, CR4_ADDR_H)) {
            sprintf(err_msg, "\n%s, [#%d]:TE3 Test write "
            		"reg CR4_ADDR_H fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, CR4_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read loopback mode reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:CR4_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif

        /* execute internal DS3170 tdm loopback */
        if (patriot_ucc_lpbk_test(PATRIOT_UCC_PASS)) {
            sprintf(err_msg, "\n%s, [#%d]:DS3170 internal loopback fail",
            		__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return  (FAILED);
        }

    }

    /* Check OOFL status if there any symptoms Out of Frame */
    if (check_status(mode)) {
        sprintf(err_msg, "\n%s, [#%d]:Found out Out Of Frame\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_te3_subrate_test
 *
 * This function tests the subrates for the specified mode (T3 or E3).
 *
 * Input : mode - T3 or E3
 *         type - type of the subrates
 *         rate - rate
 *         lpbk_mode - Internal or External loopback mode
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int
patriot_clear_te3_subrate_test(uchar mode, uchar type, ulong rate,
                               uchar lpbk_mode)
{
    uchar str[20], *str_p;
    uchar temp = 0;
    uchar use_subrate = USE_SUB;
    uchar bypass_fpga = FALSE;
    uchar buf = 0;

    printf("\npatriot_ds3170_clear_te3_subrate_test\n");

    str_p = (uchar *)str;
    str_p += sprintf(str_p,"%s", (mode == MODE_T3) ? "T3 " : "E3 ");
    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n",__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    /* Configure DS3170 mode T3 / E3 */
    if (patriot_conf_ds3170_frmr(CR4_LBM(7), mode, lpbk_mode, use_subrate,
    		bypass_fpga)) {
        sprintf(err_msg, "\n%s, [#%d]:patriot_conf_ds3170_frmr(), fail.\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    switch (type) {
    case DIGITAL_LINK:
        str_p += sprintf(str_p, "Digital Link");
        break;
    case KENTROX:
        str_p += sprintf(str_p, "Kentrox");
        break;
    case LARSCOM:
        str_p += sprintf(str_p, "Larscom");
        break;
    case ADTRAN:
        str_p += sprintf(str_p, "Adtran");
        break;
    case VERILINK:
        str_p += sprintf(str_p, "Verilink");
        break;
    case CLEAR:
    default:
        type = CLEAR;
        str_p += sprintf(str_p, "Clear");
        break;
    }

    if ((mode == MODE_E3) && (type == UNFRM_E3)) {
        /*
         * enable E3 Tx Pass Through
         */
        str_p = (uchar *)str;
        sprintf(str_p, "E3 Clear Unframed");

	/* Set Framer mode for E3 UN-FRAME  */
	if (ds3170_read(&temp, CR2_ADDR_L)) {
	    sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n"
	    		,__FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    return (FAILED);
	}
	temp |= CR2_FM(0x6);
	
	if(ds3170_write(temp, CR2_ADDR_L)) {
	    sprintf(err_msg, "\n%s, [#%d]:Config Framer mode E3 G751, "
	    		"CR2_ADDR_H Reg write fail\n",__FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    return (FAILED);
	}
	
        if (ds3170_read(&temp, G751_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, "
            		"write TFGD reg fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        temp |= G751_TCR_TFGD;
        if (ds3170_write(temp, G751_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, "
            		"write TFGD reg fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
    }

    printf("\n %s Test\n", str);

    /*
     * Configure the subrate
     */
    if (mode == MODE_T3) {
        /*
         * Assert reset_n to the low before set mode t3/e3 mode subrate and
         * bandwidth selection register
         */
        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        buf &= (~T3_SUBRATE_BLOCK_RST);

        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Write reset_n = 0, "
            		"write fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        /* Verify the reset_n = 0
         */
        buf &= (~T3_SUBRATE_BLOCK_RST);
        printf("\nBefore T3 set the mode reset_n = %x\n", buf);
        if (buf != T3_SUBRATE_BLOCK_RST)
        	printf("\n T3 reset_n is low \n");

        if (patriot_t3_set_subrate(type, rate)) {
            sprintf(err_msg, "\n%s, [#%d]:t3 set subrate fail\n"
            		,__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        /*
         * Assert reset_n to the high after set mode t3/e3 mode subrate and
         * bandwidth selection register
         */
        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        buf |= T3_SUBRATE_BLOCK_RST;

        if (p1021_i2c_write_fpga_byte (T3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 Write reset_n = 0, "
            		"write fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        if (p1021_i2c_read_fpga_byte (T3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg,"\n%s, [#%d]:T3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        /* Verify the reset_n = 1
         */
        buf &= T3_SUBRATE_BLOCK_RST;
        printf("\nAfter T3 set the mode reset_n = %x\n", buf);
        if (buf != T3_SUBRATE_BLOCK_RST) {
        	printf("\n T3 reset_n is not high \n");
        }
    } else {
        /*
         * Assert reset_n to the low before set mode t3/e3 mode subrate and
         * bandwidth selection register
         */
        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        buf &= (~E3_SUBRATE_BLOCK_RST);

        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Write reset_n = 0, "
            		"write fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        /* Verify the reset_n = 0
         */
        buf &= (~E3_SUBRATE_BLOCK_RST);
        printf("\nBefore E3 set the mode reset_n = %x\n", buf);
        if (buf != E3_SUBRATE_BLOCK_RST)
        	printf("\n E3 reset_n is not low \n");
        if (patriot_e3_set_subrate(type, rate)) {
            sprintf(err_msg, "\n%s, [#%d]:e3 set subrate fail\n"
            		,__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        /*
         * Assert reset_n to the high after set mode t3/e3 mode subrate and
         * bandwidth selection register
         */
        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        buf |= E3_SUBRATE_BLOCK_RST;

        if (p1021_i2c_write_fpga_byte (E3_SUBRATE_MODE_SEL_REG, buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Write reset_n = 0, "
            		"write fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        if (p1021_i2c_read_fpga_byte (E3_SUBRATE_MODE_SEL_REG, &buf)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 Read reset_n = 0, "
            		"read fpga fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
        /* Verify the reset_n = 1
         */
        buf &= E3_SUBRATE_BLOCK_RST;
        printf("\nAfter E3 set the mode reset_n = %x\n", buf);
        if (buf != E3_SUBRATE_BLOCK_RST) {
        	printf("\n E3 reset_n is not high \n");
        }
    }

    /* Delay of 200 usec before enabling the ucc */
    msleep(200);

    /* Enable the frame sync RSOFO
     * Enable TCLKO signal to send out the RX clock back to FPGA as non gapped */
    if(ds3170_read(&temp, CR3_ADDR_H)) {
		sprintf(err_msg, "\n%s, [#%d]:TE3 Test read reg enable RDEN fail\n"
				,__FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
		return (FAILED);
	}
    /*Table 10-22 TCLKO/TGCLK Output Pin Functions */
    temp |= CR3_RCLKS; /* select RCLKS signal */
    temp |= CR3_RSOFOS; /* select RSOFO signal */
    temp |= CR3_TCLKS;   /* select TCLKO signal */
    temp |= CR3_TSOFOS; /* select TSOFOS signal */
    
    if(ds3170_write(temp, CR3_ADDR_H)) {
	sprintf(err_msg, "\n%s, [#%d]:TE3 Test write reg enable RDEN fail\n"
			,__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (FAILED);
    }
#ifdef DEBUG
    if (ds3170_read(&temp, CR3_ADDR_H)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer RDEN reg fail\n");
        return (FAILED);
    }
    printf("\n%s, [#%d]:CR3_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    /* Enable the P8KRS to internal tx framer clock */
    if(ds3170_read(&temp, CR3_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:TE3 Test read reg P8KRS fail\n"
				,__FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
		return (FAILED);
	}

    temp |= CR3_P8KRS(0x3);  /* Enable Internal tx framer clock */

    if(ds3170_write(temp, CR3_ADDR_L)) {
	sprintf(err_msg, "\n%s, [#%d]:TE3 Test write reg P8KRS fail\n"
			,__FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
	return (FAILED);
    }
#ifdef DEBUG
    if (ds3170_read(&temp, CR3_ADDR_L)) {
        printf("\npatriot_conf_ds3170_frmr(), "
                 "config framer P8KRS reg fail\n");
        return (FAILED);
    }
    printf("\n%s, [#%d]:CR3_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

    if (lpbk_mode == EXT_LPBK) {
        printf("\nRun External Loopback Test\n");
        /* disable analog loopback for external loopback */
        if(ds3170_read(&temp, CR4_ADDR_H)) {
            printf("\nTE3 Test read reg disable lpbk fail\n");
            return (FAILED);
        }

        temp &= ~CR4_LBM(6);
        temp &= ~CR4_LBM(1);

        if(ds3170_write(temp, CR4_ADDR_H)) {
            printf("\nTE3 Test write reg disable lpbk fail\n");
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, CR4_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read loopback mode reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:CR4_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif

    } else {
        printf("\nRun Internal Loopback Test\n");
        /* set internal Loopbacks by remove any loopbacks
         * enable analog loopback
         */
        if(ds3170_read(&temp, CR4_ADDR_H)) {
            printf("\nTE3 Test read reg internal lpbk fail\n");
            return (FAILED);
        }
        temp &= ~CR4_LBM(6);
        temp |= CR4_LBM(1);
        if(ds3170_write(temp, CR4_ADDR_H)) {
            printf("\nTE3 Test write reg internal lpbk fail\n");
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, CR4_ADDR_H)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "config framer read loopback mode reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:CR4_ADDR_H contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
    }

    if ((mode == MODE_E3) || (type == UNFRM_E3)) {
    /* Transmit Frame Generation is enable */
        if (ds3170_read(&temp, G751_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, read TFGD reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        temp &= ~G751_TCR_TFGD;

        if (ds3170_write(temp, G751_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, write TFGD reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, G751_TCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "read config framer tx frame generate reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:G751_TCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
		/* Enable the Framer sync in the RX direction */
        if (ds3170_read(&temp, G751_RCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, read FRSYNC reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        temp |= G751_RCR_FRSYNC;
        if (ds3170_write(temp, G751_RCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:E3 subrate test, write FRSYNC reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, G751_RCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "read config framer rx force resync reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:G751_RCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
    } else if (mode == MODE_T3) {
        /* Transmit Frame Generation is enable */
        if (ds3170_read(&temp, T3_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 subrate test, read TFGD reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        temp &= ~T3_TCR_TFGD;

        if (ds3170_write(temp, T3_TCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 subrate test, write TFGD reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, T3_TCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "read config framer tx frame generate reg fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:T3_TCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
		/* Enable the Framer sync in the RX direction */
        if (ds3170_read(&temp, T3_RCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 subrate test, read FRSYNC reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }

        temp |= T3_RCR_FRSYNC;
        if (ds3170_write(temp, T3_RCR_ADDR_L)) {
            sprintf(err_msg, "\n%s, [#%d]:T3 subrate test, write FRSYNC reg"
            		" fail\n",__FUNCTION__, __LINE__);
            print_err(TRUE, err_msg, LVL_0);
            return (FAILED);
        }
#ifdef DEBUG
	if (ds3170_read(&temp, T3_RCR_ADDR_L)) {
	    printf("\npatriot_conf_ds3170_frmr(), "
		   "read config framer rx force framer resync fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]:T3_RCR_ADDR_L contents = 0x%02x", __FUNCTION__,
	       __LINE__, temp);
#endif
    }
    /* Check OOFL status if there any symptoms Out of Frame */
    if (check_status(mode)) {
        sprintf(err_msg, "\n%s, [#%d]:Found out OOF\n",__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    /* Delay of 200 usec before execute the loopback */
    msleep(200);

    /* execute internal DS3170 tdm loopback */
    if (patriot_ucc_lpbk_test(PATRIOT_UCC_PASS)) {
        sprintf(err_msg, "\n%s, [#%d]:DS3170 %s loopback fail"
		,__FUNCTION__, __LINE__,
		lpbk_mode == EXT_LPBK ? "external":"internal");
        print_err(TRUE, err_msg, LVL_0);
        return(FAILED);
    }

    /* Check OOFL status if there any symptoms Out of Frame */
    if (check_status(mode)) {
        sprintf(err_msg, "\n%s, [#%d]:Found out OOF\n",__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_kentrox_lpbk
 *
 * This function tests T3 kentrox subrates. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_t3_kentrox_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   1-1.5K, 2-2K, ..., 11-9.5K\n"
           "     12-10K, 13-10.5K, 14-11K, ..., 25-19.5K\n"
           "     26-20K, 27-20.5K, 28-21K, ..., 39-29.5K\n"
           "     3A-30K, 3B-30.5K, 3C-31K, ..., 44-35K\n");
    if (param_cmd_menu == PATRIOT_MENU) {
    	i = gethex_answer("Select T3 rate: ", 0x12, 1, 0x44);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, KENTROX,
                                (patriot_t3_ken_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_e3_kentrox_lpbk
 *
 * This function tests E3 kentrox subrates. In continuous mode, it only
 *  prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_e3_kentrox_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n    0-1K, 1-1.5K, 2-2K, ..., 11-9.5K\n"
           "     12-10K, 13-10.5K, 14-11K, ..., 25-19.5K\n"
           "     26-20K, 27-20.5K, 28-21K, ..., 2F-24.5K\n");
    if (param_cmd_menu == PATRIOT_MENU) {
    	i = gethex_answer("Select E3 rate: ", 0x12, 0, 0x2F);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_E3, KENTROX,
                                       (patriot_e3_ken_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_fs_ucc_lpbk_test
 *
 * This function tests HDLC loopbacks
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fs_ucc_lpbk_test(void)
{
    if (patriot_ucc_lpbk_test(PATRIOT_UCC_LPBK))
        return (TO_HOST_FREESCALE_UCC_LPBK_TEST_FAIL);
    printf("\npatriot_fs_ucc_lpbk_test completed\n");fflush(0);
    return (TO_HOST_FREESCALE_UCC_LPBK_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_ucc_lpbk_test
 *
 * This function tests HDLC external loopbacks
 *
 * Input : lpbk_op - TDM loopback or pass through
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ucc_lpbk_test(int lpbk_op)
{
    char       base_val, inc_val;
    uchar data;
    uchar *datap;
    unsigned int tx_hdlc_buf, rx_hdlc_buf, i;
    unsigned int tx_hdlc_buf_phy, rx_hdlc_buf_phy;
    int frame_num, frm_size, count;
    unsigned short pak_size[10] = {64, 108, 512, 256,
    			   1800, 65, 1511, 128, 66, 719};
    //unsigned short pak_size[1] = {64};
    
    unsigned long vir_ptr, phy_ptr;

    for (frame_num = 0; frame_num < 10; frame_num++) {
    //for (frame_num = 0; frame_num < 1; frame_num++) {
    printf("\nframe_num = %d\n", frame_num);

        frm_size = pak_size[frame_num];
        if (frm_size & 1) {
	    base_val = 0xff;
            inc_val = -1;
        } else {
	    base_val = 0;
            inc_val = 1;
	    
        }
        data = base_val;
	
	/* malloc_nm need to have 4K aligned */
        tx_hdlc_buf = (unsigned int)malloc_nm(4096*20);
	tx_hdlc_buf_phy = (unsigned int)phy_addr(tx_hdlc_buf);

	/* malloc_nm need to have 4K aligned */
        rx_hdlc_buf = (unsigned int)malloc_nm(4096*20);
	rx_hdlc_buf_phy = (unsigned int)phy_addr(rx_hdlc_buf);
#ifdef DEBUG
	printf("\ntx_hdlc_buf = 0x%08x", tx_hdlc_buf);
	printf("\ntx_hdlc_buf_phy = 0x%08x", tx_hdlc_buf_phy);
	printf("\nrx_hdlc_buf = 0x%08x", rx_hdlc_buf);
	printf("\nrx_hdlc_buf_phy = 0x%08x", rx_hdlc_buf_phy);
#endif
        memset((uchar *)tx_hdlc_buf, 0, frm_size);
        memset((uchar *)rx_hdlc_buf, 0, frm_size);

        datap = (uchar *)tx_hdlc_buf;
        for (count = 0; count < frm_size; count++) {
            *datap++ = data;
            data += inc_val;
        }
	if (hdlc_send_and_receive_data(frame_num, frm_size, (uchar *)tx_hdlc_buf,
				       (uchar *)rx_hdlc_buf,
				       (uchar *)tx_hdlc_buf_phy,
				       (uchar *)rx_hdlc_buf_phy,
				       lpbk_op)) {
	    free_nm((void *)tx_hdlc_buf);
	    free_nm((void *)rx_hdlc_buf);
	    return (FAILED);
	}
        free_nm((void *)tx_hdlc_buf);
        free_nm((void *)rx_hdlc_buf);
    }
        
    return (PASSED);
    
}


/**********************************************************************
 *
 * Function: reset_fpga_gpio_framer_bit
 *
 * This function reset the FPGA GPIO and Framer GPIO bit
 *
 * Input : Reset module FPGA GPIO or FRAMER GPIO
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int reset_fpga_gpio_framer_bit(uchar reset_module)
{
    uchar rd_data = 0;
    uchar reset_value = 0x00;
    
    if(reset_module == FPGA_GPIO) {
	if (p1021_i2c_write_fpga_byte (GPIO_FRAMER_OE_REG, reset_value)) {
	    sprintf(err_msg, "\n%s, [#%d]:Write fail offset @%#x = %#x\n"
	    		, __FUNCTION__, __LINE__, GPIO_FRAMER_OE_REG,
		   reset_value);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);
#ifdef DEBUG
	if (p1021_i2c_read_fpga_byte (GPIO_FRAMER_OE_REG, &rd_data)) {
	    printf("\n Read fail offset @%#x = %#x\n", GPIO_FRAMER_OE_REG,
		   rd_data);
	    return (FAILED);
	}
	
	printf("\n%s, [#%d]: Reset Framer GPIO (0x%02x) contents = 0x%02x",
	       __FUNCTION__, __LINE__, GPIO_FRAMER_OE_REG, rd_data);
	
	if (p1021_i2c_read_fpga_byte (GPIO_FRAMER_REG, &rd_data)) {
	    printf("\n Read fail offset @%#x = %#x\n", GPIO_FRAMER_REG, rd_data);
	    return (FAILED);
	}
	
	printf("\n%s, [#%d]: Reset Framer GPIO (0x%02x) contents = 0x%02x",
	       __FUNCTION__,__LINE__, GPIO_FRAMER_REG, rd_data);
#endif
    } else {
	/* Refer to DS3170 page 129 GIOCR to output logic 0 */
	if (ds3170_write(reset_value, GIOCR_ADDR_L)) {
	    sprintf(err_msg, "\n%s, [#%d]:GIOCR_ADDR_L Reg write fail\n"
	    		, __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);
	if (ds3170_write(reset_value, GIOCR_ADDR_H)) {
	    sprintf(err_msg, "\n%s, [#%d]:GIOCR_ADDR_H Reg write fail\n"
	    		, __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	msleep(20);
#ifdef DEBUG
	if (ds3170_read(&rd_data, GIORR_ADDR_L)) {
	    printf("\n GIORR_ADDR_L Reg write fail\n");
	    return (FAILED);
	}
	printf("\n%s, [#%d]: Reset value GIORR_ADDR_L contents = 0x%02x",
	       __FUNCTION__, __LINE__, rd_data);
#endif
    }
    
    return (PASSED);
}



/**********************************************************************
 *
 * Function: patriot_test_fpga_gpio_framer
 *
 * This function tests FPGA GPIO and Framer GPIO
 * Notes: A. FPGA test GPIO on Framer
 *           1. Walking's 1 test by write 0x01 to FPGA Framer GPIO OE register
 *           (0x3)
 *           2. Walking's 0 test by write ~0x01 to FPGA Framer GPIO register (0x2)
 *           3. Read back the Framer GPIO read register (0x1c) to verify value.
 *        B. Framer test GPIO on FPGA
 *           1. Walking's 10 to Framer GPIO control register (0x0A ~ 0x0B)
 *           2. Read back the Framer GPIO read register (0x1c) to verify value.
 *           3. Read back the FPGA Framer GPIO register (0x2) to verify value.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_test_fpga_gpio_framer(void)
{
    uchar wr_data = 0;
    uchar rd_data = 0;
    uchar masked_wr_data = 0;
    uchar invert = 0;
    uint mask = 0xFF;
    uchar gpio8 = 0;
    int ix = 0;

    /* Reset FPGA and init the i2c controller */
    patriot_fpga_reset();
    msleep(200);
    platform_cpu_i2c_init();
    msleep(100);
    
    /* Initialize ESPI */
    if (ds3170_init_espi()) {
        sprintf(err_msg, "\n%s, [#%d]:Initial fail\n", __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    /* Reset then take it out of reset */
    patriot_ds3170_reset();
    msleep(100);
    /* Shutdown PMUXCR bit 2 to zero to become GPIO instead of SDHC_WP */
    REGB->im_gur.pmuxcr &= (~0x20000000);

    /* Walking's 1 testing FPGA GPIO to FRAMER GPIO */
    printf("\n Testing FPGA GPIO to FRAMER GPIO start \n");

    wr_data = 0x01;
    for (ix=1; ix<=8; ix++) {
    	masked_wr_data = wr_data & mask;
    	if (masked_wr_data) {
    	    /* Offset FPGA Framer GPIO OE register 0x03 */
    	    if (p1021_i2c_write_fpga_byte (GPIO_FRAMER_OE_REG, masked_wr_data)) {
    	        sprintf(err_msg, "\n%s, [#%d]:Write fail offset @%#x = %#x\n"
    	        		, __FUNCTION__, __LINE__, GPIO_FRAMER_OE_REG,
		       masked_wr_data);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
    	    msleep(20);

    	    if (p1021_i2c_read_fpga_byte (GPIO_FRAMER_OE_REG, &rd_data)) {
    	        sprintf(err_msg, "\n%s, [#%d]:Read fail offset @%#x = %#x\n"
    	        		, __FUNCTION__, __LINE__, GPIO_FRAMER_OE_REG,
		       rd_data);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    rd_data &= mask;
#ifdef DEBUG
    	    printf("\n%s, [#%d]: Framer GPIO OE (0x03) contents = 0x%02x",
		   __FUNCTION__,__LINE__, rd_data);
#endif

    	    if(rd_data != masked_wr_data) {
    	    	sprintf(err_msg, "\n%s, [#%d]:FPGA(0x03)Compare value mismatch"
    	    			" !! current value 0x%02x, Expected 0x%02x\n"
    	    			, __FUNCTION__, __LINE__, rd_data, masked_wr_data);
    	    	print_err(TRUE, err_msg, LVL_0);
    	    	return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    /* write buf to the FPGA Framer GPIO register 0x02 */
    	    invert = ~masked_wr_data;

    	    if (p1021_i2c_read_fpga_byte (GPIO_FRAMER_REG, &rd_data)) {
    	        sprintf(err_msg, "\n%s, [#%d]:Read fail offset @%#x = %#x\n"
    	        		, __FUNCTION__, __LINE__, GPIO_FRAMER_REG,
		       rd_data);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    rd_data &= mask;
#ifdef DEBUG
    	    printf("\n%s, [#%d]: Framer GPIO (0x02) contents = 0x%02x",
		   __FUNCTION__,__LINE__, rd_data);
#endif

    	    if(rd_data != invert) {
    	    	sprintf(err_msg, "\n%s, [#%d]:FPGA(0x02)Compare value mismatch"
    	    			" !! current value 0x%02x, Expected 0x%02x\n"
    	    			, __FUNCTION__, __LINE__, rd_data, invert);
    	    	print_err(TRUE, err_msg, LVL_0);
    	    	return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    /* Verify the GPIO on Framer register 0x1C */
    	    if (ds3170_read(&rd_data, GIORR_ADDR_L)) {
    	        sprintf(err_msg, "\n%s, [#%d]:GIORR_ADDR_L Reg read fail\n"
    	        		, __FUNCTION__, __LINE__);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    rd_data &= mask;
#ifdef DEBUG
    	    printf("\n%s, [#%d]: GIORR_ADDR_L contents = 0x%02x", __FUNCTION__,
    		   __LINE__, rd_data);
#endif

    	    if(rd_data != invert) {
    	    	sprintf(err_msg, "\n%s, [#%d]:Framer(0x1C)Compare value "
    	    			"mismatch !! current value 0x%02x, Expected 0x%02x\n"
    	    			, __FUNCTION__, __LINE__, rd_data, invert);
    	    	print_err(TRUE, err_msg, LVL_0);
    	    	return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
    	}
    	wr_data = wr_data << 1;

        /* Reset the FPGA GPIO bit */
    	if (reset_fpga_gpio_framer_bit(FPGA_GPIO)== FAILED ) {
	    sprintf(err_msg, "\n%s, [#%d]:Reset FPGA GPIO bit fail \n"
	    		, __FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	}
    }
    printf("\n Testing FPGA GPIO to FRAMER GPIO end \n");

    printf("\n Testing FRAMER GPIO to FPGA GPIO start \n");
    /* Walking's 10 testing FRAMER GPIO to FPGA GPIO */
    wr_data = 0x02;
    gpio8 = 0x11;
    for (ix=1; ix<=4; ix++) {
    	masked_wr_data = wr_data & mask;
    	if (masked_wr_data) {
    	    /* Walking's 10 to Framer's GPIO control register 0x0A and 0x0B */
    	    if (ds3170_write(masked_wr_data, GIOCR_ADDR_L)) {
    	        sprintf(err_msg, "\n%s, [#%d]:GIOCR_ADDR_L Reg write fail\n"
    	        		, __FUNCTION__, __LINE__);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    if (ds3170_write(masked_wr_data, GIOCR_ADDR_H)) {
    	        sprintf(err_msg, "\n%s, [#%d]:GIOCR_ADDR_H Reg write fail\n"
    	        		, __FUNCTION__, __LINE__);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }

    	    /* Verify value on Framers's GPIO Read register 0x1C */
    	    if (ds3170_read(&rd_data, GIORR_ADDR_L)) {
    	        sprintf(err_msg, "\n%s, [#%d]:GIORR_ADDR_L Reg write fail\n"
    	        		, __FUNCTION__, __LINE__);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
#ifdef DEBUG
    	    printf("\n%s, [#%d]: GIORR_ADDR_L contents = 0x%02x", __FUNCTION__,
    		   __LINE__, rd_data);
#endif
    	    /* gpio8 = 0x11  ( 0001 0001 )
    	     * rd_data = ~ 1110 1110
    	     * rd_data = rd_data & mask (0xFF)
    	     * rd_data = 0x11
    	     */
            rd_data = ~rd_data;
    	    rd_data &= mask;
    	    if(rd_data != gpio8) {
    	    	sprintf(err_msg, "\n%s, [#%d]:Framer(0x1C)Compare value "
    	    			"mismatch !! current value 0x%02x, Expected 0x%02x\n"
    	    			, __FUNCTION__, __LINE__, rd_data, gpio8);
    	    	print_err(TRUE, err_msg, LVL_0);
    	    	return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
	    
    	    /* Verify value on FPGA Framer GPIO register 0x02 */
    	    if (p1021_i2c_read_fpga_byte (GPIO_FRAMER_REG, &rd_data)) {
    	        sprintf(err_msg, "\n%s, [#%d]:Read fail offset @%#x = %#x\n"
    	        		, __FUNCTION__, __LINE__, GPIO_FRAMER_REG,
		       rd_data);
    	        print_err(TRUE, err_msg, LVL_0);
    	        return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
    	    /* gpio8 = 0x11  ( 0001 0001 )
    	     * rd_data = ~ 1110 1110
    	     * rd_data = rd_data & mask (0xFF)
    	     * rd_data = 0x11
    	     */
    	    rd_data = ~rd_data;
    	    rd_data &= mask;
#ifdef DEBUG
            printf("\n%s, [#%d]: Framer GPIO (0x%02x) contents = 0x%02x",
		   __FUNCTION__, __LINE__, GPIO_FRAMER_REG, rd_data);
#endif
    	    if(rd_data != gpio8) {
    	    	sprintf(err_msg, "\n%s, [#%d]:FPGA(0x2)Compare value mismatch"
    	    			" !! current value 0x%02x, Expected 0x%02x\n"
    	    			, __FUNCTION__, __LINE__, rd_data, gpio8);
    	    	print_err(TRUE, err_msg, LVL_0);
    	    	return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	    }
    	}
    	wr_data = wr_data << 2;
    	gpio8 = gpio8 << 1;
	
        /* Reset the Framer GPIO bit */
    	if (reset_fpga_gpio_framer_bit(FRAMER_GPIO)== FAILED ) {
    		sprintf(err_msg, "\n%s, [#%d]:Reset FPGA GPIO bit fail \n"
    				, __FUNCTION__, __LINE__);
    		print_err(TRUE, err_msg, LVL_0);
    		return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL);
    	}
    }
    printf("\n Testing FRAMER GPIO to FPGA GPIO end \n");

    printf("\n Test FPGA GPIO and Framer GPIO pass\n");fflush(0);

    return (TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_OK);

}


/**********************************************************************
 *
 * Function: patriot_fpga_intr_test
 *
 * This function tests FPGA interrupts
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_fpga_intr_test(void)
{
    ushort offset = 0, i;
    uchar temp = 0;
    uchar buf = 0;
    int fpga_intr = 0, ret_val;
    sm_patriot_fpga_intr_iface_t fpga_intr_iface;
    patriot_msg_t msg;

    printf("\npatriot_fpga_interrupt_test\n");

    memset((uchar *)&fpga_intr_iface, 0, sizeof(sm_patriot_fpga_intr_iface_t));
    memset((uchar *)&msg, 0, sizeof(patriot_msg_t));
    ret_val = ioctl(p1021_fd, IOCTL_CLEAR_FPGA_INTR, (unsigned long)&msg);

    offset = 0x10;
    buf = 0x01;

    /* Turn on interrupt, do not check the return value
       becaue it will fail due to the kernel module clearing */
    p1021_i2c_write_fpga_byte (offset, buf);

    msleep(2000);
    ret_val = ioctl(p1021_fd, IOCTL_GET_FPGA_INTR, (unsigned long)&msg);
#ifdef DEBUG
    printf("\nmsg.data[0] = %d\n", msg.data[0]);
#endif    
    fpga_intr = msg.data[0];
    if (!fpga_intr) {
	sprintf(err_msg, "\n%s, [#%d]: No FPGA interrupt, fpga_intr = %d\n"
			,__FUNCTION__, __LINE__, fpga_intr);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_FPGA_INTR_TEST_FAIL);
    }

    printf("\npatriot_fpga_interrupt_test completed\n");fflush(0);
    return (TO_HOST_FPGA_INTR_TEST_OK);
}


/**********************************************************************
 *
 * Function: patriot_host_to_module_gpio1_wr1_test
 *
 * This function tests from host to module gpio
 * Note: PA23 is input.
 *       Test plan is to set 1 from host and module side to check is 1
 *       if the checked verify is 1, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_host_to_module_gpio1_wr1_test(void)
{
    printf("\n patriot_host_to_module_gpio1_wr1_test\n");

    /* Set up the GPIO pins on the CPU
       PA23 is input
       Configure pin direction and  function
       PA23 as input
    */
    /* clear direction bits for PA23 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(23));
    /* PA23 as input */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_IN(23);
    /* clear function bits for PA23 and set it as GPIO*/
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(23, 0x3));

    /* Verify the value from host IO 1 */
#ifdef DEBUG
    printf("\n%s, [#%d]: IO PA23 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddata);
#endif
    wastetime(100);
    if((REGB->im_gur.cpddata & 0x00000100) != 0x00000100) {
    	sprintf(err_msg, "\n%s, [#%d]: Failed PA23 value = 0x%08x, "
    			"expected = 0x%08x", __FUNCTION__, __LINE__,
	       (REGB->im_gur.cpddata & 0x00000100), 0x00000100);
    	print_err(TRUE, err_msg, LVL_0);
    	return (TO_FROM_HOST_CPU_GPIO_TEST_IO1_W1_FAIL);
    }

    printf("\n patriot_host_to_module_gpio1_wr1_test pass\n");fflush(0);

    return (TO_FROM_HOST_CPU_GPIO_TEST_IO1_W1_OK);
}


/**********************************************************************
 *
 * Function: patriot_host_to_module_gpio1_wr0_test
 *
 * This function tests from host to module gpio
 * Note: PA23 is input.
 *       Test plan is to set 0 from host and module side to check is 0
 *       if the checked verify is 0, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_host_to_module_gpio1_wr0_test(void)
{
    printf("\n patriot_host_to_module_gpio1_wr0_test\n");

    /* Set up the GPIO pins on the CPU
       PA23 is input
       Configure pin direction and  function
       PA23 as input
    */
    /* clear direction bits for PA23 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(23));
    /* PA23 as input */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_IN(23);
    /* clear function bits for PA23 and set it as GPIO*/
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(23, 0x3));

    /* Verify the value from host IO 1 */
#ifdef DEBUG
    printf("\n%s, [#%d]: IO PA23 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddata);
#endif
    wastetime(100);
    if((REGB->im_gur.cpddata & 0x00000100) != 0x00000000) {
    	sprintf(err_msg, "\n%s, [#%d]: Failed PA23 value = 0x%08x,"
    			" expected = 0x%08x", __FUNCTION__, __LINE__,
	       (REGB->im_gur.cpddata & 0x00000100), 0x00000000);
    	print_err(TRUE, err_msg, LVL_0);
    	return (TO_FROM_HOST_CPU_GPIO_TEST_IO1_W0_FAIL);
    }

    printf("\n patriot_host_to_module_gpio1_wr0_test pass\n");fflush(0);

    return (TO_FROM_HOST_CPU_GPIO_TEST_IO1_W0_OK);
}


/**********************************************************************
 *
 * Function: patriot_host_to_module_gpio4_wr1_test
 *
 * This function tests from host to module gpio
 * Note: PB10 is input.
 *       Test plan is to set 1 from host and module side to check is 1
 *       if the checked verify is 1, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_host_to_module_gpio4_wr1_test(void)
{
    printf("\n patriot_host_to_module_gpio4_wr1_test\n");

    /* Set up the GPIO pins on the CPU
       PB10 is input
       Configure pin direction and  function
       PB10 as input
    */
    /* clear direction bits for PB10 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(10));
    /* PB10 as input */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_IN(10);
    /* clear function bits for PB10 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(10, 0x3));
    
    /* Verify the value from host IO 4 */
#ifdef DEBUG
    printf("\n%s, [#%d]: IO PB10 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    wastetime(100);
    if((REGB->im_gur.cpddatb & 0x00200000) != 0x00200000) {
	sprintf(err_msg, "\n%s, [#%d]: Failed PB10 value = 0x%08x, expected = 0x%08x",
			__FUNCTION__, __LINE__,
	       (REGB->im_gur.cpddatb & 0x00200000), 0x00200000);
	print_err(TRUE, err_msg, LVL_0);
        return (TO_FROM_HOST_CPU_GPIO_TEST_IO4_W1_FAIL);
    }
    
    printf("\n patriot_host_to_module_gpio4_wr1_test pass\n");fflush(0);
    
    return (TO_FROM_HOST_CPU_GPIO_TEST_IO4_W1_OK);
}


/**********************************************************************
 *
 * Function: patriot_host_to_module_gpio4_wr0_test
 *
 * This function tests from host to module gpio
 * Note: PB10 is input.
 *       Test plan is to set 0 from host and module side to check is 0
 *       if the checked verify is 0, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_host_to_module_gpio4_wr0_test(void)
{
    printf("\n patriot_host_to_module_gpio4_wr0_test\n");

    /* Set up the GPIO pins on the CPU
       PB10 is input
       Configure pin direction and  function
       PB10 as input
    */
    /* clear direction bits for PB10 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(10));
    /* PB10 as input */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_IN(10);
    /* clear function bits for PB10 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(10, 0x3));

    /* Verify the value from host IO 4 */
#ifdef DEBUG
    printf("\n%s, [#%d]: IO PB10 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    wastetime(100);
    if((REGB->im_gur.cpddatb & 0x00200000) != 0x00000000) {
	sprintf(err_msg, "\n%s, [#%d]: Failed PB10 value = 0x%08x, expected = 0x%08x",
			__FUNCTION__, __LINE__,
	       (REGB->im_gur.cpddatb & 0x00200000), 0x00000000);
	print_err(TRUE, err_msg, LVL_0);
        return (TO_FROM_HOST_CPU_GPIO_TEST_IO4_W0_FAIL);
    }

    printf("\n patriot_host_to_module_gpio4_wr0_test pass\n");fflush(0);

    return (TO_FROM_HOST_CPU_GPIO_TEST_IO4_W0_OK);
}


/**********************************************************************
 *
 * Function: patriot_module_to_host_gpio3_rd1_test
 *
 * This function tests from module to host
 * Note: PB04 is output.
 *       Test plan is to set 1 from module to host side to check is 1
 *       if the checked verify is 1, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_module_to_host_gpio3_rd1_test(void)
{
    printf("\n patriot_module_to_host_gpio3_rd1_test\n");

    /* Set up the GPIO pins on the CPU */
    /* PB04 is output */
    
    /* clear direction bits for PB04 */
    REGB->im_gur.cpdir1b&= ~(MPC8500_CPDIR1_INOUT(4));
    /* PB04 as output */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_OUT(4);
    /* clear function bits for PB04 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(4, 0x3));
    /* Set PB04 to 0x08000000 */
#ifdef DEBUG
    printf("\n%s, [#%d]: Before IO PB04 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    REGB->im_gur.cpddatb |= 0x08000000;
    wastetime(100);
#ifdef DEBUG
    printf("\n%s, [#%d]: After IO PB04 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    if((REGB->im_gur.cpddatb & 0x08000000) != 0x08000000) {
    	sprintf(err_msg, "\n%s, [#%d]: Failed PB04 value = 0x%08x, "
    			"expected = 0x%08x", __FUNCTION__, __LINE__,
    			(REGB->im_gur.cpddatb & 0x08000000), 0x08000000);
    	print_err(TRUE, err_msg, LVL_0);
        return (TO_FROM_HOST_CPU_GPIO_TEST_IO3_R1_FAIL);
    }

    printf("\n patriot_module_to_host_gpio3_rd1_test pass\n");fflush(0);

    return (TO_FROM_HOST_CPU_GPIO_TEST_IO3_R1_OK);
}


/**********************************************************************
 *
 * Function: patriot_module_to_host_gpio3_rd0_test
 *
 * This function tests from module to host
 * Note: PB04 is output.
 *       Test plan is to set 0 from module to host side to check is 0
 *       if the checked verify is 0, then return pass otherwise return
 *       fail.
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_module_to_host_gpio3_rd0_test(void)
{
    printf("\n patriot_module_to_host_gpio3_rd0_test\n");

    /* Set up the GPIO pins on the CPU */
    /* PB04 is output */
    
    /* clear direction bits for PB04 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(4));
    /* PB04 as output */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_OUT(4);
    /* clear function bits for PB04 and set it as GPIO*/
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(4, 0x3));
    /* Set PB04 to ~0x08000000 */
#ifdef DEBUG
    printf("\n%s, [#%d]: Before IO PB04 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    REGB->im_gur.cpddatb &= ~0x08000000;
    wastetime(100);
#ifdef DEBUG
    printf("\n%s, [#%d]: After IO PB04 contents = 0x%08x", __FUNCTION__,
	   __LINE__, REGB->im_gur.cpddatb);
#endif
    if((REGB->im_gur.cpddatb & 0x08000000) != 0x00000000) {
    	sprintf(err_msg, "\n%s, [#%d]: Failed PB04 value = 0x%08x, "
    			"expected = 0x%08x", __FUNCTION__, __LINE__,
    			(REGB->im_gur.cpddatb & 0x08000000), 0x00000000 );
    	print_err(TRUE, err_msg, LVL_0);
        return (TO_FROM_HOST_CPU_GPIO_TEST_IO3_R0_FAIL);
    }

    printf("\n patriot_module_to_host_gpio3_rd0_test pass\n");fflush(0);

    return (TO_FROM_HOST_CPU_GPIO_TEST_IO3_R0_OK);
}

/**********************************************************************
 *
 * Function: patriot_clear_t3_dig_link_lpbk
 *
 * This function tests T3 digital link. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_t3_dig_link_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   0x0 = 300 Bandwidth \n"
    	   "\n   0x1 = 10000 Bandwidth \n"
    	   "\n   0x2 = 20000 Bandwidth \n"
    	   "\n   0x3 = 34010 Bandwidth \n"
    	   "\n   0x4 = 44210 Bandwidth \n"
           );

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("Select T3 for Digital Link rate: ", 0x3, 0, 0x4);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, DIGITAL_LINK,
                                (patriot_t3_dig_link_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_larscom_lpbk
 *
 * This function tests T3 larscom. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_t3_larscom_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   0x0 = 3100 Bandwidth \n"
    	   "\n   0x1 = 10000 Bandwidth \n"
    	   "\n   0x2 = 20000 Bandwidth \n"
    	   "\n   0x3 = 34010 Bandwidth \n"
    	   "\n   0x4 = 44210 Bandwidth \n"
           );

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("Select T3 for Larscom rate: ", 0x3, 0, 0x4);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, LARSCOM,
                                (patriot_t3_larscom_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_adtran_lpbk
 *
 * This function tests T3 adtran. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_t3_adtran_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   0x0 = 75 Bandwidth \n"
    	   "\n   0x1 = 10000 Bandwidth \n"
    	   "\n   0x2 = 20000 Bandwidth \n"
    	   "\n   0x3 = 34010 Bandwidth \n"
    	   "\n   0x4 = 44210 Bandwidth \n"
           );

    if (param_cmd_menu == PATRIOT_MENU){
        i = gethex_answer("Select T3 for Adtran rate: ", 0x3, 0, 0x4);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, ADTRAN,
                                (patriot_t3_adtran_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: patriot_clear_t3_verilink_lpbk
 *
 * This function tests T3 verilink. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_t3_verilink_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   0x0 = 1500 Bandwidth \n"
    	   "\n   0x1 = 10000 Bandwidth \n"
    	   "\n   0x2 = 20000 Bandwidth \n"
    	   "\n   0x3 = 34010 Bandwidth \n"
    	   "\n   0x4 = 44210 Bandwidth \n"
           );

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("Select T3 for Verilink rate: ", 0x3, 0, 0x4);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, VERILINK,
                                (patriot_t3_verilink_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_clear_e3_dig_link_lpbk
 *
 * This function tests E3 Digital Link. In continuous mode, it only
 * prompts for the input once.
 *
 * Input : lpbk_mode - Internal or External Loopback mode
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_clear_e3_dig_link_lpbk(uchar lpbk_mode)
{
    static ulong i;

    printf("\n   0x0 = 358 Bandwidth \n"
    	   "\n   0x1 = 10000 Bandwidth \n"
    	   "\n   0x2 = 20000 Bandwidth \n"
    	   "\n   0x3 = 34010 Bandwidth \n"
           );

    if (param_cmd_menu == PATRIOT_MENU) {
        i = gethex_answer("Select E3 for Digital link rate: ", 0x3, 0, 0x3);
    }else {
    	i = param_arr[3];
    }

    if (patriot_clear_te3_subrate_test(MODE_T3, DIGITAL_LINK,
                                (patriot_e3_dig_link_tbl[i]), lpbk_mode))
        return (FAILED);
    return (PASSED);
}

/**********************************************************************
 * Function: patriot_ddr_ecc_single_bit_err_test
 *
 * This function will enable single bit error reporting on DRAM DDR
 * bus if not already enabled, then it injects single bit ECC error.
 * CPU should corrects the single bit error on the DDR bus and 
 * generates interrupt. this function hence makes sure that the
 * interrupt is serviced 
 *
 * Input : 
 *
 * Output:
 *
 **********************************************************************
 */
int
patriot_ddr_ecc_single_bit_err_test (void) 
{
    ulong *ptr = &uncached_data;
    int ecc_detect = 0, ret_val, i;
    uchar *temp;

    printf("\npatriot_ddr_ecc_single_bit_err_test\n");

    /* Make sure DDR internal interrupt is disabled in PIC */
    REGB->im_pic.iivpr2 |= MPC8500_PIC_INTR_MASK;
    /* 
     * Setup DDR controller for single-bit error 
     */
    /* how many single bit error, before DDR reports */
    REGB->im_ddr1.err_sbe = (0x01 << 16); 

    /* Enable detection and reporting of single-bit error */
    if (REGB->im_ddr1.err_disable & MPC8500_DDR_SBED) {
	/* single-bit error is not enabled, lets enable it */ 

	REGB->im_ddr1.err_disable &= ~MPC8500_DDR_SBED;
    }
 
    /* Enable single-bit error interrupt */
    if (!(REGB->im_ddr1.err_int_en & MPC8500_DDR_SBEE)) {
	/* single-bit error interrupt is not enabled, lets enable it */ 

	REGB->im_ddr1.err_int_en |= MPC8500_DDR_SBEE;
    }

    /* Enable error injection */
    REGB->im_ddr1.ecc_err_inject |=MPC8500_MEM_ERR_INJ_EN;  

    /* Invert bit31 on memory bus write */
    REGB->im_ddr1.data_err_inject_lo =0x00000001;  

    /* Write something to uncached memory to generate ecc error */
    *ptr = 0xaaaa5555;  // <-- this should generated internal DDR interrupt

    msleep(1000);
    printf("\n*ptr = 0x%08x\n", *ptr);

    msleep(1000);

#ifdef DEBUG
    printf("\nREGB->im_ddr1.err_detect @0x%08x = 0x%08x\n",
	   &REGB->im_ddr1.err_detect, REGB->im_ddr1.err_detect);

    printf("\nREGB->im_ddr1.err_sbe @0x%08x = 0x%08x\n",
	   &REGB->im_ddr1.err_sbe, REGB->im_ddr1.err_sbe);
#endif
    
	
    ecc_detect = REGB->im_ddr1.err_detect & MPC8500_MEM_ERR_SBE;
    
   
    REGB->im_ddr1.ecc_err_inject &= ~MPC8500_MEM_ERR_INJ_EN;
    REGB->im_ddr1.data_err_inject_lo =0x00000000;
    REGB->im_ddr1.err_detect = MPC8500_MEM_ERR_SBE | 
				   MPC8500_MEM_ERR_MME;
    REGB->im_ddr1.err_disable |= MPC8500_DDR_SBED;
    REGB->im_ddr1.err_int_en &= ~MPC8500_DDR_SBEE;
    REGB->im_ddr1.err_sbe = 0x00000000;

    /* Read and write back the correct value */
    printf("\n*ptr = 0x%08x\n", *ptr);
    *ptr = 0xaaaa5555;

#ifdef DEBUG   
    for (i = 0; i < 10; i++) {
	printf("\nREGB->im_ddr1.err_detect @0x%08x = 0x%08x\n",
	       &REGB->im_ddr1.err_detect, REGB->im_ddr1.err_detect);
	
	printf("\nREGB->im_ddr1.err_sbe @0x%08x = 0x%08x\n",
	       &REGB->im_ddr1.err_sbe, REGB->im_ddr1.err_sbe);
	msleep(500);
    }
#endif    
    printf("\necc_detect = 0x%08x\n", ecc_detect);
    if (ecc_detect) {
	printf("\nECC Error occurred and corrected by CPU\n");
    } else {
	sprintf(err_msg, "\n%s, [#%d]:ECC Error did not occur"
			, __FUNCTION__, __LINE__);
	print_err(TRUE, err_msg, LVL_0);
        return(TO_HOST_ECC_TEST_FAIL);
    }
    printf("\npatriot_ddr_ecc_single_bit_err_test completed\n");fflush(0);
    return(TO_HOST_ECC_TEST_OK);

}

/**********************************************************************
 * Function: patriot_uart_test
 *
 * This function will get a string from host and compare to the orininal
 * string to check they are the same. Then it uses printf to print out
 * another string. The host side will read and check it.
 * This test only applies to Overlord
 *
 * Input : None
 *
 * Output: TO_HOST_UART_TEST_OK/TO_HOST_UART_TEST_FAIL
 *
 **********************************************************************
 */
int
patriot_uart_test (void) 
{
    int i;
    char buf[64], *ptr, *ret_str;
    char *str = "ABCDEFGH\n";
    char *str1 = "STUVWXYZ\n";
    int status;

    ptr = &buf[0];

    ret_str = fgets(ptr, strlen(str) + 1, stdin);
	
    if (ret_str == NULL) {
	sprintf (err_msg, "\n%s, [#%d]:Receive string is NULL\n"
			, __FUNCTION__, __LINE__);fflush(0);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UART_TEST_FAIL);
    }
#if DEBUG
    printf("\nstrlen(ptr) = %d", strlen(ptr));fflush(0);
    for (i = 0; i < strlen(ptr); i++) {
	printf("\n0x%02x", ptr[i]);
    }
#endif    
    
    status = strncmp(str, ptr, strlen(str));
    
    if (status) {
	sprintf(err_msg, "\n%s, [#%d]:Fail to get the string from host, "
			"expect %s, receive %s\n", __FUNCTION__, __LINE__, str, ptr);
	print_err(TRUE, err_msg, LVL_0);
	return (TO_HOST_UART_TEST_FAIL);
    }
    /* Print out the second string so the host can read */
    printf("%s", str1);fflush(0);
    printf("\npatriot_uart_test completed\n");fflush(0);

    return (TO_HOST_UART_TEST_OK);
}

/***********************************************************************
 * Name: patriot_ge0_loopback_test
 *
 * Description:
 *    This test will send/receive ethernet message packets to the host
 * and loopback there
 *
 * Input: option: none
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int patriot_ge0_loopback_test(void)
{
    int retval, count, frame, etsec_num, num_bytes;
    int cnt, speed, i, j;
    uchar       base_val, inc_val = 0;
    ccsr_tsec_t *regs;
    tsec_bd_t *tx_bd, *rx_bd, *tx_bd_vir_addr, *rx_bd_vir_addr;
    fe_packet_t *tx_buf, *tx_buf_vir_addr;
    tsec_info_struct_t *tsec_p;
    unsigned char *temp_p, *rx_buf_phy, *rx_buf_vir;
    unsigned char *tx_buf_phy, *tx_buf_vir;

    unsigned short pak_size[NUM_RX_BD] = {64, 108, 512, 256,
                                          1490, 65, 1411, 128,
                                          66, 719};

    printf("\npatriot_ge0_loopback_test\n");

    etsec_num = ETSEC2;
    /* Only on Overlord, we have the 2nd GE interface */
    if (patriot_setup_eth_dev(etsec_num, SGMII_LPBK_NONE)) {
	return (FAILED);
    }

    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    
    regs = (ccsr_tsec_t *)tsec_p->reg_base_addr;

    retval = TO_HOST_GE0_LPBK_TEST_OK;

    for (i = 0; i < NUM_RX_BD; i++) {
	tx_bd = (tsec_bd_t *)etsec_get_txbd(tsec_p);
	tx_bd_vir_addr = (tsec_bd_t *)vir_addr((ulong)tx_bd);
	tx_buf = (fe_packet_t *)tx_bd_vir_addr->buf_ptr;
	tx_buf_vir_addr = (fe_packet_t *)vir_addr((ulong)tx_buf);
#ifdef DEBUG
	printf("\ntx_bd = 0x%08x", tx_bd);
	printf("\ntx_bd_vir_addr = 0x%08x", tx_bd_vir_addr);
	printf("\ntx_buf = 0x%08x", tx_buf);
	printf("\ntx_buf_vir_addr = 0x%08x", tx_buf_vir_addr);
#endif	
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.dest_addr),
	       (char *)host_mac_addr, MAC_ADDR_SIZE);
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.src_addr),
	       (char *)module_mac_addr, MAC_ADDR_SIZE);

	for (j = 0; j < pak_size[i] - sizeof(ether_hdr_t); j++ ) {
	    tx_buf_vir_addr->data[j] = j;
	}
	
	tx_bd_vir_addr->length = pak_size[i];

#ifdef DEBUG
	printf("\necntrl %#.8x, maccfg1 @%#x=%#.8x, maccfg2 @%#x=%#.8x\n",
	       regs->ecntrl, &regs->maccfg1, regs->maccfg1,
	       &regs->maccfg2, regs->maccfg2);
#endif
	
	etsec_recv_nframes[etsec_num - 1] = 0; /* Clear receive frame counter */
	etsec_tx_nframes[etsec_num - 1] = 0; /* Clear transmit frame counter */

#ifdef DEBUG		
	printf("\n Before TX\n");
	dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x80, vir_addr(tsec_p->rx_bd),
	       BW_32BITS);
#endif
	tx_bd_vir_addr = (tsec_bd_t *)vir_addr((ulong)tx_bd);
	tx_buf_phy = (uchar *)tx_bd_vir_addr->buf_ptr;
	tx_buf_vir = (uchar *)vir_addr((ulong)tx_buf_phy);

#ifdef DEBUG
	printf("\ntx_buf_vir = 0x%08x\n", tx_buf_vir);
	dismem((uchar *)tx_buf_vir, 0x40, tx_buf_vir, BW_32BITS);
#endif
	/* Transmit the frame */
	if (etsec_send(etsec_num, tx_bd_vir_addr) != 0) {
	    sprintf(err_msg, "\n%s, [#%d]:Unable to transmit frame%d to eTSEC%d, "
		   "txbd @%#x", __FUNCTION__, __LINE__, frame, etsec_num, tx_bd);
	    print_err(TRUE, err_msg, LVL_0);
	    retval = TO_HOST_GE0_LPBK_TEST_FAIL;
	    break;
	}
#ifdef DEBUG		
	printf("\nAfter etsec_send");
	dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x40, vir_addr(tsec_p->rx_bd),
	       BW_32BITS);
#endif		
	msleep(100);

	/* wait for frame reception */
	for (count = 100; count > 0; count--) {
	    if (etsec_recv_frame_ready(etsec_num, POLL_MODE)) {
		break;
	    }
	    msleep(1);
	}
	
	if (count == 0) {
	    break;
	}
	
	/* get received frame */
	rx_bd = (tsec_bd_t *)etsec_get_rxbd(tsec_p);
	
	if (rx_bd == 0) {
	    sprintf(err_msg, "\n%s, [#%d]:Unable to get rx_bd\n"
	    		, __FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    break;
	}
	
	rx_bd_vir_addr = (tsec_bd_t *)vir_addr((ulong)rx_bd);
	rx_buf_phy = (uchar *)rx_bd_vir_addr->buf_ptr;
	rx_buf_vir = (uchar *)vir_addr((ulong)rx_buf_phy);
#ifdef DEBUG	
	printf("\nrx_buf_vir = 0x%08x\n", rx_buf_vir);
	dismem((uchar *)rx_buf_vir, 0x40, rx_buf_vir, BW_32BITS);
#endif	
	if (rx_bd == 0) {
	    sprintf(err_msg, "%s, [#%d]:Error receiving frame %d on eTSEC%d, "
		   "rxbd @%#x=%#x", __FUNCTION__, __LINE__,
		   frame, etsec_num, &regs->rbptr, regs->rbptr);
	    print_err(TRUE, err_msg, LVL_0);
	    retval = TO_HOST_GE0_LPBK_TEST_FAIL;
	    break;
	} else {
	    /* verify transmission and reception status */
	    retval  = check_tsec_tx_status(tx_bd_vir_addr);
	    retval |= check_tsec_rx_status(rx_bd_vir_addr);
	    retval |= check_tsec_rx_frame (tx_bd_vir_addr, rx_bd_vir_addr);
	    
	    /*
	     * if wrap occurs, we must re-initialize the tx and rx
	     * buffer descriptors so that we can Tx/Rx more frames
	     */
	    retval = check_for_bd_wrap(etsec_num);
	    if (retval) {
		    sprintf(err_msg, "%s, [#%d]:Error receiving frame %d on eTSEC%d, "
			   "rxbd @%#x=%#x", __FUNCTION__, __LINE__,
			   frame, etsec_num, &regs->rbptr, regs->rbptr);
		    print_err(TRUE, err_msg, LVL_0);
		retval = TO_HOST_GE0_LPBK_TEST_FAIL;
		break;
	    }
	}

    }
    printf("\npatriot_ge0_loopback_test PASSED, retval = %d\n", retval);fflush(0);
    cleanup_tsec(etsec_num);
    return (TO_HOST_GE0_LPBK_TEST_OK);
}



/*------------------------------------------------------------------------------
 * $Log: patriot_test.c,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.17  2014/03/06 01:56:52  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.16  2012/12/12 19:15:34  huanngo
 * Fix the error message in T3 subrate loopback
 *
 * Revision 1.15  2012/12/04 13:04:45  steja
 * 1. backing back the DLB to ALB for framer interrupt
 * 2. add missing error message that left before.
 *
 * Revision 1.14  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.13  2012/10/25 07:24:10  steja
 * Remove "Clear" from the subrate test
 *
 * Revision 1.12  2012/10/16 07:42:40  steja
 * Improve the GPIO test
 *
 * Revision 1.11  2012/10/15 21:19:18  huanngo
 * Program the correct MAC address to the ethernet header before sending the packets to host
 *
 * Revision 1.10  2012/09/14 23:41:56  huanngo
 * Adding the utility to display FPGA secure boot registers and multiboot info table
 *
 * Revision 1.9  2012/08/07 07:03:22  steja
 * 1. Fix the Framer Interrupt test to not download FPGA image
 * 2. update the code for Framer interrupt test, add delay, clear bits.
 *
 * Revision 1.8  2012/08/06 17:34:57  huanngo
 * Fix bugs in SPI PROM test/read/write/erase
 *
 * Revision 1.7  2012/07/24 22:21:48  huanngo
 * Adding "test ... completed" after each test on the module side
 *
 * Revision 1.6  2012/07/18 23:52:59  huanngo
 * Check if the DONE pin is high, skip programming FPGA
 *
 * Revision 1.5  2012/06/29 22:45:19  huanngo
 * Update UART test and check the DONE pin for FPGA, if it's asserted, skip download FPGA
 *
 * Revision 1.4  2012/06/08 23:35:39  huanngo
 * Adding constant definitions for ECC memory,UART and GE 0 loopback tests
 *
 * Revision 1.3  2012/05/17 02:53:50  steja
 * Remove the IO 7 test for P2 board
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.31  2012/04/30 18:37:25  huanngo
 * Download FPGA when necesary, not right after Linux boot up
 *
 * Revision 1.1.4.30  2012/04/19 17:28:55  huanngo
 * Add support for E3 Kentrox subrate tests
 *
 * Revision 1.1.4.29  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.28  2012/04/06 07:32:14  steja
 * Update the GPIO test for rework changes from PA24 to PB4
 *
 * Revision 1.1.4.27  2012/03/27 07:49:59  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.26  2012/03/21 08:38:20  steja
 * Include  the verilink subrate on the loopback test
 *
 * Revision 1.1.4.25  2012/03/21 07:31:09  steja
 * 1. adjust subrate test time slots
 * 2. add debug flag
 *
 * Revision 1.1.4.24  2012/03/16 12:05:52  steja
 * Update the code to support Subrate individual test loopback
 *
 * Revision 1.1.4.23  2012/03/13 13:40:03  steja
 * 1. Support Framer Interrupt
 * 2. Fix E3 Unframe Config
 *
 * Revision 1.1.4.22  2012/03/12 23:01:52  huanngo
 * Increase the packet size to 1800 in HDLC and fix the bug in FPGA interrupt test
 *
 * Revision 1.1.4.21  2012/03/01 19:10:50  huanngo
 * Remove the subrate bypass loopback tests and add subrate loopback tests
 *
 * Revision 1.1.4.20  2012/02/28 02:23:11  huanngo
 * Cosmetic change to fix some line allignement
 *
 * Revision 1.1.4.19  2012/02/06 22:29:04  huanngo
 * Update to not compile code using bitbake, use make with local kernel
 *
 * Revision 1.1.4.18  2012/01/17 04:05:41  steja
 * Update for loopback code
 *
 * Revision 1.1.4.17  2012/01/14 01:55:49  huanngo
 * Clean up and fix the timing
 *
 * Revision 1.1.4.16  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.15  2011/12/22 12:11:50  steja
 * Fix the GPIO test for Rev 1B board.
 *
 * Revision 1.1.4.14  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.13  2011/12/08 15:07:11  steja
 * Update IO Test function
 *
 * Revision 1.1.4.12  2011/12/01 18:51:05  huanngo
 * Support new command to write MAC address to EEPROM and fix bugs
 *
 * Revision 1.1.4.11  2011/11/24 12:15:50  steja
 * Update Patriot Code
 *
 * Revision 1.1.4.10  2011/11/24 09:33:34  steja
 * Update Patriot code
 *
 * Revision 1.1.4.9  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.8  2011/11/23 12:24:24  steja
 * Update the GPIO testing
 *
 * Revision 1.1.4.7  2011/11/17 15:18:08  steja
 * Add Test Function for FPGA GPIO and GPIO on Framer
 *
 * Revision 1.1.4.6  2011/11/15 14:05:53  steja
 * Update DS3170 code
 * 1. Fix the AIS test
 * 2. Register BERT test
 *
 * Revision 1.1.4.5  2011/10/27 09:35:08  steja
 * Update DS3170 BERT test
 *
 * Revision 1.1.4.4  2011/10/07 01:11:46  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.3  2011/09/20 10:10:56  steja
 * Update DS3170 code for AIS and BERT register
 *
 * Revision 1.1.4.2  2011/08/18 19:43:25  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.33  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.32  2011/08/03 01:51:35  steja
 * Update DS3170 code :
 * 1. Init DS3170
 * 2. Loopback test
 *
 * Revision 1.1.2.31  2011/07/26 14:35:51  steja
 * Update DS3170 code
 *
 * Revision 1.1.2.30  2011/07/21 12:14:15  steja
 * Update DS3170 functionality
 *
 * Revision 1.1.2.29  2011/07/19 06:11:35  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.28  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.27  2011/07/11 07:43:10  steja
 * Update DS3170 function patriot_clear_e3_ais_test(void)
 *
 * Revision 1.1.2.26  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.25  2011/07/07 16:21:54  steja
 * 1. Clean up code
 * 2. Add check statur register after loopback test for DS3170.
 *
 * Revision 1.1.2.24  2011/07/05 09:57:36  steja
 * Update Loopback pass for DS3170 code
 *
 * Revision 1.1.2.23  2011/07/04 09:54:35  steja
 * Update DS3170 code :
 * 1. Add {FROM_HOST_CLR_T3_EX_LPBK_TEST}
 * 2. FROM_HOST_CLR_T3_EX_LPBK_TEST
 *     FROM_HOST_CLR_T3_SUB_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_SUB_EX_LPBK_TEST
 * 3. Update return (TO_HOST_CLR_E3_EX_LPBK_TEST_FAIL) and
 *     return(TO_HOST_CLR_E3_EX_LPBK_TEST_OK)
 *
 * Revision 1.1.2.22  2011/07/04 07:43:03  steja
 * 1. Change TO_HOST_CLR_T3_SUB_IDVL_LPBK_TEST_OK  to PASSED
 * 2. Change TO_HOST_CLR_T3_SUB_IDVL_LPBK_TEST_FAIL to FAILED
 *
 * Revision 1.1.2.21  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.20  2011/07/01 15:39:07  steja
 * 1. Update DS3170 utility test code
 * 2. Update Internal and External loopback test for DS3170
 *
 * Revision 1.1.2.19  2011/06/30 16:31:42  steja
 * 1. Update DS3170 Register table
 * 2. Update DS3170 patriot_clear_t3_intr_test
 *
 * Revision 1.1.2.18  2011/06/29 16:24:55  steja
 * Update DS3170 code.
 *
 * Revision 1.1.2.17  2011/06/28 16:59:50  steja
 * 1. Update FPGA register read and write function
 * 2. Update DS3170 register test function
 * 3. Update Common register test, reg alter, reg display
 *
 * Revision 1.1.2.16  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.15  2011/06/27 14:14:06  steja
 * 1. Update FPGA register test function
 * 2. Add FPGA dump register function
 * 3. Add FPGA register read / write utility function
 * 4. Add FPGA initialization function
 *
 * Revision 1.1.2.14  2011/06/22 02:37:18  steja
 * Update DS3170 code Interrupt Handler function
 *
 * Revision 1.1.2.13  2011/06/17 07:03:54  steja
 * 1. Move Patriot_fpga_test to patriot_main.c
 * 2. Remove fpga loopback test item
 *
 * Revision 1.1.2.12  2011/06/14 10:33:07  steja
 * Temporary variable for DS3170 interrupt test
 *
 * Revision 1.1.2.11  2011/06/14 10:13:42  steja
 * Update DS3170 code and FPGA Register test
 *
 * Revision 1.1.2.10  2011/06/13 12:21:27  steja
 * Add submenu utilites for DS3170 and FPGA
 *
 * Revision 1.1.2.9  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.8  2011/06/09 07:03:37  steja
 * Update the code for DS3170 and FPGA's Patriot
 *
 * Revision 1.1.2.7  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.6  2011/05/26 03:17:40  steja
 * Modified uint to uchar
 *
 * Revision 1.1.2.5  2011/05/25 16:05:06  steja
 * Update the DS3170 testing function based on specs
 *
 * Revision 1.1.2.4  2011/05/21 01:01:29  huanngo
 * Support memory test, I2C interface
 *
 * Revision 1.1.2.3  2011/05/09 21:07:23  huanngo
 * Update code for HDLC over TDM loopback
 *
 * Revision 1.1.2.2  2011/05/09 15:38:37  steja
 * Initial Check in Maxim DS3170 Framer
 *
 * Revision 1.1.2.1  2011/05/02 23:33:23  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */


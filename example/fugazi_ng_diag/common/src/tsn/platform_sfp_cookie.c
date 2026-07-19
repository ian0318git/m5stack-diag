/* $Id: platform_sfp_cookie.c,v 1.4 2018/05/30 10:06:57 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_sfp_cookie.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_sfp_cookie.c
 * Description: TSN SFP cookie.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "platform_i2c.h"
#include "i2c_address.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "platform_fpga.h"
#include "plat_defs.h"
#include "platform_sfp_cookie.h"

static char *connector[16] = {
	"Unknown",
	"SC",
	"Fibre Channel Style 1 copper connector",
	"Fibre Channel Style 2 copper connector",
	"BNC/TNC",
	"Fibre Channel coaxial headers",
	"FiberJack",
	"LC",
	"MT-RJ",
	"MU",
	"SG",
	"Optical pigtail"};

/*
 * Function: read_sfp_cookie 
 *
 * Description:
 *   This function is the function to ready SFP eeprom contents
 *
 * Parameters:
 *   opt -- dummy 
 *
 * Returns:
 *   PASSED/FAILED
 */

int read_sfp_cookie (int opt)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = PASSED;
    unsigned char A50[SFP_BUFFER_256]; //only interested in the first 128 bytes
    int j;
    unsigned char index_start = 0;
    unsigned char index_end = 0;
    int i = 0, ix =0;
    int sfp_is_detected = FALSE; 
    unsigned int bus;
    unsigned char vendor[SFP_EEPROM_16_LENGTH]; //16 bytes - address 20 to 35
    unsigned char partnumber[SFP_EEPROM_16_LENGTH]; //16 bytes - address 40 to 55
    unsigned char serial[SFP_EEPROM_16_LENGTH]; //16 bytes - address 68 to 83
    unsigned char date[SFP_EEPROM_8_LENGTH];//8 bytes - address 84 to 91 
    int cwdm_wave;
    
    memset(&i2c_if, 0, sizeof(i2c_if));
    bus = 1; // 0x1
    i2c_if.i2c_bus_type = bus;
    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;
    i2c_if.mux = 0; 
    i2c_if.i2c_dev = 0x50; // 0xA0 > 1 = 0x50
    i2c_if.offset = 0;  
    i2c_if.size = 1;  
    memset(A50, 0, sizeof(A50)); 
    i2c_if.buf = (char *)A50;

    /* Check if SFP is available. if yes then continue */
    if (is_sfp_present(&sfp_is_detected) != PASSED) {
        rc = FAILED;
        cterr('f', 0, "Failed to check SFP status.");
        return (rc);
    }
    if (sfp_is_detected != TRUE) {
        printf("SFP module is not detected.");
        return (rc);
    } else {
        printf("SFP module is detected");
    } 
    mdelay(3000); /* wait for SFP ready*/
    printf("\n");
    printf ("Dump eeprom contents to: \n") ;
    printf("     0   1   2   3   4   5   6   7   8   9   a   b  "
            " c   d   e   f   0123456789abcdef");
    for (ix = 0; ix < 256; ix++) {
        i2c_if.offset = ix;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            /*
             * Unable to read data
            */
            printf("%s: Failed to I2C read (rc = 0x%08x).", __FUNCTION__, rc);
            return (rc);
        }
        A50[ix] = *i2c_if.buf & 0xFF; 
        if((ix % 0x10) == 0) {
            index_end = ix;
            printf("  ");
            for(j = index_start; j <index_end; j++) {
                if(A50[index_start] == 0x0 ||
                    A50[index_start] == 0xff)
                    printf(".");
                 else
                 if (A50[index_start] < 32 ||
                     A50[index_start] >=127){
                     printf("?");
                 }
                 else
                     printf("%c",A50[index_start]);
                     index_start++;
            }
            index_start = index_end;
            printf("\n%02x:",i);
	    i = i + 0x10;
        } 
        printf("  %02x", A50[ix]);  
    }

    //print the connector type
    printf("\nConnector Type = %s",connector[A50[2]]);

    //print the transceiver type
    if(A50[6] & 16) {
	    printf("\nTransceiver is 100Base FX");
    } else if(A50[6] & 8) {
	    printf("\nTransceiver is 1000Base TX");
    } else if(A50[6] & 4) {
        printf("\nTransceiver is 1000Base CX");
    } else if(A50[6] & 2) {
        printf("\nTransceiver is 1000Base LX");
    } else if(A50[6] & 1) {
        printf("\nTransceiver is 1000Base SX");
    } else if(A50[3] & 64) {
        printf("\nTransceiver is 10GBase-ER");
    } else if(A50[3] & 32) {
        printf("\nTransceiver is 10GBase-LRM");
    } else if(A50[3] & 16) {
        printf("\nTransceiver is 10GBase-LR");
    } else if(A50[4] & 12) {
        printf("\nTransceiver is 10GBase-ZR");
    } else if(A50[3] & 8) {
        printf("\nTransceiver is 10GBase-SR");
    } else {
        printf("\nTransceiver is unknown");
    }

    //3 bytes.60 high order, 61 low order. Byte 62 is the mantissa
    //print sfp wavelength 
    cwdm_wave = ((int) A50[60]<<8) | ((int) A50[61]);
    printf("\nWavelength = %d.%d",cwdm_wave,A50[62]);

    //print vendor id bytes 20 to 35
    memcpy(&vendor, &A50[20],16);
    vendor[16] = '\0';
    printf("\nVendor = %s",vendor);

    //Print partnumber values address 40 to 55  
    memcpy(&partnumber, &A50[40], 16);
    partnumber[16] = '\0';
    printf("\nPartnumber = %s", partnumber);

    //Print serial values address 68 to 83  
    memcpy(&serial, &A50[68], 16);
    serial[16] = '\0';
    printf("\nSerial = %s", serial);

    //Print date values address 84 to 91  
    memcpy(&date, &A50[84], 8);
    date[8] = '\0';
    printf("\ndate = %s", date);
	
    return (rc);
}

/*
 * Function: get_sfp_reg 
 *
 * Description:
 *   This function to get SFP Reg 
 *
 * Parameters:
 *   Offset -- i2c offset register
 *
 * Returns:
 *   Register value / Failed
 */

int get_sfp_reg (uint offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int rc = PASSED;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = CPU_I2C1;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    i2c_if.i2c_dev = MB_I2C_ADDR_SFP0;

    i2c_if.offset = offset;
    
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (d32[0]);
}

/*
 * Function: read_sfp_ext_id 
 *
 * Description:
 *   Read SFP Extended ID.
 *   (Check if the SFP is GLC-GE-100FX. Refer to ENG-107393.
 *
 * Parameters:
 *  int i2c_bus 
 *
 * Returns:
 *   GLC-GE-100FX or other SFPs Ext. ID
 */

int read_sfp_ext_id (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;
    unsigned int rc = PASSED;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = i2c_bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    i2c_if.i2c_dev = MB_I2C_ADDR_SFP0;

    i2c_if.offset = SFP_COO_GECC;
    
    i2c_if.size = SFP_COO_GECC_L;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 
    
    /* Read Gigabit Ethernet compliance code */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    if (d32[0]) {
        /* Standard GBIC. Not GLC-GE-100FX */
        return (FALSE); 
    }

    /* Read Extended GBIC ID */
    i2c_if.offset = SFP_COO_XID;
    
    i2c_if.size = SFP_COO_XID_L;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
    }
    
    return (d32[0]);  
}

/*-------------------------------------------------
 * $Log: platform_sfp_cookie.c,v $
 * Revision 1.4  2018/05/30 10:06:57  steja
 * <CSCvj57981>Enhance SFP read Ext.ID functionality to be reuse.
 *
 * Revision 1.3  2018/05/24 09:47:10  steja
 * CSCvj57981-Enhance SFP GLC-GE-100FX Support
 *
 * Revision 1.2  2017/08/02 14:21:49  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.6.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.4.3  2017/07/21 10:46:03  steja
 * Update based on code review comment
 *
 * Revision 1.1.4.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.2.1  2016/09/08 13:45:19  steja
 * Add Utilitiy to read SFP eeprom
 *
 *
*/

/* $Id: phy_common.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/phy_common.c,v $
 *-----------------------------------------------------------------------------
 * phy_common.c - Leverage from BCM API
 * BCM82757_SW/MIUR_1_1/miura_reference_app/phy_common.c
 *
 * August 2017, meho
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

/*
* This reference program was intented to show how to use BCM APIs to configure miura.
* This reference program may not work in different environments.
*/

#include <stdio.h>      /* standard input / output functions */
#include <stdlib.h>
#include <string.h>     /* string function definitions*/
#include <unistd.h>     /* UNIX standard function definitions */
#include <fcntl.h>      /* File control definitions */
#include <errno.h>      /* Error number definitions */
#include <termios.h>    /* POSIX terminal control definitions */
#include "miura_common.h"

#include "bcm82752_api.h"

/* Open USB devices */
int device_open_init()
{
    DWORD NumDevs = 1, k = 0;
    int ftStatus = 0 ,i = 0;
    int rv = 0, p_ctxt = 5;
    unsigned int data;

    FT_DEVICE_LIST_INFO_NODE *devInfo;
    //ftStatus = FT_CreateDeviceInfoList(&NumDevs);
    if (ftStatus == FT_OK) 
    {
        if (NumDevs > 0) 
        {
            devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * NumDevs);
            //ftStatus = FT_GetDeviceInfoList(devInfo, &NumDevs);
            if (ftStatus == FT_OK) 
            {
                for (k = 0; k < NumDevs; k ++) 
                {
                    printf("Index = [%d], SerialNumber = [%s], Description = [%s]\n",k, devInfo[k].SerialNumber, devInfo[k].Description);
                    
                    /* Open First USB Device based on USB Serial Number */                    
                    if (!strcmp("FT1B6CZV",devInfo[k].SerialNumber)) 
                    {
                        //ftStatus = FT_Open(k, &ftHandle[0]);
                        if (ftStatus != 0) 
                        {
                            printf("Error in opening device\n");
                            return ftStatus;
                        } 
                        else 
                        {
                            for (i = 0; i <= 2;i++ ) 
                            {
                                rv = mdio_read((void *)&p_ctxt, i, 0x18b00, &data);
                                if (rv == 0) 
                                {
                                    printf("Chip ID for PHY:0x%x is : 0x%x USB Dev:%d\n", i, data, k);
                                }
                                else 
                                {
                                    data = 0;
                                    printf("mdio_read failed\n");
                                }
                            }
                        }
                        break;
                    }
                }
            } 
            else 
            {
                printf("Error in opening device(s)\n");
                return ftStatus;
            }
        }
        else 
        {
            printf("Error in opening device(s)\n");
            return ftStatus;
        }
    }  
    else 
    {
        printf("Error in opening device(s)\n");
        return ftStatus;
    }
    return ftStatus;
}

/* MDIO read */
int mdio_read(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data)
{
    int ftStatus=0;
    int dev_id = (reg_addr >> 16) & 0x1F;
    int regnum = reg_addr & 0xFFFF;
    *data = bcm82752_reg_rd(mdio_addr, dev_id, regnum);
    if (*data < 0) {
        printf("Failed to read 10GE PHY, TE%d, %d.0x%x\n", mdio_addr, dev_id, regnum);
    } else {
        //printf("mdio_addr%d, %d.%#.4x = %#.4x \n", mdio_addr, dev_id, regnum, *data);
    }
#if 0
    int ftStatus;
    DWORD BytesRW;
    FT_HANDLE read_fth;

    /* Here we have 2 PHYs on same MDIO bus at mdio_address 0 and 4 resp */
    /* mdio_addr = 4 is added for Broadcast support.
       In current test setup, First PHY_ID is at mdio_aadr = 0 and Second PHY_ID (on same MDIO bus) is at mdio_Addr = 4 
    */
    if((mdio_addr == 0) || (mdio_addr == 4) )
    {       
        read_fth = ftHandle[0];
    }
    else if(mdio_addr == 1) 
    {
        read_fth = ftHandle[1];
        mdio_addr = 8; /* PHY_ID on Second USB device is at mdio_addr = 8 */
    } 

    unsigned char port_addr = (unsigned char) mdio_addr;
    unsigned char sbuf[6] = {0}, rbuf[2] = {0};
    unsigned char dev_addr = (reg_addr >> 16) & 0x1f;
    if (!user_acc) 
    {
        return -1;
    }

    sbuf[0] = port_addr >> 1;
    sbuf[1] = ((port_addr & 1)  << 7) | ((dev_addr & 0x1F) << 2) | 2;
    sbuf[2] = (reg_addr >> 8);
    sbuf[3] = (reg_addr & 0xFF);
    sbuf[4] = sbuf[0] | 0x30;
    sbuf[5] = sbuf[1];
    ftStatus = FT_Write(read_fth, sbuf, sizeof(sbuf), &BytesRW);
    if (ftStatus == FT_OK) 
    {
        ftStatus = FT_Read(read_fth, rbuf, sizeof(rbuf), &BytesRW);
        if (ftStatus == FT_OK) 
        {
            *data   = rbuf[0];
            *data <<= 8;
            *data  |= rbuf[1];
        }
    }
#endif
    return ftStatus;
}

/* MDIO Write */
int mdio_write(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int data)
{
    int ftStatus=0;
    int dev_id = (reg_addr >> 16) & 0x1F;
    int regnum = reg_addr & 0xFFFF;
    ftStatus = bcm82752_reg_wr(mdio_addr, dev_id, regnum, data);
    if (ftStatus < 0) {
        printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", mdio_addr, dev_id, regnum);
    } else {
        //printf("mdio_addr%d, %d.%#.4x <-- %#.4x \n", mdio_addr, dev_id, regnum, data);
    }
#if 0
    int ftStatus;
    DWORD BytesRW;
    FT_HANDLE write_fth;
   
    /* Here we have 2 PHYs on same MDIO bus at mdio_address 0 and 4 resp */
    /* mdio_addr = 4 is added for Broadcast support.
       In current test setup, First PHY_ID is at mdio_aadr = 0 and Second PHY_ID (on same MDIO bus) is at mdio_Addr = 4 
    */
    if( (mdio_addr == 0) || (mdio_addr == 4))
    {
        write_fth = ftHandle[0];
    }
    else if(mdio_addr == 1)
    {
        write_fth = ftHandle[1];
        mdio_addr = 8; /* PHY_ID on Second USB device is at mdio_addr = 8 */
    }

    unsigned char sbuf[8] = {0};
    unsigned char dev_addr = (reg_addr >> 16) & 0x1f;
    unsigned char port_addr = (unsigned char) mdio_addr;
    if (!user_acc)
    {
        return -1;
    }

    data = data & 0xFFFF;
    sbuf[0] = port_addr >> 1;
    sbuf[1] = ((port_addr & 1)  << 7) | ((dev_addr & 0x1F) << 2) | 2;
    sbuf[2] = (reg_addr >> 8);
    sbuf[3] = (reg_addr & 0xFF);
    sbuf[4] = sbuf[0] | 0x10;
    sbuf[5] = sbuf[1];
    sbuf[6] = (data >> 8);
    sbuf[7] = (data & 0xFF);
    ftStatus = FT_Write(write_fth, sbuf, sizeof(sbuf), &BytesRW);
#endif
    return ftStatus;
}

int macsec_initialize(bcm_plp_sec_phy_access_t sec_info, int bypass )
{
    int secy_rc, cfye_rc; /* Return code */
    bcm_plp_cfye_init_t init_settings;
    memset(&init_settings, 0, sizeof(bcm_plp_cfye_init_t));

	/* If bypass flag is enable, set Cfye to bypass mode */
    if (bypass)
        init_settings.flow_latency_bypass = 1;
 
    cfye_rc = bcm_plp_cfye_device_init("miura",
                &sec_info,
                    &init_settings
                );

    if (cfye_rc != BCM_PLP_CFYE_STATUS_OK)
    {
        test_debug_msg(sec_info.phy_info, "bcm_plp_cfye_device_init API failed", cfye_rc);
        return cfye_rc;
    } 
    else 
    {
        test_debug_msg(sec_info.phy_info, "bcm_plp_cfye_device_init API success", cfye_rc);
    }

    bcm_plp_secy_settings_t settings;
    memset(&settings, 0, sizeof(bcm_plp_secy_settings_t));
    
    if (bypass)
    {
        settings.drop_bypass.fbypass = 1;   
    }
    else
    {
        settings.drop_bypass.drop_type = BCM_PLP_SECY_SA_DROP_INTERNAL;    
    }
    
    /*Initializes a SecY device_id instance identified by IntefaceId parameter.*/
    secy_rc = bcm_plp_secy_device_init("miura",
                                        &sec_info,
                                        &settings
                                    );
    if (secy_rc != BCM_PLP_SECY_STATUS_OK)
    {
        test_debug_msg(sec_info.phy_info, "bcm_plp_secy_device_init API failed", secy_rc);
        return secy_rc;
    } 
    else 
    {
        test_debug_msg(sec_info.phy_info, "bcm_plp_secy_device_init API success", secy_rc);
    }
    
    return (secy_rc || cfye_rc);
}

void test_debug_msg(bcm_plp_access_t phy_info, char * name, int rv)
{
    if(DEBUG_ENABLE)
    {
        printf("%s: %s on lane 0x%x %s phy_id %d rv[%d]\n",rv? "FAILED":"PASSED", name, phy_info.lane_map,
                phy_info.if_side ? "System side": "Line side", phy_info.phy_addr,rv);
    }
}

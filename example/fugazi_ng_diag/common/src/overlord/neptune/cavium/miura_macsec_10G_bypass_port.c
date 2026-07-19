/* $Id: miura_macsec_10G_bypass_port.c,v 1.3 2018/06/07 01:35:36 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/miura_macsec_10G_bypass_port.c,v $
 *-----------------------------------------------------------------------------
 * miura_macsec_10G_bypass_port.c - Leverage from BCM API
 * BCM82757_SW/MIUR_1_1/miura_reference_app/miura_macsec_10G_bypass_port.c
 *
 * August 2017, meho
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Script: miura_macsec_10G_bypass_port.c
This script deals with bypassing Secy and Cfey functionality for each port on 10G.
The flow of test case is below:
1. Find USB id 
2. Initialize miura chip.
3. Initialize Macsec (Secy and Cfye). Set Cfye param to bypass.
4. Set Secy Config mode per port
5. Set mode config on Line and System side
6. Set Secy to bypass mode (per port)
7. Send Traffic.

Note: Please read appropriate statistics to validate 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
* This reference program was intented to show how to use BCM APIs to configure miura.
* This reference program may not work in different environments.
*/

#include "miura_common.h"
#include <stdlib.h>
#include <string.h>
#include "miura_config.h"
#include <unistd.h>
#include "bcm82752_api.h"
/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define uint32_t unsigned int 
#define uint8_t  unsigned short int
#define EGRESS    0
#define INGRESS   1
#define MAX_MACSEC_SIDE_ALLOWED 2

/*----------------------------------------------------------------------------
 * Local variables
 */

bcm_plp_access_t plp_info;
bcm_plp_sec_phy_access_t sec_info;
extern int bcm8275x_hw_init_done;

int miura_fw_download()
{
    unsigned int phy_id = 0;
    int rv = 0, macsec_side = 0;
    int lane, phy;
    int speed = 10000; /* speed - 10G */

    memset(&sec_info, 0, sizeof(sec_info));
    memset(&plp_info, 0, sizeof(plp_info));
    
    plp_info.platform_ctxt = (void*)5;
    
    /*++++++++++++++++++++++++++++++++++++++++++
    Open device and load firmware through MDIO
    ++++++++++++++++++++++++++++++++++++++++++++*/
    rv = device_open_init();
    if (rv != 0) 
    {
        //FT_Close(ftHandle[0]);
        printf("Device open fails\n");
        return rv;
    }

    /* Initialize chip and download Firmware (using Unicast) */
    bcm_plp_firmware_load_type_t firmware_load_type;
    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;
    
    plp_info.lane_map = LANE_MAP;
    plp_info.if_side = LINE_SIDE;

    for(phy_id = 0; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        rv = bcm_plp_init_fw_bcast("miura", plp_info, mdio_read, mdio_write , &firmware_load_type, bcmpmFirmwareBroadcastNone);
        if(rv)
        {
            printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option failed for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rv);
            return rv;
        } 
        printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option Success for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rv);
    }

    /*+++++++++++++++++++++++++++ */
    /* MACSec init*/
    /*Initialize Device CfyE and SecY */    
    plp_info.lane_map = LANE_MAP;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));

        /* Egress: 0 ; Ingress: 1 */
        for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
        {
            sec_info.macsec_side = macsec_side;
            rv = macsec_initialize(sec_info, 1 /* set to Bypass */);
            if (rv)
            {
                printf("FAILED: MACSec Initialize failed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
                return rv;
            }
            printf("PASSED: MACSec Initialize passed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
        }
    }

    /* Set Secy Config set for Both Egress and Ingress device */

    /* Set PHY-0 as Egress and PHY-1 as Ingress */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        sec_info.macsec_side = EGRESS;
        
        /* Secy Config set */
        rv = bcm_plp_secy_config_set("miura", &sec_info);
        if (rv)
        {
            printf("bcm_plp_secy_config_set failed for device-id [%d], return code [%d] \n", sec_info.macsec_side, rv);
            return rv;
        }
    }

    /*Ingress */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = INGRESS;
        
        /* Secy Config set */
        rv = bcm_plp_secy_config_set("miura", &sec_info);
        if (rv)
        {
            printf("bcm_plp_secy_config_set failed for device-id [%d], return code [%d] \n", sec_info.macsec_side, rv);
            return rv;
        }
    }
    
    /* Mode Config set for each port */
    {
        int side;
        plp_info.lane_map = 0x1;
        for (phy =0 ; phy < 1 ;phy ++) 
        {
            int aux; 
            plp_info.phy_addr = 0;
            for (lane = 0; lane < 2; lane ++) 
            {
                plp_info.lane_map = 1 << lane;
                for (side = 0; side <=1 ; side ++) 
                {
                    plp_info.if_side = side;
                    printf("Lane MAP:%x\n", plp_info.lane_map);
                    rv = bcm_plp_mode_config_set("miura", plp_info, speed, bcm_pm_InterfaceXFI, bcm_pm_RefClk156Mhz, bcm_pm_Interface_mode_IEEE, &aux);
                    if (rv != 0) 
                    {
                        printf("Error in setting Config\n");
                        return rv;
                    }
                }
            }
        }
    }

    /* Egress */
    /* Set SECY to Bypass */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = EGRESS;
        
        /* Secy Bypass */       
        rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
        if (rv)
        {
            printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", plp_info.phy_addr, sec_info.macsec_side, rv);
            return rv;
        }
    }
    
    /* Ingress */
    /* Set SECY to Bypass */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = INGRESS;
        
        /* Secy Bypass */       
        rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
        if (rv)
        {
            printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", plp_info.phy_addr, sec_info.macsec_side, rv);
            return rv;
        }
    }
    
    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\n Please send packets now. \n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
    
    /* BCM82757 emphasis setting */
    bcm82757_emphasis_setting();
    //FT_Close(ftHandle);
    return rv;
}


int miura_macsec_xfi_lrm_sr_config(int bcm_pm_intf)
{
    unsigned int phy_id = 0;
    int rv = 0, macsec_side = 0;
    int lane;
    int aux;
    bcm_plp_access_t plp_info;
    bcm_plp_sec_phy_access_t sec_info;
    memset(&plp_info, 0, sizeof(plp_info));
    memset(&sec_info, 0, sizeof(sec_info));
    bcm_plp_firmware_load_type_t firmware_load_type;

    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    plp_info.platform_ctxt = (void*)5;

    /*++++++++++++++++++++++++++++++++++++++++++
      Open device and load firmware through MDIO
      ++++++++++++++++++++++++++++++++++++++++++++*/
    rv = device_open_init();
    if (rv != 0) 
    {
        //FT_Close(ftHandle[0]);
        printf("Device open fails\n");
        return rv;
    }

    /* Initialize chip and download Firmware (using Unicast) */
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;
    plp_info.lane_map = LANE_MAP; /*UNUSED*/
    plp_info.if_side = LINE_SIDE; /*UNUSED*/
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        rv = bcm_plp_init_fw_bcast("miura", plp_info, mdio_read, mdio_write , &firmware_load_type, bcmpmFirmwareBroadcastNone);
        if(rv)
        {
            printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option failed for PHY-ID[%d] with return code [%d]\n", phy_id, rv);
            return rv;
        } 
        printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option Success for PHY-ID[%d]\n", phy_id);
    }

    /*+++++++++++++++++++++++++++ */
    /* MACSec init*/
    /*Initialize Device CfyE and SecY */    
    plp_info.lane_map = LANE_MAP; /*UNUSED*/
    plp_info.if_side = LINE_SIDE; /*UNUSED*/
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));

        /* Egress: 0 ; Ingress: 1 */
        for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
        {
            sec_info.macsec_side = macsec_side;
            rv = macsec_initialize(sec_info, 1 /* set to Bypass */);
            if (rv)
            {
                printf("FAILED: MACSec Initialize failed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
                return rv;
            }
            printf("PASSED: MACSec Initialize passed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
        }
    }

    /* Set Secy Config set for Both Egress and Ingress device */
    plp_info.if_side  = LINE_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        for (lane = 0; lane < 2; lane ++) 
        {
            /* Filling plp_info */  
            plp_info.lane_map = 1 << lane;
            memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
            for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
            {
                sec_info.macsec_side = macsec_side;

                /* Secy Config set */
                rv = bcm_plp_secy_config_set("miura", &sec_info);
                if (rv)
                {
                    printf("bcm_plp_secy_config_set failed for device-id [%d], return code [%d] \n", sec_info.macsec_side, rv);
                    return rv;
                }
            }
        }
    }


//// 10G mode only

    /* Mode Config set for line side */
    /* Configuring lane 0, 1 in 10G -LR interface*/
    plp_info.if_side = LINE_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        for (lane = 0; lane < 2; lane ++) 
        {
            plp_info.lane_map = 1 << lane;
            rv = bcm_plp_mode_config_set("miura", plp_info, 10000, bcm_pm_intf, bcm_pm_RefClk156Mhz, bcm_pm_Interface_mode_IEEE, &aux);
            if (rv != 0) 
            {
                printf("Error in setting Config\n");
                return rv;
            }
        }
    }

    /* Mode Config set for system side */
    /* Configuring lane 0, 1 in 10G -KR interface*/
    plp_info.if_side = SYS_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        for (lane = 0; lane < 2; lane ++) 
        {
            plp_info.lane_map = 1 << lane;
            rv = bcm_plp_mode_config_set("miura", plp_info, 10000, bcm_pm_InterfaceXFI, bcm_pm_RefClk156Mhz, bcm_pm_Interface_mode_IEEE, &aux);
            if (rv != 0) 
            {
                printf("Error in setting Config\n");
                return rv;
            }
        }
    }


    /* Set SECY to Bypass */
    plp_info.if_side  = LINE_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        for (lane = 0; lane < 2; lane ++) 
        {
            /* Filling plp_info */  
            plp_info.lane_map = 1 << lane;
            memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
            for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
            {
                sec_info.macsec_side = macsec_side;


                /* Secy Bypass */       
                rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
                if (rv)
                {
                    printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", phy_id, macsec_side, rv);
                    return rv;
                }
            }
        }
    } 


    /* Set SECY to Bypass */
    plp_info.if_side  = LINE_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        for (lane = 0; lane < 2; lane ++) 
        {
            /* Filling plp_info */  
            plp_info.lane_map = 1 << lane;
            memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
            for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
            {
                sec_info.macsec_side = macsec_side;

                /* Secy Bypass */       
                rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
                if (rv)
                {
                    printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", phy_id, macsec_side, rv);
                    return rv;
                }
            }
        }
    } 

    printf("bcm_pm_interface = %d\n", bcm_pm_intf);
    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\n Please send packets now. \n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");

    /* BCM82757 emphasis setting */
    bcm82757_emphasis_setting();
    //FT_Close(ftHandle);
    return rv;
}

int miura_macsec_bypass_1000x()
{
    unsigned int phy_id = 0;
    int rv = 0, macsec_side = 0;
    int lane, phy;
    int speed = 1000; /* speed - 10G */

    memset(&sec_info, 0, sizeof(sec_info));
    memset(&plp_info, 0, sizeof(plp_info));
    unsigned short tech_ability = 0x1; /* 1G_1000X tech ability*/
    unsigned short fec_ability = 0, pause_ability = 0;
    bcm_plp_an_config_t an_config;
    an_config.master_lane = 0; /* master lane is don't care */
    an_config.cl72_en = 1; /* AN with CL72 */
    unsigned int an = 0, an_done = 0;
   
    plp_info.platform_ctxt = (void*)5;
    
    /*++++++++++++++++++++++++++++++++++++++++++
    Open device and load firmware through MDIO
    ++++++++++++++++++++++++++++++++++++++++++++*/
    rv = device_open_init();
    if (rv != 0) 
    {
        //FT_Close(ftHandle[0]);
        printf("Device open fails\n");
        return rv;
    }

    /* Initialize chip and download Firmware (using Unicast) */
    bcm_plp_firmware_load_type_t firmware_load_type;
    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;
    
    plp_info.lane_map = LANE_MAP;
    plp_info.if_side = LINE_SIDE;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        rv = bcm_plp_init_fw_bcast("miura", plp_info, mdio_read, mdio_write , &firmware_load_type, bcmpmFirmwareBroadcastNone);
        if(rv)
        {
            printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option failed for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rv);
            return rv;
        } 
        printf("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone option Success for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rv);
    }

    /*+++++++++++++++++++++++++++ */
    /* MACSec init*/
    /*Initialize Device CfyE and SecY */    
    plp_info.lane_map = LANE_MAP;
    for(phy_id = PHY_ID; phy_id < PHY_ID + NUM_OF_PHY; phy_id++)
    {
        plp_info.phy_addr = phy_id;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));

        /* Egress: 0 ; Ingress: 1 */
        for(macsec_side = 0 ; macsec_side < MAX_MACSEC_SIDE_ALLOWED; macsec_side ++)
        {
            sec_info.macsec_side = macsec_side;
            rv = macsec_initialize(sec_info, 1 /* set to Bypass */);
            if (rv)
            {
                printf("FAILED: MACSec Initialize failed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
                return rv;
            }
            printf("PASSED: MACSec Initialize passed for PHY-ID[%d], device-id [%d] \n", phy_id, sec_info.macsec_side);
        }
    }

    /* Set Secy Config set for Both Egress and Ingress device */

    /* Set PHY-0 as Egress and PHY-1 as Ingress */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        sec_info.macsec_side = EGRESS;
        
        /* Secy Config set */
        rv = bcm_plp_secy_config_set("miura", &sec_info);
        if (rv)
        {
            printf("bcm_plp_secy_config_set failed for device-id [%d], return code [%d] \n", sec_info.macsec_side, rv);
            return rv;
        }
    }

    /*Ingress */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = INGRESS;
        
        /* Secy Config set */
        rv = bcm_plp_secy_config_set("miura", &sec_info);
        if (rv)
        {
            printf("bcm_plp_secy_config_set failed for device-id [%d], return code [%d] \n", sec_info.macsec_side, rv);
            return rv;
        }
    }
    
    /* Mode Config set for each port */
    {
        int side;
        plp_info.lane_map = 0x1;
        for (phy =0 ; phy < 1 ;phy ++) 
        {
            int aux; 
            plp_info.phy_addr = phy;
            for (lane = 0; lane < 2; lane ++) 
            {
                plp_info.lane_map = 1 << lane;
                for (side = 0; side <=1 ; side ++) 
                {
                    plp_info.if_side = side;
                    printf("Lane MAP:%x\n", plp_info.lane_map);
                    rv = bcm_plp_mode_config_set("miura", plp_info, speed, bcm_pm_Interface1000X, bcm_pm_RefClk156Mhz, bcm_pm_Interface_mode_IEEE, &aux);
                    if (rv != 0) 
                    {
                        printf("Error in setting Config\n");
                        return rv;
                    }
                }
            }
        }
    }

    /* Egress */
    /* Set SECY to Bypass */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = EGRESS;
        
        /* Secy Bypass */       
        rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
        if (rv)
        {
            printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", plp_info.phy_addr, sec_info.macsec_side, rv);
            return rv;
        }
    }
    
    /* Ingress */
    /* Set SECY to Bypass */
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        memcpy(&sec_info.phy_info, &plp_info, sizeof(bcm_plp_access_t));
        
        sec_info.macsec_side = INGRESS;
        
        /* Secy Bypass */       
        rv = bcm_plp_secy_bypass_set("miura", &sec_info, 1);
        if (rv)
        {
            printf("bcm_plp_secy_bypass_set failed for PHY-ID [%d], macsec_side [%d], return code [%d] \n", plp_info.phy_addr, sec_info.macsec_side, rv);
            return rv;
        }
    }
    /* AN Configuration start from here*/
    plp_info.if_side = LINE_SIDE;  
    for (phy =0 ; phy < 1 ;phy ++) 
    {
        plp_info.phy_addr = phy;
        for (lane = 0; lane < 2; lane ++) 
        {
            plp_info.lane_map = 1 << lane;
            rv = bcm_plp_cl73_ability_set("miura", plp_info, tech_ability, fec_ability, pause_ability, an_config);
            if (rv != 0) 
            {
                printf("Ability set failed for for PHY-ID [%d], lane_map [0x%x], return code [%d]\n", plp_info.phy_addr, plp_info.lane_map, rv);
                return rv;
            }
        }
    }
    plp_info.if_side = LINE_SIDE;  
    plp_info.flags = BCM_PLP_AN_MODE_CL37; /* Setting flag for CL37*/
    for (phy =0 ; phy < 1 ;phy ++) 
    {
        plp_info.phy_addr = phy;
        plp_info.if_side = LINE_SIDE;  
        for (lane = 0; lane < 2; lane ++) 
        {
            plp_info.lane_map = 1 << lane;
            rv = bcm_plp_cl73_set("miura", plp_info, 1);
            if (rv != 0) 
            {
                printf("AN enable set failed for for PHY-ID [%d], lane_map [0x%x], return code [%d]\n", plp_info.phy_addr, plp_info.lane_map, rv);
                return rv;
            }
        }
    }
    sleep(2);
    plp_info.if_side = LINE_SIDE;  
    for (phy =0 ; phy < 1 ;phy ++) 
    {
        plp_info.phy_addr = phy;
        for (lane = 0; lane < 2; lane ++) 
        {
            an = 0;
            an_done = 0;
            plp_info.lane_map = 1 << lane;
            rv = bcm_plp_cl73_get("miura", plp_info, &an, &an_done);
            if (rv != 0) 
            {
                printf("AN Status get failed for for PHY-ID [%d], lane_map [0x%x], return code [%d]\n", plp_info.phy_addr, plp_info.lane_map, rv);
                return rv;
            }
            printf("AN Status an = [%d] an_done = [%d] for PHY-ID [%d], lane_map [0x%x] \n", an, an_done, plp_info.phy_addr, plp_info.lane_map);
        }
    }

    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\n Please send packets now. \n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
    
    //FT_Close(ftHandle);
    return rv;
}

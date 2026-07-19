/* $Id: quadra28_common.c,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/quadra28_common.c,v $
 *-----------------------------------------------------------------------------
 * quadra28_common.c - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/quadra28_common.c
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "ftd2xx.h"
#include "epdm_v1v2v0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* string function definitions*/
#include <unistd.h>     /* UNIX standard function definitions */
#include <fcntl.h>      /* File control definitions */
#include <errno.h>      /* Error number definitions */
#include <termios.h>    /* POSIX terminal control definitions */
#include "bcm82752_api.h"
#include "bcm_pm_if_api.h"

#define MAX_PHYID 1 //7
int p_ctxt = 5;
int sam_write(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int data);
int sam_read(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data);

/* Defining the chip name */
static char *quadra28="quadra28";
int quadra28_device_open()
{
    DWORD NumDevs = 2, i = 0,j = 0, k = 0;
    int ftStatus = 0;
    int rv = -1;
    unsigned int data=0;
    int found =0;
    plp_quadra28_static_config_t stat;
    plp_quadra28_static_config_t stat_s;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    memset(&stat,0,sizeof(plp_static_config_t));
    memset(&stat_s,0,sizeof(plp_static_config_t));
    if (ftStatus == FT_OK) {
        if (NumDevs > 0) {
            devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * NumDevs);
            if (ftStatus == FT_OK) {
                for (k = 0; k < NumDevs; k ++){
                    printf("NUM dev:%d type:%d ID = %08x usb:%d\n", NumDevs,devInfo[k].Type,devInfo[k].ID,k);
                    for(j = 0 ; j< 32; j++){
                        sam_read(&p_ctxt, j,0x1c802,&data);
                        if(data==0x2780 || data==0x278f || data==0x2758 || data==0x2752){
                           printf("chip found 0x%x\n",data); 
                           found=1;
                           break;
                        } 
                    }
                    if(found)
                        break;
                }
                bcm_plp_firmware_load_type_t fw_load_type;
                bcm_plp_access_t phy_info;
                phy_info.platform_ctxt=&p_ctxt;
                fw_load_type.firmware_load_method=bcmpmFirmwareLoadMethodInternal;
                fw_load_type.force_load_method = bcmpmFirmwareLoadForce;
                stat.ull_dp=0;
                stat.rptr_mode=0;
                stat.an_master_lane=0;
                for(i = 0; i <= MAX_PHYID; i++){
                    phy_info.phy_addr=i;

                /***************************************************************************************** 
                 * Set static configuration
                 * Datapath setting is done through static configuration. 
                 * We can select datapath as normal or ULL datapath  
                 *****************************************************************************************/
                    rv = bcm_plp_quadra28_static_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), (void*)&stat);
                   if(rv != 0)
                        printf("\nbcm_plp_quadra28_static_config_set failed\n");
                    rv = bcm_plp_quadra28_static_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), (void*)&stat);
                    if(rv != 0)
                        printf("\nbcm_plp_quadra28_static_config_get failed\n");
                    else
                    printf("\n stat structure= static.ull_dp=%d\n",stat_s.ull_dp);

                    /***************************************************************************************** 
                     * Enable the broadcast for all phy devices connecoted on same MDIO bus  
                     * Broadcast should be enabled for all devices  
                     *****************************************************************************************/
                    fw_load_type.firmware_load_method=bcmpmFirmwareLoadMethodNone;
                    rv = bcm_plp_quadra28_init_fw_bcast((*(bcm_plp_quadra28_access_t*) (&phy_info)), sam_read, sam_write,
                                                        (bcm_plp_quadra28_firmware_load_type_t*) &fw_load_type, bcmpmFirmwareBroadcastNone);
                    if(rv != 0){
                        printf("Bcast enable init failed for phy_id %d rv %d\n",phy_info.phy_addr,rv);
                    }
                }
            }
        } 
    }  
    return ftStatus;
}

int sam_write(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int data)
{
    int ftStatus = 0;
    int dev_id = (reg_addr >> 16) & 0x1F;
    int regnum = reg_addr & 0xFFFF;
    ftStatus = bcm82752_reg_wr(mdio_addr, dev_id, regnum, data);
    if (ftStatus < 0) {
        printf("Failed to write 10GE PHY, TE%d, %d.0x%x\n", mdio_addr, dev_id, regnum);
    } else {
        //printf("TE%d, %d.%#.4x <-- %#.4x \n", mdio_addr, dev_id, regnum, data);
    }

    return ftStatus;
}

int sam_read(void *user_acc, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data)
{
    int ftStatus = 0;
    int dev_id = (reg_addr >> 16) & 0x1F;
    int regnum = reg_addr & 0xFFFF;
    *data = bcm82752_reg_rd(mdio_addr, dev_id, regnum);
    if (*data < 0) {
        printf("Failed to read 10GE PHY, TE%d, %d.0x%x\n", mdio_addr, dev_id, regnum);
    } else {
        //printf("TE%d, %d.%#.4x = %#.4x \n", mdio_addr, dev_id, regnum, *data);
    }

    return ftStatus;
}

int bcm_reg_read(void *p_ctxt,int if_side,unsigned int phy_id,unsigned int lane,unsigned int dev_id,unsigned int *reg_addr,unsigned int *val,int n)
{
    int i,rv;
    int lane_index;
    unsigned int v;
    bcm_plp_access_t phy_info;
    phy_info.phy_addr=phy_id;
    phy_info.lane_map = lane;
    phy_info.platform_ctxt = (void*)&p_ctxt;
    phy_info.if_side = if_side;
    /* Slelect the side  */
    if(if_side==1){
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
        v |=1;
        rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
    }else{
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
        v &= ~(1);
        rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
    }
    /* Slelect the channel based on lane */
    if(lane == 0xf)
    {
        /*Set the address extension register*/  
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc702,&v);
        v &= ~(0xf);
        rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc702,v);
        /*Enable the broadcast */ 
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc712,&v);
        v |= 1;
        rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc712,v);


    }else{     /*Select the channel based on lane number */
        for(lane_index=0;lane_index<4;lane_index++){
            if(lane & (1<<lane_index)){
                rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc702,&v);
                v |= lane_index;
                rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc702,v);
            } 
        }
    }
    /* Get the value  */ 
    for(i=0;i<n;i++)
    {
        rv= bcm_plp_reg_value_get(quadra28,phy_info,dev_id,reg_addr[i],&val[i]);
        printf("phy_id=%d  reg_addr=0x%x    val=0x%x\n",phy_id,reg_addr[i],val[i]);
    }
    /* reset the broadcast and line side interface   */
    rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
    v &= ~(1);
    rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
    rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc712,&v);
    v &=~(1);
    rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc712,v);
    return rv;  
}
int bcm_reg_write(void *p_ctxt,int if_side,unsigned int phy_id,unsigned int lane,unsigned int dev_id,unsigned int *reg_addr,unsigned int *val,int n)
{
    int i,rv;
    int lane_index;
    unsigned int v;
    bcm_plp_access_t phy_info;
    phy_info.phy_addr=phy_id;
    phy_info.lane_map = lane;
    phy_info.platform_ctxt = (void*)&p_ctxt;
    phy_info.if_side = if_side;
    /* Slelect the side  */
    if(if_side==1){
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
        v |=1;
        rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
        printf("side_select=%d\n",v);
    }else{
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
        v &= ~(1);
        rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
        printf("side_select=%d\n",v);
    }
    /* Slelect the channel based on lane */
    if(lane == 0xf)
    {
        /*Set the address extension register*/  
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc702,&v);
        v &= ~(0xf);
        rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc702,v);
        /*Enable the broadcast */ 
        rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc712,&v);
        v |= 1;
        rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc712,v);


    }else{     /*Select the channel based on lane number */
        for(lane_index=0;lane_index<4;lane_index++){
            if(lane & (1<<lane_index)){
                rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc702,&v);
                v |= lane_index;
                rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc702,v);
                printf("channel_select=%d\n",v);
            } 
        }
    }
    /* Set the value  */ 
    for(i=0;i<n;i++)
    {
        rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,reg_addr[i],val[i]);
        printf("phy_id=%d reg_addr=0x%x    val=0x%x\n",phy_id,reg_addr[i],val[i]);
    }
    /* reset the broadcast and line side interface   */
    rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xffff,&v);
    v &= ~(1);
    rv= bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xffff,v);
    rv=bcm_plp_reg_value_get(quadra28,phy_info,dev_id,0xc712,&v);
    v &=~(1);
    rv=bcm_plp_reg_value_set(quadra28,phy_info,dev_id,0xc712,v);
    return rv;  
}

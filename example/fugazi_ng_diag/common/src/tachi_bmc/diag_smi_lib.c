/* $Id: diag_smi_lib.c,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_smi_lib.c,v $ 
 *------------------------------------------------------------------
 *
 * diag_smi_lib.c - smi Library
 *
 * June 2015, Ben Chen
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "common.h"
#include "error.h"
#include "types.h"
#include "diag_smi_lib.h"

#include <netinet/in.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <sys/io.h>
#include "nvmonvars.h"

int diag_smi_reg_read(char *, ulong , ulong, ulong *);
int diag_smi_reg_write(char *, ulong , ulong, ulong);
int diag_smi_6320_reg_read(char *, int, int, int, int *);
int diag_smi_6320_reg_write(char *, int, int, int, int);


/**********************************************************************
 *
 * Function:	diag_smi_read_fn
 *
 * This function: reads a register of specified page.
 *
 * Input:	dev_name :interface name
 *              phy_addr : phy id
 *              reg_addr: register address
 *              buf: data buf
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int diag_smi_reg_read(char *dev_name, ulong phy_addr, ulong reg_addr, ulong *buf)
{
    int retval = PASSED;
    struct ifreq ifr;
    int sock;
    struct mii_ioctl_data *mii_data;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: Failed to create ioctl socket\n", __FUNCTION__);
        retval = FAILED;
    }

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

    /* Load ifr with MII/Phy details */
    if (ioctl(sock, SIOCGMIIPHY, &ifr) == -1) {
        close (sock);
        printf("%s: Retrieve the interface index fails!\n", __FUNCTION__);
        retval = FAILED;
    }

    mii_data = (struct mii_ioctl_data *)(&ifr.ifr_data);
    mii_data->phy_id = phy_addr;
    mii_data->reg_num = reg_addr;

    /* Read the register */
    if (ioctl(sock, SIOCGMIIREG, &ifr) == -1) {
        close (sock);
        printf("%s: Read from register fails!\n", __FUNCTION__);
        retval = FAILED;
    }

    *buf = mii_data->val_out;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("phy_id is 0X0%x, reg_num is 0X%x and value is 0X%x",
                mii_data->phy_id, mii_data->reg_num, mii_data->val_out);
    }
    close(sock);

    if(retval == FAILED){
        cterr('f', 0, "smi read fn failed.");        
    }

    return (retval);
}

/**********************************************************************
 *
 * Function:	diag_smi_write_fn
 *
 * This function: writes a register of specified page.
 *
 * Input:	dev_name :interface name
 *              phy_addr : phy id
 *              reg_addr: register address
 *              buf: data buf
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int diag_smi_reg_write(char *dev_name, ulong phy_addr, ulong reg_addr, ulong buf)
{
    int retval = PASSED;
    struct ifreq ifr;
    int sock;
    struct mii_ioctl_data *mii_data;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: Failed to create ioctl socket\n", __FUNCTION__);
        retval = FAILED;
    }

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

    /* Load ifr with MII/Phy details */
    if (ioctl(sock, SIOCGMIIPHY, &ifr) == -1) {
        close (sock);
        printf("%s: Retrieve the interface index fails!\n", __FUNCTION__);
        retval = FAILED;
    }

    mii_data = (struct mii_ioctl_data *)(&ifr.ifr_data);
    mii_data->phy_id = phy_addr;
    mii_data->reg_num = reg_addr;
    mii_data->val_in = buf;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Try to write %x into phy_id is 0X0%x, reg_num is 0X%x\n",
                             mii_data->val_in, mii_data->phy_id, mii_data->reg_num);
    }

    /* Write into register now */
    if (ioctl(sock, SIOCSMIIREG, &ifr) == -1) {
        close (sock);
        printf("%s: Write to register fails!\n", __FUNCTION__);
        retval = FAILED;
    }
    close(sock);

    if(retval == FAILED){
        cterr('f', 0, "smi write fn failed.");
    }
    return (retval);
}


/**********************************************************************
 *
 * Function:	diag_smi_6320_reg_read
 *
 * This function: Read a register of specified page.
 *
 * Input:	dev_name :interface name
 *              id : phy id
 *              port: port num
 *              reg: register
 *              data: value of register
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int diag_smi_6320_reg_read(char *dev_name, int id, int port, int reg, int *data)
{
	ulong reg_d;
    int retval = PASSED;

    /* 1. cmd register */
	reg_d = MRVL6320_SMI_BUSY_MODE | MRVL6320_SMI_READ | MRVL6320_DEV_ADDR(port)
            | MRVL6320_REG_ADDR(reg) ;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        int temp;
        temp = (int)reg_d;
	    printf("Access SMI CMD REG Write Port %d, Register %x and Value %x\n",
                                         port, reg, temp);
    }
    
    if ( diag_smi_reg_write(PHY_DEVICE_NAME, id, SMI_CMD_REG, reg_d) == FAILED ) {
        retval = FAILED;
    }
	
    /* 2. data register */
    if ( diag_smi_reg_read(PHY_DEVICE_NAME, id, SMI_DATA_REG, &reg_d) == FAILED ) {
	    retval = FAILED;
	}

    *data = (int)reg_d;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("data value is %x\n",*data);
    }

    if(retval == FAILED){
        cterr('f', 0, "6320 read fn failed.");
    }
    return (retval);
}

/**********************************************************************
 *
 * Function:   diag_smi_6320_reg_write	
 *
 * This function: writes a register of specified page.
 *
 * Input:	dev_name :interface name
 *              id : phy id
 *              smi_op: smi operation
 *              port: port num
 *              reg: register
 *              data: value of register
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int diag_smi_6320_reg_write(char *dev_name, int id, int port, int reg, int data)
{
    int retval = PASSED;
    ulong reg_d;
    reg_d = (unsigned long)data;

    /* 1. data register */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Access SMI DATA REG Write Port %d, Register %x and Value %x\n",
                                      port, reg, data);
    }

    if ( diag_smi_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                            SMI_DATA_REG, data) == FAILED ) {
        retval = FAILED;
    }

	/* 2. cmd register */
    reg_d = MRVL6320_SMI_BUSY_MODE | MRVL6320_SMI_WRITE
            | MRVL6320_DEV_ADDR(port) | MRVL6320_REG_ADDR(reg) ;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        int temp;
        temp = (int)reg_d;
        printf("Access SMI CMD REG Write Port %d, Register %x and Value %x\n",
                                         port, reg, temp);
    }
    if ( diag_smi_reg_write(PHY_DEVICE_NAME, id, SMI_CMD_REG, reg_d)
                            == FAILED ) {
        retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "6320 write fn failed.");
    }

    return (retval);
}

/*---------------------------------------------------------------
$Log: diag_smi_lib.c,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/14 05:49:25  benchen2
Add verbose flag

Revision 1.1.2.1  2015/07/31 07:21:32  hondwang
smi lib

$Endlog$
*/

/* $Id: diag_mcu_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_mcu_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_mcu_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "nvmonvars.h"
#include "types.h"
#include "common.h"
#include "common_utils.h"
#include "diag_i2c_lib.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "diag_mcu_util.h"
#include "diag_mcu_lib.h"
#include "byteswap.h"
#include "defs.h"
#include "linux_api.h"
#include "free.h"
#include "diag_moka_fpga_lib.h"


/* local MCU function */
static uint32_t mcu_read_mem_cmd_bl(int, uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_write_mem_cmd_bl(int, uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_go_addr_cmd_bl(int, uint8_t *);
static uint32_t mcu_en_upgrade_bl(int);
static uint32_t mcu_check_ack_bl(int, uint8_t *);
static uint32_t mcu_write_mem_set_bl_f1(int, uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_write_mem_set_bl_f2(int, uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_write_mem_set_bl_f3(int, uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_read_mem_set_bl_f1(int , uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_read_mem_set_bl_f2(int , uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_read_mem_set_bl_f3(int , uint8_t *, uint16_t, uint8_t *);
static uint32_t mcu_go_addr_set_bl(int, uint8_t *);

static int verify_pwr_seq_fw(void);

static uint16_t start[MAX_REGION]   = {0x8000, 0x1000};
static uint16_t end[MAX_REGION]     = {0xFC00, 0x1400};
boolean byswap=FALSE;
int gdev_mcu;

/*******************************************************************************
 *
 * Function    : mcu_volcur_check
 * Description : Function to show MCU 
 *          VP12P0 Voltage: Latest Reading:    (0x0020),data:0x2f9e.
 *          Main Board 12V Current: Latest Reading:    (0x0058),data:0x026d.
 * Inputs      : void
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int mcu_volcur_check(void)
{
    uint16_t reg_val = 0;

    if (plat_mcu_reg_rd(PLAT_MCU_VP12P0_VOLTAGE_REG, &reg_val) != PASSED) {
        printf("Could not read VP12P0 Voltage reg.\n");
        return (FAILED);
    }
    printf("MCU VP12P0 Voltage: (0x0020),data:(0x%02X)-> %3.3f Volts.\n", 
            reg_val,(((float)(reg_val)))/THOUSAND );

    if (plat_mcu_reg_rd(PLAT_MCU_MB12V_CUR_REG, &reg_val) != PASSED) {
        printf("Could not read Main Board 12V Current reg.\n");
        return (FAILED);
    }
    printf("MCU Main Board 12V Current: (0x0058),data:(0x%02X)-> %1.3f Amps.\n",
            reg_val,(((float)(reg_val)))/THOUSAND );

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : mcu_fw_verno
 * Description : Function to get MCU version.
 * Inputs      : void
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int mcu_fw_verno(void)
{
    uint32_t reg_offset = 0;
    uint16_t reg_val = 0;

    if (plat_mcu_reg_rd(PLAT_MCU_VERSION_REG, &reg_val) != PASSED) {
        printf("Could not check MCU version, please ensure MCU under APP mode.\n");
        return (FAILED);
    }
    printf("\nMCU VERSION reg.(0x%02X) = 0x%04X\n", reg_offset, reg_val);

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : mcu_read_mem_set_bl_f1 
 * Description : Function to read MCU memory under bootloader mode function 1
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                len  : read data count
 *                data : read data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_read_mem_set_bl_f1 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_READ_F1_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* 1. <S><SlaveID-W><A><RM_Command><A><P>                  */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = RM_COMMAND; 
    
    /* 2. <S><SlaveID-W><A><Neg-RM_Command><A><P>              */
    (e2prom_data.msgs[1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[1]).buf[0] = ENV_RM_COMMAND; 
  
    /* 3. <S><SlaveID-R><A><ACK/NACK><A><P>// Check Ack(0x79)  */ 
    (e2prom_data.msgs[2]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[2]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[2]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[2]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET FUN 1.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_read_mem_set_bl_f2 
 * Description : Function to read MCU memory under bootloader mode function 2
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                len  : read data count
 *                data : read data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_read_mem_set_bl_f2 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    uint8_t maddrsum = 0;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_READ_F2_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* 1. <S><SlaveID-W><A><ADDR3><A><P>       //MSB           */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = *(maddr+3); 
  
    /* 2. <S><SlaveID-W><A><ADDR2><A><P>                       */
    (e2prom_data.msgs[1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[1]).buf[0] = *(maddr+2); 
  
    /* 3. <S><SlaveID-W><A><ADDR1><A><P>                       */
    (e2prom_data.msgs[2]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[2]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[2]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[2]).buf[0] = *(maddr+1); 
  
    /* 4. <S><SlaveID-W><A><ADDR0><A><P>                       */
    (e2prom_data.msgs[3]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[3]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[3]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[3]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[3]).buf[0] = *(maddr); 
 
    /* 5. <S><SlaveID-W><A>< ADDR CheckSum ><A><P>             */
    maddrsum = (*(maddr+3)) ^ (*(maddr+2)) ^ (*(maddr+1)) ^ (*(maddr));
    
    (e2prom_data.msgs[4]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[4]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[4]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[4]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[4]).buf[0] = maddrsum; 
   
    /* 6. <S><SlaveID-R><A><ACK/NACK><A><P> // Check Ack(0x79) */
    (e2prom_data.msgs[5]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[5]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[5]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[5]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[5]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET function 2.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : mcu_read_mem_set_bl_f3 
 * Description : Function to read MCU memory under bootloader mode function 3
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                len  : read data count
 *                data : read data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_read_mem_set_bl_f3 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_READ_F3_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* 1. <S><SlaveID-W><A><RM_ DataCount ><A><P>             */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = (mlen - 1); 
  
    /* 2. <S><SlaveID-W><A>< Neg-DataCount ><A><P>            */
    (e2prom_data.msgs[1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[1]).buf[0] = ~(mlen - 1); 
  
    /* 3. <S><SlaveID-R><A><ACK/NACK><A><P>// Check Ack(0x79) */
    (e2prom_data.msgs[2]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[2]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[2]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[2]).buf[0] = 0; 
  
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET function 2.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_read_mem_cmd_bl 
 * Description : Function to read MCU memory under bootloader mode
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                len  : read data count
 *                data : read data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_read_mem_cmd_bl (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret, ix; 

    /* Read command have two I2C single */
    e2prom_data.nmsgs = mlen;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* Creat read data msgs  */
    for (ix = 0; ix < mlen; ix++) {
        (e2prom_data.msgs[ix]).len = DIAG_MCU_COMMAND_SIZE_1;
        (e2prom_data.msgs[ix]).addr = DIAG_MCU_BLMODE_ADD;
        (e2prom_data.msgs[ix]).flags = DIAG_MCU_FLAG_READ;
        (e2prom_data.msgs[ix]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
        (e2prom_data.msgs[ix]).buf[0] = 0; 
    }
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd, I2C_RDWR, (unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU read command.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
       for (ix = 0; ix < mlen; ix++) {
           printf("%s:%d e2prom_data.msgs[%d].buf[0] = 0x%2x.\n",
                   __FUNCTION__, __LINE__,ix,(e2prom_data.msgs[ix]).buf[0]);
       }
    }
    /* Copy read data to mdata */
    for (ix = 0;ix < mlen;ix++) {
        *(mdata + ix) = (e2prom_data.msgs[ix]).buf[0]; 
    }   
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);
 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_write_mem_set_bl_f1 
 * Description : Function to set write MCU memory function 1
 * Inputs      :  
 *                fd   : file description
 *                addr : Address point
 *                len  : write data count
 *                data : write data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_write_mem_set_bl_f1 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_WRITE_F1_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* 1. <S><SlaveID-W><A><WM_Command><A><P>                  */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = WM_COMMAND; 
    
    /* 2. <S><SlaveID-W><A><Neg-WM_Command><A><P>              */
    (e2prom_data.msgs[1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[1]).buf[0] = ENV_WM_COMMAND; 
   
    /* 3. <S><SlaveID-R><A><ACK/NACK><A><P> //Check Ack(0x79)  */
    (e2prom_data.msgs[2]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[2]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[2]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[2]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET FUN 1.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_write_mem_set_bl_f2 
 * Description : Function to set write MCU memory function 2
 * Inputs      :  
 *                fd   : file description
 *                addr : Address point
 *                len  : write data count
 *                data : write data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_write_mem_set_bl_f2 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    uint8_t maddrsum = 0;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_WRITE_F2_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }

    /* 1. <S><SlaveID-W><A><ADDR3><A><P>       //MSB           */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = *(maddr+3); 
  
    /* 2. <S><SlaveID-W><A><ADDR2><A><P>                       */
    (e2prom_data.msgs[1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[1]).buf[0] = *(maddr+2); 
  
    /* 3. <S><SlaveID-W><A><ADDR1><A><P>                       */
    (e2prom_data.msgs[2]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[2]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[2]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[2]).buf[0] = *(maddr+1); 
  
    /* 4. <S><SlaveID-W><A><ADDR0><A><P>                       */
    (e2prom_data.msgs[3]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[3]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[3]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[3]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[3]).buf[0] = *(maddr); 
  
    /* 5. <S><SlaveID-W><A>< ADDR CheckSum ><A><P>             */
    maddrsum = (*(maddr+3)) ^ (*(maddr+2)) ^ (*(maddr+1)) ^ (*(maddr));
    
    (e2prom_data.msgs[4]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[4]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[4]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[4]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[4]).buf[0] = maddrsum; 
    
    /* 6. <S><SlaveID-R><A><ACK/NACK><A><P> ///Check Ack(0x79) */
    (e2prom_data.msgs[5]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[5]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[5]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[5]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[5]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET function 2.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_write_mem_set_bl_f3 
 * Description : Function to set write MCU memory function 3
 * Inputs      :  
 *                fd   : file description
 *                addr : Address point
 *                len  : write data count
 *                data : write data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_write_mem_set_bl_f3 (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_SET_WRITE_F3_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
    
    /* 1. <S><SlaveID-W><A>< DataCount ><A><P>                 */
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = mlen - 1;/* MCU spec datacount is total value - 1 */ 

    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command SET function 3.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_write_mem_cmd_bl 
 * Description : Function to write MCU memory under bootloader mode
 * Inputs      :  
 *                fd   : file description
 *                addr : Address point
 *                len  : write data count
 *                data : write data point (Max 64 byte)
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_write_mem_cmd_bl (int fd, uint8_t *maddr, uint16_t mlen, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    uint8_t mdatasum = 0;
    int ret,ix; 

    e2prom_data.nmsgs = DIAG_MCU_WRITE_S_TIMES + mlen;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
    
    /* Fill Data and Data check sum 9~(9+len) */
    mdatasum = mlen - 1;
    for (ix = 0; ix < mlen; ix++) {
        mdatasum = mdatasum ^ (*(mdata + ix)); 
    }
    /* 1. <S><SlaveID-W><A>< Data0><A><P>                       */
    /* 2. <S><SlaveID-W><A>< DataN><A><P>                       */
    for (ix = 0; ix < mlen; ix++){
        (e2prom_data.msgs[ix]).len = DIAG_MCU_COMMAND_SIZE_1;
        (e2prom_data.msgs[ix]).addr = DIAG_MCU_BLMODE_ADD;
        (e2prom_data.msgs[ix]).flags = DIAG_MCU_FLAG_WRITE;
        (e2prom_data.msgs[ix]).buf = (unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
        (e2prom_data.msgs[ix]).buf[0] = *(mdata + ix);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
       for (ix = 0; ix < mlen; ix++) {
           printf("%s:%d e2prom_data.msgs[%d].buf[0] = 0x%2x.\n",
                   __FUNCTION__, __LINE__,ix,(e2prom_data.msgs[ix]).buf[0]);
       }
    }
    /* 3. <S><SlaveID-W><A>< Data CheckSum ><A><P>              */
    (e2prom_data.msgs[mlen]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[mlen]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[mlen]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[mlen]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[mlen]).buf[0] = mdatasum; 
    
    /* 4. <S><SlaveID-R><A><ACK/NACK><A><P>  // Check Ack(0x79) */
    (e2prom_data.msgs[mlen + 1]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[mlen + 1]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[mlen + 1]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[mlen + 1]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[mlen + 1]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU write command WRITE.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : mcu_go_addr_set_bl 
 * Description : Function set MCU jump to address under bootloader mode
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_go_addr_set_bl (int fd, uint8_t *maddr)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret, ix; 

    e2prom_data.nmsgs = DIAG_MCU_SET_GO_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
   
    /* Fill stable matrix */
    for (ix = 0; ix < DIAG_MCU_SET_GO_S_TIMES; ix++) {
        (e2prom_data.msgs[ix]).len = DIAG_MCU_COMMAND_SIZE_1;
        (e2prom_data.msgs[ix]).addr = DIAG_MCU_BLMODE_ADD;
        (e2prom_data.msgs[ix]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    }

    /* 1. <S><SlaveID-W><A><GT_Command><A><P>                   */
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf[0] = GO_COMMAND; 
   
    /* 2. <S><SlaveID-W><A><Neg-GT_Command><A><P>               */
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf[0] = ENV_GO_COMMAND; 
   
    /* 3. <S><SlaveID-R><A><ACK/NACK><A><P> //Check Ack(0x79)   */
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[2]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU GO command.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);
    
    return (PASSED);
}
/*******************************************************************************
 *
 * Function    : mcu_go_addr_cmd_bl 
 * Description : Function set MCU jump to address under bootloader mode
 * Inputs      : 
 *                fd   : file description
 *                addr : Address point
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_go_addr_cmd_bl (int fd, uint8_t *maddr)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    uint8_t maddrsum = 0;
    int ret, ix; 

    /* Write command only one I2C single */
    e2prom_data.nmsgs = DIAG_MCU_GO_S_TIMES;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
   
    /* Fill stable matrix */
    for (ix = 0; ix < DIAG_MCU_GO_S_TIMES; ix++) {
        (e2prom_data.msgs[ix]).len = DIAG_MCU_COMMAND_SIZE_1;
        (e2prom_data.msgs[ix]).addr = DIAG_MCU_BLMODE_ADD;
        (e2prom_data.msgs[ix]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    }

    /* 1. <S><SlaveID-W><A><ADDR3><A><P>       //MSB            */
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf[0] = *(maddr+3); 
  
    /* 2. <S><SlaveID-W><A><ADDR2><A><P>                        */
    (e2prom_data.msgs[1]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[1]).buf[0] = *(maddr+2); 
  
    /* 3. <S><SlaveID-W><A><ADDR1><A><P>                        */
    (e2prom_data.msgs[2]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[2]).buf[0] = *(maddr+1); 
  
    /* 4. <S><SlaveID-W><A><ADDR0><A><P>                        */
    (e2prom_data.msgs[3]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[3]).buf[0] = *(maddr); 
  
    /* 5. <S><SlaveID-W><A>< ADDR CheckSum ><A><P>             */
    maddrsum = (*(maddr+3)) ^ (*(maddr+2)) ^ (*(maddr+1)) ^ (*(maddr));
    (e2prom_data.msgs[4]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[4]).buf[0] = maddrsum; 

    /* 6. <S><SlaveID-R><A><ACK/NACK><A><P> //Check Ack(0x79)   */
    (e2prom_data.msgs[5]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[5]).buf[0] = 0; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU GO command.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_check_ack_bl 
 * Description : Function check MCU ACK under bootloader mode
 * Inputs      : 
 *                fd   : file description
 *                mdata: reply data
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_check_ack_bl (int fd, uint8_t *mdata)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_COMMAND_SIZE_1;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
    
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_READ;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = DIAG_MCU_CLEAN_BUF; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd, I2C_RDWR, (unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU Enable ACK command.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    *mdata  = (e2prom_data.msgs[0]).buf[0]; 
    free(e2prom_data.msgs);
 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_en_upgrade_bl 
 * Description : Function enable MCU command under bootloader mode
 * Inputs      : 
 *                fd   : file description
 *                 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t mcu_en_upgrade_bl (int fd)
{
    struct i2c_rdwr_ioctl_data e2prom_data;
    int ret; 

    e2prom_data.nmsgs = DIAG_MCU_COMMAND_SIZE_1;
    e2prom_data.msgs = (struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
    if (!e2prom_data.msgs) {
        printf("Fail to malloc mcu e2prom.\n");
        return (FAILED);
    }
    
    (e2prom_data.msgs[0]).len = DIAG_MCU_COMMAND_SIZE_1;
    (e2prom_data.msgs[0]).addr = DIAG_MCU_BLMODE_ADD;
    (e2prom_data.msgs[0]).flags = DIAG_MCU_FLAG_WRITE;
    (e2prom_data.msgs[0]).buf=(unsigned char*)malloc(DIAG_MCU_COMMAND_SIZE_1);
    (e2prom_data.msgs[0]).buf[0] = DIAG_MCU_UPGRADE_ENABLE; 
    
    /* Write Data by IOCTL*/
    ret = ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
    if (ret < MCU_ZERO) {
        printf("Fail to IOCTL MCU Enable command.\n");
        free(e2prom_data.msgs);
        return (FAILED);
    }
    usleep(MCU_UPGRADE_MODE_WAIT);
    free(e2prom_data.msgs);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_fw_upgrade
 * Description : Function to upgragew MCU firmware
 * Inputs      : None
 *               mcu firmware name : mcuimg
 *               must copy mcuimg to /firmware/ folder before run this function
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t mcu_fw_upgrade(void)
{
	int file_desc, rwloop, rwloop_rm;
	int ret, ix, iy;
    uint fwsize;
    uint8_t addr[4]; 
    uint8_t data[DIAG_MCU_RW_MAX_SIZE]; 
    uint8_t *fwdata = (uint8_t *)malloc(DIAG_MCU_AP_SIZE * sizeof(uint8_t)); 
    uint8_t *fwdataop;
    uint32_t fulladdr, fulladdrop;
    FILE  *fd;
    char parms_file[64];

    /* Sanity check for malloc */
    if (fwdata == NULL) {
        printf("%s: Failed to malloc for FW data buffer, fwdata.\n", __func__);
        return (FAILED);
    }

    /* Open and Load MCU firmware */
    sprintf(parms_file, DIAG_MCU_FILE);
    fd = fopen(parms_file, "rb");
    if (fd == NULL) {
        printf("\nFail to open MCU firmware file at firmware/mcuimg.\n");
        free(fwdata);
        return (FAILED);
    }

    fseek(fd, 0, SEEK_END);
    fwsize = ftell(fd);
    rewind(fd);

    fwdataop = fwdata;
    fread(fwdataop, fwsize, 1,fd);

    if (fwsize >= DIAG_MCU_AP_SIZE) {
        printf("\nMCU Firmware Size BIG than 0x3000 fwsize = 0x%8x.\n",fwsize);
        free(fwdata);
        fclose(fd);
        return (FAILED);
    } else {
        /* Already copy all MCU firmware into fwdata matrix */
		fclose(fd); 
    }

    /* Check MCU under Normal mode or Upgrade mode */

    /* Switch Star MCU from APP mode to bootloader mode */ 
    if (plat_mcu_reg_wr(DIAG_MCU_UPGRADE_OFFSET, DIAG_MCU_UPGRADE_SET) != PASSED) {
        printf("MCU did not working with APP mode.\n");
    } else {
        printf("MCU working with APP mode.\n");
        printf("It will take 5 MIN for MCU upgrade. Please do not power cycle system.\n");
        usleep(MCU_SWITCH_MODE_WAIT); 
    }

    /* Star MCU on I2C bus 2 */
    file_desc = open(I2C2_DEVICE_PATH, O_RDWR);
    if (file_desc < MCU_ZERO) {
        printf("\nMCU open I2C bus 2 fail!\n");
        free(fwdata);
        return (FAILED);        
    }
    /* Check MCU under bootloader mode or not by send Ident-byte */ 
    if (mcu_en_upgrade_bl(file_desc) == FAILED) {
        printf("\nFAIL!!! MCU did not under APP or Bootloader mode.\n");
        free(fwdata);
        close(file_desc); 
        return (FAILED);        
    }
    if (mcu_check_ack_bl(file_desc, data) == FAILED ) {
        printf("\nMCU under Bootloader mode but check ACK fail.\n");
        free(fwdata);
        close(file_desc); 
        return (FAILED);
    }
    
    /* Count Loop Times for R/W */
    rwloop = fwsize / DIAG_MCU_RW_MAX_SIZE;
    rwloop_rm = fwsize % DIAG_MCU_RW_MAX_SIZE;
    
    fulladdr = DIAG_MCU_APP_MODE_ADDR;
    fwdataop = fwdata;
    /*** WRITE DATA to MCU***/
    /* Write FULL 64 Bytes loops*/
    for (ix = 0; ix < rwloop; ix++) {
        /* Set Write Address */
        fulladdrop = fulladdr + (ix * DIAG_MCU_RW_MAX_SIZE);
        for (iy = 0;iy < MCU_ADDR_SIZE;iy++) {
            addr[iy] = SPEAD_ADD(fulladdrop, iy );
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s:%d fulladdr = 0x%8x, addr[%d] = 0x%2x.\n",
                         __FUNCTION__, __LINE__, fulladdr, iy, addr[iy]);
            }
        }

        /* prepare this loop data */ 
        for (iy = 0; iy < DIAG_MCU_RW_MAX_SIZE; iy++) {
            /* KEY byte write 0x00, the byte will be program later */
            if ((ix == 0) && (iy == 0) ) {
                data[iy] = DIAG_MCU_UPGRADE_MAGIC;
                fwdataop++;
                continue;
            }
            data[iy] = *fwdataop;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s:%d WRTIE FULL loop %d, data[%d] = 0x%2x.\n",
                         __FUNCTION__, __LINE__, ix, iy, *fwdataop);
            }
            fwdataop++;
        }
    
        /* Write command set */ 
        mcu_write_mem_set_bl_f1(file_desc, addr,DIAG_MCU_RW_MAX_SIZE, data);
        mcu_write_mem_set_bl_f2(file_desc, addr,DIAG_MCU_RW_MAX_SIZE, data);
        mcu_write_mem_set_bl_f3(file_desc, addr,DIAG_MCU_RW_MAX_SIZE, data);

        /* Write data to MCU  */
        ret = mcu_write_mem_cmd_bl(file_desc, addr,DIAG_MCU_RW_MAX_SIZE, data);
        if (ret == FAILED) {
            printf("\nMCU WRITE DATA CMD fail at FULL loop %d!\n", ix);
            free(fwdata);
            close(file_desc);
            return (FAILED);        
        }
    } /* end of write FULL 64 */

    /* No remind bytes, skip remind write */
    if (rwloop_rm == MCU_ZERO) {
        goto NO_REMIND_WRITE;
    }

    /* Write remind bytes */
    /* Set remind start address */
    fulladdrop = fulladdr + (rwloop * DIAG_MCU_RW_MAX_SIZE);
    for (ix = 0;ix < MCU_ADDR_SIZE;ix++) {
        addr[ix] = SPEAD_ADD(fulladdrop, ix );
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d fulladdrop = 0x%8x, addr[%d] = 0x%2x.\n",
                    __FUNCTION__, __LINE__, fulladdrop, ix, addr[ix]);
        }
    }
    for (ix = 0; ix < rwloop_rm; ix++) {
        data[ix] = *fwdataop;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d WRTIE FULL loop %d, data[%d] = 0x%2x.\n",
                     __FUNCTION__, __LINE__, ix, iy, *fwdataop);
        }
        fwdataop++;
    }
    
    /* Write command set */ 
    mcu_write_mem_set_bl_f1(file_desc, addr,rwloop_rm, data);
    mcu_write_mem_set_bl_f2(file_desc, addr,rwloop_rm, data);
    mcu_write_mem_set_bl_f3(file_desc, addr,rwloop_rm, data);

    /* Write data to MCU  */
    ret = mcu_write_mem_cmd_bl(file_desc, addr,rwloop_rm, data);
    if (ret == FAILED) {
        printf("\nMCU WRITE DATA CMD fail at remind loop!\n");
        free(fwdata);
        close(file_desc);
        return (FAILED);        
    }
NO_REMIND_WRITE:

    /*** READ DATA from MCU  ***/
    /* Reset FW data operation point to start */
    fwdataop = fwdata;
    fulladdr = DIAG_MCU_APP_MODE_ADDR;
    for (ix = 0; ix < rwloop; ix++) {
        /* Set Write Address */
        fulladdrop = fulladdr + (ix * DIAG_MCU_RW_MAX_SIZE);
        for (iy = 0;iy < MCU_ADDR_SIZE; iy++) {
            addr[iy] = SPEAD_ADD(fulladdrop, iy );
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s:%d fulladdrop = 0x%8x, addr[%d] = 0x%2x.\n",
                         __FUNCTION__, __LINE__, fulladdrop, iy, addr[iy]);
            }
        }
        /* Clear data matrix */
        for (iy = 0;iy < DIAG_MCU_RW_MAX_SIZE; iy++) {
            data[iy] = MCU_ZERO;
        }

        /* READ command set */ 
        mcu_read_mem_set_bl_f1(file_desc, addr, DIAG_MCU_RW_MAX_SIZE, data);
        mcu_read_mem_set_bl_f2(file_desc, addr, DIAG_MCU_RW_MAX_SIZE, data);
        mcu_read_mem_set_bl_f3(file_desc, addr, DIAG_MCU_RW_MAX_SIZE, data);
    
        /* READ data from MCU  */
        for (iy = 0; iy < DIAG_MCU_RW_MAX_SIZE; iy++) {
            ret = mcu_read_mem_cmd_bl(file_desc, addr, DIAG_MCU_COMMAND_SIZE_1, data+iy);
            if (ret == FAILED) {
                printf("\nMCU READ DATA CMD fail at FULL loop %d at byte %d!\n", ix, iy);
                free(fwdata);
                close(file_desc);
                return (FAILED);        
            }
        }

        /* compare this loop data */ 
        for (iy = 0; iy < DIAG_MCU_RW_MAX_SIZE; iy++) {
            /* SKIP KEY byte compare, the byte will be program later */
            if ((ix == 0) && (iy == 0) ) {
                fwdataop++;
                continue;
            }

            if ((*(data+iy)) != (*fwdataop)) {
                printf("MCU upgrade fail with FULL loop %d, fulladdr = 0x%8x,\n\
                        data_offset iy= %d, re_data=0x%2x, ex_data=0x%2x,\n\
                        re_data point= 0x%p, ex_data point = 0x%p,\n"\
                        , ix, fulladdr, iy, (*(data+iy)), (*fwdataop),(data+iy), fwdataop);
                free(fwdata);
                close(file_desc);
                return (FAILED);        
            }
            fwdataop++;
        }

    } /* Finish READ and Compare FULL loop */
    
    if (rwloop_rm == MCU_ZERO) {
        goto NO_REMIND_READ;
    }

    /* Read remind data */
    fulladdrop = fulladdr + (rwloop * DIAG_MCU_RW_MAX_SIZE);
    for (ix = 0;ix < MCU_ADDR_SIZE; ix++) {
        addr[ix] = SPEAD_ADD(fulladdrop, ix );
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d fulladdrop = 0x%8x, addr[%d] = 0x%2x.\n",\
                     __FUNCTION__, __LINE__, fulladdrop, ix, addr[ix]);
        }
    }
    /* Clear data matrix */
    for (ix = 0;ix < DIAG_MCU_RW_MAX_SIZE; ix++) {
        data[ix] = MCU_ZERO;
    }

    /* READ command set */ 
    mcu_read_mem_set_bl_f1(file_desc, addr, rwloop_rm, data);
    mcu_read_mem_set_bl_f2(file_desc, addr, rwloop_rm, data);
    mcu_read_mem_set_bl_f3(file_desc, addr, rwloop_rm, data);
    
    /* READ data from MCU  */
    for (ix = 0; ix < rwloop_rm; ix++) {
        ret = mcu_read_mem_cmd_bl(file_desc, addr, DIAG_MCU_COMMAND_SIZE_1, data+ix);
        if (ret == FAILED) {
            printf("\nMCU READ DATA CMD fail at remind loop %d!\n", ix);
            free(fwdata);
            close(file_desc);
            return (FAILED);        
        }
    }
    /* compare this loop data */ 
    for (ix = 0; ix < rwloop_rm; ix++) {
        if ((*(data+ix)) != (*fwdataop)) {
            printf("MCU upgrade fail with Remind loop, fulladdr =0x%8x, fulladdrop = 0x%8x,\n \
                    data_offset ix= %d, re_data=0x%2x, ex_data=0x%2x,\n \
                    re_data point= 0x%p, ex_data point = 0x%p.\n", \
                    fulladdr, fulladdrop, ix, (*(data+ix)), (*fwdataop),(data+ix),fwdataop);
            free(fwdata);
            close(file_desc);
            return (FAILED);        
        }
        fwdataop++;
    }

NO_REMIND_READ:

    /*** WRITE KEY Byte to 0x9000 ***/
    /* Reset FW data operation point to start */
    fwdataop = fwdata;
    fulladdr = DIAG_MCU_APP_MODE_ADDR;
    for (ix = 0;ix < MCU_ADDR_SIZE; ix++) {
        addr[ix] = SPEAD_ADD(fulladdr, ix );
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d fulladdr = 0x%8x, addr[%d] = 0x%2x.\n",
                    __FUNCTION__, __LINE__, fulladdr, ix, addr[ix]);
        }
    }
    /* Replace magic number to real one */
    data[MCU_ZERO] = *fwdataop;
    
    /* Write command set */ 
    mcu_write_mem_set_bl_f1(file_desc, addr,DIAG_MCU_COMMAND_SIZE_1, data);
    mcu_write_mem_set_bl_f2(file_desc, addr,DIAG_MCU_COMMAND_SIZE_1, data);
    mcu_write_mem_set_bl_f3(file_desc, addr,DIAG_MCU_COMMAND_SIZE_1, data);

    /* Write data to MCU  */
    ret = mcu_write_mem_cmd_bl(file_desc, addr,DIAG_MCU_COMMAND_SIZE_1, data);
    if (ret == FAILED) {
        printf("\nMCU WRITE DATA CMD fail at Write KEY magic number!\n");
        free(fwdata);
        close(file_desc);
        return (FAILED);        
    }

    /*** GO and reboot CPU ***/
    fulladdr = DIAG_MCU_APP_MODE_ADDR;
    for (iy = 0;iy < MCU_ADDR_SIZE;iy++) {
        addr[iy] = SPEAD_ADD(fulladdr, iy );
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d fulladdr = 0x%8x, addr[%d] = 0x%2x.\n",
                    __FUNCTION__, __LINE__, fulladdr, iy, addr[iy]);
        }
    }
    
    /* GO command SET */ 
    ret = mcu_go_addr_set_bl(file_desc, addr);
    
    /* GO command */ 
    ret = mcu_go_addr_cmd_bl(file_desc, addr);
    if (ret == FAILED) {
        printf("\nMCU READ DATA CMD fail!\n");
        free(fwdata);
        close(file_desc);
        return (FAILED);        
    }

    free(fwdata);
    close(file_desc);
	return (PASSED);
}

/* Cisco MCU firmware */
/*******************************************************************************
 *
 * Function    : pwr_seq_eeprom_update 
 * Description : to execute command update 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_seq_eeprom_update (void)
{
    int rc = PASSED;
    n2g_i2c_if_t *i2c_if;
    uint16_t data;
    
    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }
    if (byswap == FALSE) {
        /* set cmd to UPDATE mode */
        i2c_if->offset = PWR_SEQ_FW_CMD_REG;
        data = PWR_SEQ_CMD_UPDATE;
        i2c_if->buf = (char *)&data;
        if (rc != PASSED) {
            printf("Set cmd to update mode failed (offset = %X, data = %x)\n", \
                   i2c_if->offset, data);
            goto fun_ret;
        }
        byswap = TRUE;
    }
    
fun_ret:

    return rc;
}


/*******************************************************************************
 *
 * Function    : pwr_seq_eeprom_rd 
 * Description : to execute read process on eeprom data
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_seq_eeprom_rd (uint16_t to_addr, uint16_t *rd_data)
{
    int rc = FAILED;
    uint16_t     reg_val = 0; 
    uint16_t     buf1, buf2, buf3;
    n2g_i2c_if_t *i2c_if;
    
    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }

    /* set write address to corresponding addr of  the begining block */
    i2c_if->offset = PWR_SEQ_FW_CMD_REG;
    if (byswap == TRUE) {
        /* Since UPD 0XFFEE is enable, the address need to swap 
         * ex.: buf1 = 0xfb02 --> 0x0200
         *      buf2 = 0xfb02 --> 0xfb
         *      buf3 = 0x2fb   */ 
        buf1 = ((to_addr & 0xFF) << 8);
        buf2 = ((to_addr & 0xFF00) >> 8); 
        buf3 = buf1 + buf2;
        i2c_if->buf = (char *)&buf3;
    } else {
        i2c_if->buf = (char *)&to_addr; /* two bytes address */
    }
    *rd_data = 0;
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X)\n",\
               i2c_if->offset);
        goto fun_ret;
    }
    
    i2c_if->offset = PWR_SEQ_FW_DATA_REG;
    i2c_if->buf = (char *)&reg_val;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        printf("FW data register read failed (offset = %X)\n", \
               i2c_if->offset); 
        goto fun_ret;
    }
    if (byswap == TRUE) {
        *rd_data  = (uint16_t)reg_val;
    } else {
        *rd_data = (uint16_t)dswap2(reg_val); 
    }
    rc = PASSED;

fun_ret:

    return rc;
}

/*******************************************************************************
 *
 * Function    : pwr_seq_eeprom_rd_util 
 * Description : to execute read process on eeprom data
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int pwr_seq_eeprom_rd_util (void)
{
    uint16_t to_addr, val;
    int rc = FAILED;

    to_addr = gethex_answer("Enter address to read from", 0x8000, 0, 0xFC00);
    rc = pwr_seq_eeprom_rd(to_addr, &val);
    printf("\nPwr seq reg.(0x%02X) = 0x%04X\n", to_addr, val);
    return (rc);
}

/*******************************************************************************
 *
 * Function    : pwr_seq_eeprom_wr_util
 * Description : to execute write process on eeprom data
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int pwr_seq_eeprom_wr_util (void)
{
    uint16_t buf_data, buf1, buf2, buf3;
    uint16_t wr_data, to_addr; 
    int rc = FAILED;
    n2g_i2c_if_t *i2c_if;
    
    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        goto fun_ret;
    }
    /*to_addr = gethex_answer("Enter address to write", 0x8000, 0, 0xFC00); */
    printf("Offset address range from 0x8000 ~ 0xFC00.\n");
    to_addr = gethex_answer("Enter address to write", 0x0000, 0, 0xFFFF);
    buf_data = gethex_answer("Enter value to write", 0, 0, 0xFFFF);

    /* if byswap already set no need to set twice */
    if (byswap == FALSE) {
        /* put device into update mode */
        i2c_if->offset = PWR_SEQ_FW_CMD_REG;
        wr_data = PWR_SEQ_CMD_UPDATE;
        i2c_if->buf = (char *)&wr_data;
        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
            printf("Set cmd to update mode failed (offset = %X, data = %x)\n", \
                   i2c_if->offset, wr_data);
            goto fun_ret;
        }
        byswap = TRUE;
    }
    
    /*
     * 1. if byswap = TRUE
     * 2. swap address from fb02 -> 02fb
     * 3. swap value from 1234 -> 3412 
     * 4. if byswap = FALSE 
     * 5. no swap address fb02 
     * 3. no swap value 1234 
     * */ 
    /* set write address */
    i2c_if->offset = PWR_SEQ_FW_CMD_REG;   // 0xFE
    if (byswap == TRUE) {
        /* Since UPD 0XFFEE is enable, the address need to swap 
         * ex.: buf1 = 0xfb02 --> 0x0200
         *      buf2 = 0xfb02 --> 0xfb
         *      buf3 = 0x2fb   */ 
        buf1 = ((to_addr & 0xFF) << 8);
        buf2 = ((to_addr & 0xFF00) >> 8); 
        buf3 = buf1 + buf2;
        i2c_if->buf = (char *)&buf3; /* two bytes address */
    } else {
        i2c_if->buf = (char *)&to_addr; /* two bytes address */  
    } 
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if->offset, wr_data);
        goto fun_ret;
    }
    
    /* begin write data */
    i2c_if->offset = PWR_SEQ_FW_DATA_REG;
    wr_data = buf_data;
    if (byswap == TRUE) {
        /* Since UPD 0XFFEE is enable, the data need to swap 
         * ex.: buf1 = 0x1234 --> 0x3400
         *      buf2 = 0x1234 --> 0x12
         *      buf3 = 0x3412   */ 
        buf1 = ((wr_data & 0xFF) << 8);
        buf2 = ((wr_data & 0xFF00) >> 8); 
        buf3 = buf1 + buf2;
        i2c_if->buf = (char *)&buf3;
    } else {
        i2c_if->buf = (char *)&wr_data;
    }
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("FW data register write failed (offset = %X, data = %x)\n", \
               i2c_if->offset, wr_data);
        goto fun_ret;
    }
    rc = PASSED;

fun_ret:
    return rc;
}

/*******************************************************************************
 *
 * Function    : verify_pwr_seq_fw 
 * Description : to execute verify process on eeprom data
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int verify_pwr_seq_fw (void)
{
    uint16_t rd_data, wr_data, to_addr, end_addr;
    uint16_t buf1, buf2, buf3;
    unsigned char *hex;
    int id, region;
    int rc = FAILED;
    int mismatch = 0;
    n2g_i2c_if_t *i2c_if;

    if ((hex = (unsigned char *)malloc(FW_BIN_SZ)) == NULL) {
        printf("system is out of memory.\n");
        return -1;
    }
    if ((readfile(BIN_FW, hex, FW_BIN_SZ) < 0)) {
        printf("unable to read binary firmware file.\n");
    }

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }

    /* set write address to corresponding addr of  the begining block */
    for (region = 0; region < MAX_REGION; region++) {
        to_addr = start[region];
        end_addr = end[region];

        i2c_if->offset = PWR_SEQ_FW_CMD_REG;
        if (byswap == TRUE) {
            /* Since UPD 0XFFEE is enable, the address need to swap 
             * ex.: buf1 = 0xfb02 --> 0x0200
             *      buf2 = 0xfb02 --> 0xfb
             *      buf3 = 0x2fb   */ 
            buf1 = ((to_addr & 0xFF) << 8);
            buf2 = ((to_addr & 0xFF00) >> 8); 
            buf3 = buf1 + buf2;
            i2c_if->buf = (char *)&buf3; /* two bytes address */
        } else {
            i2c_if->buf = (char *)&to_addr; /* two bytes address */
        }
        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
            printf("Reseting CMD register failed (offset = %X, data = %x)\n", \
                   i2c_if->offset, wr_data);
            goto fun_ret;
        }

        printf("verifying [0x%04x-0x%04x]...\n",  to_addr, end_addr);
    
        /* firwmare has 64K byte */
        for (mismatch = 0, id = to_addr; id < end_addr; id += 2) {
            i2c_if->offset = PWR_SEQ_FW_DATA_REG;
            i2c_if->buf = (char *)&wr_data;
            rc = n2g_i2c_read(i2c_if);
            if (rc != PASSED) {
                printf("FW data register write failed (offset = %X, data = %x)\n", \
                       i2c_if->offset, wr_data);
                goto fun_ret;
            }
            if ((id%256)==0) {
                print_spining_wheel(-1);
            }

            if (byswap == TRUE) {
                rd_data  = (uint16_t)wr_data;
            } else {
                rd_data = (uint16_t)dswap2(wr_data); 
            }
            if  ((((rd_data >> 8) & 0xFF) != hex[id])
                 && (((rd_data & 0xFF) != hex[id+1]))) {
                rc = FAILED;
                printf("data mismatch @0x%06x ", id);
                printf("expect [0x%02x%02x] : found [0x%02x%02x]\n", hex[id],
                       hex[id+1], (rd_data >> 8) & 0xFF, (rd_data & 0xFF));
                
                if (mismatch++ > 10) {
                    printf("too many mistmatches...aborting comparison\n");
                    break;
                }
            }
        }
    }
 fun_ret:
    if (hex)
        free(hex);
    return rc;
}

/*******************************************************************************
 *
 * Function    : dump_pwr_seq_fw 
 * Description : to dump data firmware from eeprom 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dump_pwr_seq_fw (void)
{
    FILE *fp;
    uint16_t wr_data, to_addr, end_addr;
    uint16_t     buf1, buf2, buf3;
    int id;
    int rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }
    
    /* set write address to corresponding addr of  the begining block */
    to_addr = gethex_answer("Enter start address", 0x8000, 0, 0xFC00);
    end_addr = gethex_answer("Enter end address", 0xFC00, 0, 0xFC00);
    
    i2c_if->offset = PWR_SEQ_FW_CMD_REG;
    if (byswap == TRUE) {
        /* Since UPD 0XFFEE is enable, the address need to swap 
         * ex.: buf1 = 0xfb02 --> 0x0200
         *      buf2 = 0xfb02 --> 0xfb
         *      buf3 = 0x2fb   */ 
        buf1 = ((to_addr & 0xFF) << 8);
        buf2 = ((to_addr & 0xFF00) >> 8); 
        buf3 = buf1 + buf2;
        i2c_if->buf = (char *)&buf3; /* two bytes address */
    } else {
        i2c_if->buf = (char *)&to_addr; /* two bytes address */
    }
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if->offset, wr_data);
        goto fun_ret;
    }

    if ((fp = fopen("pseq_dump.txt", "w")) == NULL) {
        printf("can't open file to store fw");
        goto fun_ret;
    }

    /* firwmare has 64K byte */
    for (id = to_addr; id < end_addr; id += 2) /* two bytes per write */
    {
        i2c_if->offset = PWR_SEQ_FW_DATA_REG;
        i2c_if->buf = (char *)&wr_data;
        rc = n2g_i2c_read(i2c_if);
        if (rc != PASSED) {
    	    printf("FW data register write failed (offset = %X, data = %x)\n",\
                    i2c_if->offset, wr_data);
            goto fun_ret;
        }
        if ((id%8)==0 && (id>0)) { 
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x%02x", (wr_data >> 8) & 0xFF, (wr_data & 0xFF));
        fflush(fp);
    }
    rc = PASSED;
fun_ret:
    fclose(fp);
    return rc;
}

/*******************************************************************************
 *
 * Function   : write_pwr_seq_fw
 * Description: Write a section of power sequencer firmware through I2C
 * Inputs     : fw_buf: pointer to  a buffer to hold the binary data
 *              to_address: the start address (target addr) of the binary data
 *              byte_len: length of the binary data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int write_pwr_seq_fw(unsigned char *fw_buf, uint16_t to_address,
                  uint16_t end_address)
{
    uint16_t buf1, buf2, buf3;
    uint16_t wr_data;
    int id, idx;
    int rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }
    
    /* set write address to corresponding addr of  the begining block */
    i2c_if->offset = PWR_SEQ_FW_CMD_REG;
    if (byswap == TRUE) {
        /* Since UPD 0XFFEE is enable, the address need to swap 
         * ex.: buf1 = 0xfb02 --> 0x0200
         *      buf2 = 0xfb02 --> 0xfb
         *      buf3 = 0x2fb   */ 
        buf1 = ((to_address & 0xFF) << 8);
        buf2 = ((to_address & 0xFF00) >> 8); 
        buf3 = buf1 + buf2;
        i2c_if->buf = (char *)&buf3;
    } else {
        i2c_if->buf = (char *)&to_address; /* two bytes address */
    } 
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if->offset, wr_data);
        goto fun_ret;
    }
    fflush(NULL);

    for (idx = 0, id = to_address; id < end_address; id += 2,
             idx+=2 /*2 bytes per wr */) 
    {
        wr_data = ((fw_buf[idx] & 0xFF) << 8) +
            (fw_buf[idx+1] & 0xFF);
        if ((idx%ONE_K)==0) {
            print_spining_wheel(-1);
        }
        
        /* begins write data */
        i2c_if->offset = PWR_SEQ_FW_DATA_REG;
        if (byswap == TRUE) {
            /* Since UPD 0XFFEE is enable, the data need to swap 
             * ex.: buf1 = 0x1234 --> 0x3400
             *      buf2 = 0x1234 --> 0x12
             *      buf3 = 0x3412   */ 
            buf1 = ((wr_data & 0xFF) << 8);
            buf2 = ((wr_data & 0xFF00) >> 8); 
            buf3 = buf1 + buf2;
            i2c_if->buf = (char *)&buf3;
        } else {
            i2c_if->buf = (char *)&wr_data;
        }

        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
    	    printf("FW data register write failed (offset = %X, data = %x)\n",\
                    i2c_if->offset, wr_data);
            goto fun_ret;
        }
    }
    rc = PASSED;

fun_ret:
    return rc;
}

/*******************************************************************************
 *
 * Function   : cnt_record 
 * Description: Counting FW line as a record 
 * Inputs     : fw_path: pointer to  a buffer to hold the binary data
 *              line: pointer of the line 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint16_t cnt_record(char *fw_path, uint16_t *line)
{
    FILE *fp;
    uint16_t ret = PASSED;
    char buf[SREC_DATA_SZ* 2 + 32]; /*data + header, etc... */

    if ((fp = fopen(fw_path, "rb")) == NULL) {
        printf("unable to open srec file %s.\n", fw_path);
        perror("");
        return (FAILED);
    }
    *line = 0;
    while ( fgets(buf, sizeof(buf), fp) ) {
        *line = *line+1;
        if (*line == 0xFFFF) {
            printf("too many records in srec file\n");
            ret = FAILED;
            break;
        }
    }
    fclose(fp);
    if (*line <= 1) {
        printf("Too few records in srec file\n");
        ret = FAILED;
    }
    printf("%d records in srec file\n", *line);
    return (ret);
}

/******************************************************************************
 *
 * Function   : write_pwr_seq_fw_util
 * Description:	Function to update power sequencer's firmware
 * srec2bin will be called which takes srec file and produces bin file called
 * pseq.bin
 * to debug: we can convert pseq.bin to hex file using command:
 * 'xxd -ps -c 8 pseq.bin > pseq_fw.txt'
 * then use menu item to read out content of eeprom which will be stored in a
 * file called 'pseq_dump.txt'.
 * then compare the 2 files pseq.txt and pseq_dump.txt
 * 
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
uint32_t write_pwr_seq_fw_util (int dummy)
{
    n2g_i2c_if_t *i2c_if;
    time_t start_t, stop_t;
    uint32_t rc = FAILED;
    uint16_t data;
    uint16_t buf1, buf2, buf3;
    int byte_len, idx, ret_val;
    char fw_path[256] = {0};
    char *tk = NULL;
    char *argv[10];
    char cmd[80];
    unsigned char *fw_buf = NULL;
    FILE *fp_bin = NULL;
    uint16_t to_address, blk_size, size_opt;
    int *addr_list, *byte_cnt_list;
    uint16_t line = 0;

    byte_cnt_list = addr_list = NULL;
    
    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        rc = FAILED;
        goto fun_ret;
    }
    
    /* query fw path and filename */
    printf("Please enter srec file [%s] (Enter q to quit): ", \
           PWR_SEQ_FW_DEFAULT_PATH);
    fflush(stdout);
    get_line(fw_path, sizeof(fw_path));
    if (strcmp(fw_path, "q") == 0)
    {   /* quit */
        return PASSED;
    }
    if (strlen(fw_path) <= 0)
    {
        /* use default path */
        memcpy(fw_path, PWR_SEQ_FW_DEFAULT_PATH, \
                        strlen(PWR_SEQ_FW_DEFAULT_PATH));
    }
    if (cnt_record(fw_path, &line) == FAILED) {
        return (FAILED);
    }
    addr_list = (int *)malloc(sizeof(int *) * (line+1));
    byte_cnt_list = (int *)malloc(sizeof(int *) * (line+1));

    /* usage: srec2bin output_file -k file_size -v 0 -d 0 -s pseq.s19
     * -v : verbose
     * -f : padd with 0
     * -s : input file..has to be the last option
    */ 
    size_opt = FW_BIN_SZ / ONE_K;
    sprintf(cmd, "./srec2bin %s -K %d -v 0 -d 0 -s %s", BIN_FW, size_opt, fw_path);
    tk = strtok(cmd, " ");
    idx = 0;
    argv[idx++] = tk;
    while (tk != NULL) {
        tk = strtok(NULL, " ");
        argv[idx++] = tk;
        if (tk)
            printf("%s ", tk);
    }
    idx--;
    printf("\nsrec file is used to create %s [%d bytes].\n", BIN_FW,
           size_opt * ONE_K);
    srec2bin_main(idx, argv, addr_list, byte_cnt_list);
    if ((fp_bin = fopen(BIN_FW, "rb")) == NULL) {
        printf("unable to open binary file for programming.\n");
        return (FAILED);
    }

    /* check size of binary pseq_fw.bin (should be 64K) */
    fseek(fp_bin, 0, SEEK_END);
    byte_len = ftell(fp_bin);
    if (byte_len != FW_BIN_SZ) {
        printf("%s has incorrect size of %d bytes. expect %d bytes.\n", BIN_FW,
               byte_len, FW_BIN_SZ);
        goto fun_ret;
    }
    
    fw_buf = (unsigned char *)malloc(byte_len);
    memset(fw_buf, 0, sizeof(byte_len));

    if (byswap == FALSE) {
        /* set cmd to UPDATE mode */
        i2c_if->offset = PWR_SEQ_FW_CMD_REG;
        data = PWR_SEQ_CMD_UPDATE;
        i2c_if->buf = (char *)&data;
        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
            printf("Set cmd to update mode failed (offset = %X, data = %x)\n",\
                i2c_if->offset, data);
            rc = FAILED;
            goto fun_ret;
        }
        byswap = TRUE;
    }

    printf("Using %s file to update.\n", BIN_FW);
    printf("Regions to be updated: [0x%4x-%4x] and [0x%4x-%4x]\n\n",
           start[0], end[0]-1, start[1], end[1]-1);
    time(&start_t);
    pwr_seq_eeprom_update();
    
    for (idx=0;idx<line;idx++) {
        if ( ((addr_list[idx] >= start[0]) && (addr_list[idx] < end[0])) ||
             ((addr_list[idx] >= start[1]) && (addr_list[idx] < end[1])) ) {

            to_address = addr_list[idx];
            blk_size = byte_cnt_list[idx];

            /* seek and read data from file and store to fw_buf */
            fseek(fp_bin, to_address, SEEK_SET);
            byte_len = fread(fw_buf, 1, blk_size,  fp_bin);
            if (byte_len != blk_size) {
                printf("at record %d @0x%4x, found %d bytes; expect %d bytes\n",
                       idx, blk_size, byte_len, to_address);
                goto fun_ret;
            }
            /* handle special case when a record has odd number of data */
            if (blk_size%2) {
                /*read 2 bytes from eeprom; store only upper nibble to the
                 end of buffer to make the size of data even. */
                pwr_seq_eeprom_rd(to_address+blk_size, &data);
                fw_buf[blk_size] = (data >> 8) & 0xFF;
                /*round up to even number */
                blk_size++;
            }
            write_pwr_seq_fw(fw_buf, to_address, to_address+blk_size);
        } 
    }

    time(&stop_t);
    printf("Update done. Took %.0f secs.\n", difftime(stop_t, start_t));
    ret_val = verify_pwr_seq_fw();
    time(&stop_t);
    printf("total time: %.0f secs.\n", difftime(stop_t, start_t));
    
    if (ret_val < 0) {
        printf("\n\n****WARNING****\n\n");
        printf("\nFirmware verification failed. Please re-program firmware.\n");
        printf("Do not reboot system until firmware upgrade is successful.\n");
    } else
        printf("To load the updated firmware, please reboot system.\n");

    
    if (getc_answer("Reboot system? (y/n)", "yn", 'n') == 'y') {
        /* reboot */
        i2c_if->offset = PWR_SEQ_FW_CMD_REG;
        data = PWR_SEQ_CMD_REBOOT;
        if (byswap == TRUE) {
            /* Since UPD 0XFFEE is enable, the address need to swap 
             * ex.: buf1 = 0xfb02 --> 0x0200
             *      buf2 = 0xfb02 --> 0xfb
             *      buf3 = 0x2fb   */ 
            buf1 = ((data & 0xFF) << 8);
            buf2 = ((data & 0xFF00) >> 8); 
            buf3 = buf1 + buf2;
            i2c_if->buf = (char *)&buf3; /* two bytes address */ 
        } else {
            i2c_if->buf = (char *)&data; /* two bytes address */  
        } 
        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
	        printf("reboot power sequencer failed (offset = %X, data = %x)\n",\
                i2c_if->offset, data);
            goto fun_ret;
        }
        sleep(5); /* power sequencer is rebooting */
        exit(0);
    }
    rc = PASSED;
    
fun_ret:
    if (fp_bin!=NULL)
        fclose(fp_bin);
    if (fw_buf!=NULL)
        free(fw_buf);
    if (addr_list)
        free(addr_list);
    if (byte_cnt_list)
        free(byte_cnt_list);

    return rc;
}

/*******************************************************************************
 *
 * Function   : get_pwr_seq_fw_rev
 * Description:	Returns Power sequencer FW revision.
 * Inputs     : option - to determine use cterr or not
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int get_pwr_seq_fw_rev (int option)
{
    uint32_t rc = FAILED;
    uint16_t rev = 0;

    rc = plat_mcu_reg_rd(PWR_SEQ_REV, &rev); 
    if (rc != PASSED) {
        if (option) {
	    cterr('f', 0, "%s:%d Failed to read Power Sequencer FW revision.",
              __FUNCTION__, __LINE__);
        }
        return (rc);
    }
    
    printf("Power Sequencer FW version is %d.%02d.\n",
           (rev & PS_REV_MAJOR_MSK) >> PS_REV_MAJOR_SHIFT,
           (rev & PS_REV_MINOR_MSK));

    return (rc);
}

/*******************************************************************************
 *
 * Function   : mcu_fw_upg
 * Description:	Upgrade MCU FW based on SKU.
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t mcu_fw_upg (void)
{
    uint rc = PASSED;
    rc = mcu_fw_upgrade();
    return (rc);
}

/*******************************************************************************
 *
 * Function   : mcu_fw_show
 * Description:	Show MCU FW based on SKU.
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t mcu_fw_show (void)
{
    uint rc = PASSED;
    rc = mcu_fw_verno();
    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_mcu_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

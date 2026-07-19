/* $Id: dnv_gpio_lib.c,v 1.6 2020/03/04 00:02:51 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/dnv_gpio_lib.c,v $
 *------------------------------------------------------------------
 * 
 * dnv_gpio_lib.c
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "dnv_gpio_lib.h"


/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */
int     dnv_p2sb_access_read (uint , uint *);
int     dnv_p2sb_access_write (uint , uint );
int     dnv_gpio_read_util (void);
int     dnv_gpio_write_util (void);
int     dnv_gpio_write(uint , uint );
int     dnv_gpio_read(uint , uint *);
int     dnv_gpio_read_rx_val(uint, uint *);
int     dnv_set_pcie_register(void);
int     dnv_get_gpio_addr(uint , uint *);
int     dnv_p2sb_read(void);
int     dnv_p2sb_write(void);
int     set_dnv_gpio_direction(int, int);

/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */


/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */

/*******************************************************************************
 *
 * Function    : dnv_gpio_read_util
 * Description : Function to read GPIO value
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
int dnv_gpio_read_util (void)
{
    uint gpio_pin = 0, value;

    gpio_pin = gethex_answer("Enter GPIO number: ", 0, 0, 0x9);

    dnv_gpio_read(gpio_pin, &value);

    printf("value: %x\n", value);

    return (PASSED);

}

/************************************************************
 *
 * Function    : dnv_gpio_write_util
 * Description : Function to write GPIO value
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */

int dnv_gpio_write_util (void)
{
    uint gpio_pin = 0, value;

    gpio_pin = gethex_answer("Enter which GPIO (0x0 ~ 0x9): ",
                              0, 0, 0xff);

    dnv_gpio_read(gpio_pin, &value);

    printf("DNV GPIO_%x, value: %x\n", gpio_pin, value);

    value = gethex_answer("Enter data: ", 0, 0, 0xffffffff);

    dnv_gpio_write(gpio_pin, value);

    dnv_gpio_read(gpio_pin, &value);

    return (PASSED);

}




/*******************************************************************************
 *
 * Function    : dnv_get_gpio_addr
 * Description : Function to get GPIO Address
 * Inputs      : which_gpio - which GPIO
 *               *val   - data buffer (0 for Low, 1 for High)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_get_gpio_addr (uint which_gpio, uint *gpio_addr)
{
    uint target = 0xFF; 

    switch (which_gpio) {
        case DNV_GPIO_0:
            target = PAD_CFG_DW0_GPIO_0;
            break;
        case DNV_GPIO_1:
            target = PAD_CFG_DW0_GPIO_1;
            break;
        case DNV_GPIO_2:
            target = PAD_CFG_DW0_GPIO_2;
            break;
        case DNV_GPIO_3:
            target = PAD_CFG_DW0_GPIO_3;
            break;
        case DNV_GPIO_4:
            target = PAD_CFG_DW0_GPIO_4;
            break;
        case DNV_GPIO_5:
            target = PAD_CFG_DW0_GPIO_5;
            break;
        case DNV_GPIO_6:
            target = PAD_CFG_DW0_GPIO_6;
            break;
        case DNV_GPIO_7:
            target = PAD_CFG_DW0_GPIO_7;
            break;
        case DNV_GPIO_8:
            target = PAD_CFG_DW0_GPIO_8;
            break;
        case DNV_GPIO_9:
            target = PAD_CFG_DW0_GPIO_9;
            break;
        case DNV_GBE0_SDP0:
            target = PAD_CFG_DW0_GBE0_SDP0;
            break;
        default:
            printf("Please enter the correct GPIO number\n");
            return (FAILED);
    } 

    *gpio_addr = target;


    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : dnv_gpio_read_rx_val
 * Description : Function to read the RX value of GPIO 
 * Inputs      : which_gpio - which GPIO
 *               *val   - data buffer (0 for Low, 1 for High)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_gpio_read_rx_val (uint which_gpio, uint *val)
{
    uint data;

    if (dnv_gpio_read(which_gpio, &data) == FAILED) {
        printf("%s: Read GPIO (%d) Fails\n", __func__, which_gpio);
        return (FAILED);
    }

    if (data & DENVERTON_GPIO_RX_VAL_MASK) {
        *val = GPIO_HIGH;
    } else {
        *val = GPIO_LOW;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : dnv_p2sb_access_read
 * Description : Function to read Denverton p2sb memory space
 * Inputs      : which_reg - which register
 *               *rd_buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_p2sb_access_read (uint reg_addr, uint *rd_data)
{
    off_t    target = 0;
    int      fd = -1;
    void     *map_base, *virt_addr;
    unsigned map_size;

    map_size = MAP_SIZE;

    target = (off_t)reg_addr;


#ifdef GPIO_DEBUG
    printf("GPIO P2SB Address: %x\n", (unsigned int)target);
#endif


    fd = open(DNV_PCIE_1F_1, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

#ifdef GPIO_DEBUG
    printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
           PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
    fflush(stdout);
#endif

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    virt_addr = map_base + (target & MAP_MASK);
    *rd_data = *(volatile uint32_t*)virt_addr;
    
#ifdef GPIO_DEBUG
    printf("Data: %x\n", *rd_data);
#endif

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : dnv_p2sb_access_write
 * Description : Function to write Denverton p2sb memory space
 * Inputs      : reg_addr - which register
 *               wr_data   - buffer to write register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_p2sb_access_write (uint reg_addr, uint wr_data)
{

    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size;

    map_size = MAP_SIZE;

    target = (off_t)reg_addr;


    fd = open(DNV_PCIE_1F_1, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

#ifdef GPIO_DEBUG
    printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
           PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
    fflush(stdout);
#endif

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
         close(fd);
         return (FAILED);
    }
    virt_addr = map_base + (target & MAP_MASK);

#ifdef GPIO_DEBUG
    printf("Data: %x\n", *(volatile uint32_t*)virt_addr);
#endif

    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : dnv_gpio_read
 * Description : Function to read Denverton GPIO by byte.
 * Inputs      : which_gpio - which GPIO
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_gpio_read (uint which_gpio, uint *buf)
{
    uint gpio_addr;
    uint rd_buf;


    dnv_get_gpio_addr(which_gpio, &gpio_addr);


    if (dnv_p2sb_access_read(gpio_addr, &rd_buf) == FAILED) {
        cterr('f', 0, "%s: Cannot access P2SB Register");
        return (FAILED);
    }

    *buf = rd_buf;

    return (PASSED);

}


/*******************************************************************************
 *
 * Function    : dnv_gpio_write
 * Description : Function performs write Denverton GPIO by byte.
 * Inputs      : Which GPIO
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_gpio_write (uint which_gpio, uint wr_data)
{
    uint gpio_addr;

    dnv_get_gpio_addr(which_gpio, &gpio_addr);


    if (dnv_p2sb_access_write(gpio_addr, wr_data) == FAILED) {
        cterr('f', 0, "%s: Cannot access P2SB Register");
        return (FAILED);
    }

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : dnv_set_pcie_register
 * Description : Function to set up PCIE 1f.1 register 0x1E for access P2SB register
 * Inputs      : None
 *               
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_set_pcie_register (void)
{
    int      fd = -1;
    unsigned int page_size, tmp_bus_dev_fun, offset, map_size;
    off_t    target = 0;
    unsigned int pcie_bus, pcie_device, pcie_function;
    void     *map_base, *virt_addr;
    unsigned int buf;
    char *pci_rescan = DNV_PCIE_RESCANE;
    char cmd[128];

    pcie_bus = 0;
    pcie_device = 0x1F;
    pcie_function = 1;

    tmp_bus_dev_fun = (pcie_bus << DNV_BUS_FUNC_SHIFT_1M) | 
                      (pcie_device << DNV_DEV_NUM_SHIFT_32K) | 
                      (pcie_function << DNV_FUNC_NUM_SHIFT_4K);
    offset = CONFIG_NUTELLA_PCIE_ADDR + tmp_bus_dev_fun;

#ifdef GPIO_DEBUG
    printf("Offset: %x\n", offset);
#endif

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~(off_t)(page_size - 1));
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
        close(fd);
        return (FAILED);
    }

    virt_addr = map_base + (target & MAP_MASK) + DNV_P2SB_CONTOL_HIDE_DEV; 
    buf = *(volatile uint32_t*)virt_addr;

#ifdef GPIO_DEBUG
    printf("Data: %x\n", buf);
#endif

    *(volatile uint32_t*)virt_addr = 0;

    buf = *(volatile uint32_t*)virt_addr;

#ifdef GPIO_DEBUG
    printf("Data: %x\n", buf);
#endif

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);

    sprintf(cmd, "echo 1 > %s", pci_rescan);
    system(cmd);


    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : dnv_p2sb_read_util
 * Description : Function to read DNV P2SB
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_p2sb_read_util (void)
{
    uint reg_addr;
    uint rd_buf;


    printf("Read P2SB register\n");
    reg_addr = gethex_answer("Enter reg:", 0, 0, 0xFFFFFFFF);


    if (dnv_p2sb_access_read(reg_addr, &rd_buf) == FAILED) {
        cterr('f', 0, "%s: Cannot access P2SB Register");
        return (FAILED);
    }

    printf("P2SB reg: %x, Data: %x\n", reg_addr, rd_buf);

    return (PASSED);


}

/*******************************************************************************
 *
 * Function    : dnv_p2sb_write_util
 * Description : Function to write DNV P2SB
 * Inputs      : None
 *               
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int dnv_p2sb_write_util (void)
{
    uint reg_addr;
    uint rd_buf, wr_data;;

    printf("Write P2SB register\n");
    reg_addr = gethex_answer("Enter reg:", 0, 0, 0xFFFFFFFF);

    if (dnv_p2sb_access_read(reg_addr, &rd_buf) == FAILED) {
        cterr('f', 0, "%s: Cannot Read P2SB Register");
        return (FAILED);
    }

    printf("P2SB reg: %x, Data: %x\n", reg_addr, rd_buf);


    wr_data = gethex_answer("Enter write value:", 0, 0, 0xFFFFFFFF);
    if (dnv_p2sb_access_write(reg_addr, wr_data) == FAILED) {
        cterr('f', 0, "%s: Cannot Write P2SB Register");
        return (FAILED);
    }

    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : set_dnv_gpio_direction
 * Description : Function to set GPIO direction
 * Inputs      : gpio pin - number of gpio
 *               opt - gpio direction
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_dnv_gpio_direction (int gpio_pin, int opt)
{
    uint data, rc = FAILED;
    
    if (dnv_gpio_read(gpio_pin, &data) == FAILED) {
        cterr('f', 0, "%s: Read GPIO (%d) Fails\n", __FUNCTION__, gpio_pin);
        return (FAILED);
    }

    if (opt == GPIO_IN) {
        data &=(uint)~PADCFG0_GPIOTXRXDIS_MASK;
        data |= (uint)PADCFG0_GPIOTXDIS_VAL;
        rc = dnv_gpio_write(gpio_pin, data);
    } else {
        data &=(uint)~PADCFG0_GPIOTXRXDIS_MASK;
        data |= (uint)PADCFG0_GPIORXDIS_VAL;
        rc = dnv_gpio_write(gpio_pin, data);
    }
	
    if (rc == FAILED) {
        cterr('f', 0, "Cannot Write P2SB Register\n"); 
        return (FAILED);
    }
    
    return (PASSED);
}

/*-------------------------------------------------
$Log: dnv_gpio_lib.c,v $
Revision 1.6  2020/03/04 00:02:51  alicehua
CSCvt24819: Enable IRQ test items for XE build.

Revision 1.5  2020/02/04 08:49:43  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/

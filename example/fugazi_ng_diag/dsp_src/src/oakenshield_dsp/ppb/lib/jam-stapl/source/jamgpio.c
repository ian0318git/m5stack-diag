/**
 * @file	jamgpio.c
 * @brief	Raspberry Pi GPIO functions for JTAG programming
 */
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "jamgpio.h"

#include "types.h"
#include "libgeneric.h"
#include "diag_fpga.h"

uint16_t fpga_addr;
int size;
uint32_t data;

void show_cpld_ver(int);
extern void bsp_debug_printf(const char *fmt, ...);


void gpio_init_jtag(int target)
{
    show_cpld_ver(target);

    switch (target)
    {
        case CPLD_MB:
            bsp_debug_printf("\r %s(): Enable MB JTAG\n", __FUNCTION__);
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            bsp_debug_printf("\r %s(): Enable DB2 JTAG\n", __FUNCTION__);
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            bsp_debug_printf("\r %s(): Enable DB3 JTAG\n", __FUNCTION__);
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    data = JTAG_ENABLE; 
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_close_jtag(int target)
{
    switch (target)
    {
        case CPLD_MB:
            bsp_debug_printf("\r %s(): Disable MB JTAG\n", __FUNCTION__);
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            bsp_debug_printf("\r %s(): Disable DB2 JTAG\n", __FUNCTION__);
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            bsp_debug_printf("\r %s(): Disable DB3 JTAG\n", __FUNCTION__);
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    data = JTAG_DISABLE; 
    fpga_spi_direct_write(fpga_addr, size, data);

    show_cpld_ver(target);
}

void gpio_set_tdi(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data |=  JTAG_TDI_MASK;
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_clear_tdi(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data &=  (~JTAG_TDI_MASK);
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_set_tms(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data |=  JTAG_TMS_MASK;
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_clear_tms(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data &=  (~JTAG_TMS_MASK);
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_set_tck(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data |=  JTAG_TCK_MASK;
    fpga_spi_direct_write(fpga_addr, size, data);
}

void gpio_clear_tck(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);
    data &=  (~JTAG_TCK_MASK);
    fpga_spi_direct_write(fpga_addr, size, data);
}

unsigned int gpio_get_tdo(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_JTAG_CONTROL;
            break;
        case CPLD_DB2:
            fpga_addr = DB2_JTAG_CONTROL;
            break;
        case CPLD_DB3:
            fpga_addr = DB3_JTAG_CONTROL;
            break;
        default:
            return -1;
    }
    size = JTAG_CON_SIZE;
    fpga_spi_direct_read(fpga_addr, size, &data);

    data &=  JTAG_TDO_MASK;

    return ((data) ? 1 : 0);
}


//local functions
void show_cpld_ver(int target)
{
    switch (target)
    {
        case CPLD_MB:
            fpga_addr = MB_PLD_REV;
            size = PLD_REV_SIZE;
            fpga_spi_direct_read(fpga_addr, size, &data);
            bsp_debug_printf("\r %s(): MB PLD Revision, FPGA register :%x, Data: 0x%x\n", __FUNCTION__, fpga_addr, data);
            break;
        case CPLD_DB2:
            fpga_addr = DB2_PLD_REV;
            size = PLD_REV_SIZE;
            fpga_spi_direct_read(fpga_addr, size, &data);
            bsp_debug_printf("\r %s(): DB2 PLD Revision, FPGA register :%x, Data: 0x%x\n", __FUNCTION__, fpga_addr, data);
            break;
        case CPLD_DB3:
            fpga_addr = DB3_PLD_REV;
            size = PLD_REV_SIZE;
            fpga_spi_direct_read(fpga_addr, size, &data);
            bsp_debug_printf("\r %s(): DB3 PLD Revision, FPGA register :%x, Data: 0x%x\n", __FUNCTION__, fpga_addr, data);
            break;
        default:
            return;
    }
}


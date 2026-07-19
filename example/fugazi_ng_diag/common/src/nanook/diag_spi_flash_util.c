/* $Id: diag_spi_flash_util.c,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 *------------------------------------------------------------------
 * 
 * diag_spi_flash_util.c
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
#include <errno.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "diag_storage_lib.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "dnv_gpio_lib.h"
#include "diag_boot_flash_test.h"
#include "diag_spi_flash_util.h"
#include "dash_fpga.h"
//#include "diag_fpga_upgrade.h"
#include "platform_prom.h"

//Temp define
#define FPGA_SPI_CONTROL_REG    0x31800

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */

/* Local functions */
int denverton_spi_ctrl_mm_read32(uint, uint*);
int denverton_spi_ctrl_mm_write32(uint, uint);
int denverton_spi_hw_cycle(uchar, int);
int denverton_spi_read_block(uchar*, int);
int denverton_spi_write_block(uchar*, int);
int denverton_spi_read_reg(uchar, uchar*, int);
int denverton_spi_write_status_reg(int);
int denverton_spi_read_status_reg(int);
int denverton_spi_read_id(uint*);
int nanook_spi_flash_utils(int opt);
int nanook_spi_flash_get_prod_id(int, uint*);
int nanook_spi_flash_get_prod_name(int, char*);
int nanook_spi_flash_get_rdsr(int, uchar*);
int nanook_spi_flash_set_wrsr(int, uchar); 
int nanook_spi_flash_show_info(int); 
int nanook_spi_flash_write_protect_enable(int);
int diag_spi_flash_write_protect_test(int);
int spi_flash_write_protect_test(int);
int mtd_spi_flash_write_protect_test(char*);
int fpga_spi_flash_write_protect_test(void);
int fpga_spi_read_id(uint *);

static submenu_xtable_t spi_flash_utils_tbl[] = {
    {"Show Rommon Golden SPI Flash Info", (type_t(*)())nanook_spi_flash_show_info, GOLDEN_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show Rommon Upgrade SPI Flash Info", (type_t(*)())nanook_spi_flash_show_info, UPGRADE_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show FPGA SPI Flash Info", (type_t(*)())nanook_spi_flash_show_info, FPGA_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
#if 0
    {"Enable Rommon Golden SPI Flash Write Protect", (type_t(*)())nanook_spi_flash_write_protect_enable, GOLDEN_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Enable Rommon Upgrade SPI Flash Write Protect", (type_t(*)())nanook_spi_flash_write_protect_enable, UPGRADE_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Enable FPGA SPI Flash Write Protect", (type_t(*)())nanook_spi_flash_write_protect_enable, FPGA_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Rommon Golden SPI Flash Write Protect Test", (type_t(*)())diag_spi_flash_write_protect_test, GOLDEN_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Rommon Upgrade SPI Flash Write Protect Test", (type_t(*)())diag_spi_flash_write_protect_test, UPGRADE_ROMMON_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FPGA SPI Flash Write Protect Test", (type_t(*)())diag_spi_flash_write_protect_test, FPGA_SPI_FLASH, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
#endif
#ifdef SPI_FLASH_DEBUG
     /* for Debug*/
     {"RDSR - Golden", (type_t(*)())denverton_spi_read_status_reg, GOLDEN_ROMMON_SPI_FLASH, 0,
     (type_t(*)())this_is_nanook_j, 0, (type_t(*)())0,   0},
     {"WRSR - Golden", (type_t(*)())denverton_spi_write_status_reg, GOLDEN_ROMMON_SPI_FLASH, 0,
     (type_t(*)())this_is_nanook_j, 0, (type_t(*)())0,   0},
     {"RDSR - Upgrade", (type_t(*)())denverton_spi_read_status_reg, UPGRADE_ROMMON_SPI_FLASH, 0,
     (type_t(*)())this_is_nanook_j, 0, (type_t(*)())0,   0},
     {"WRSR - Upgrade", (type_t(*)())denverton_spi_write_status_reg, UPGRADE_ROMMON_SPI_FLASH, 0,
     (type_t(*)())this_is_nanook_j, 0, (type_t(*)())0,   0},
#endif
};


#define SPI_FLASH_UTILS_TBL_SIZE (sizeof(spi_flash_utils_tbl) / sizeof(submenu_xtable_t))

/* SPI Flash Utils items (filled in from xtable) */
static mitem_t spi_flash_utils_pri_items[SPI_FLASH_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t spi_flash_utils_sec_items[SPI_FLASH_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* SPI Flash Utils submenu */
menuinfo_t spi_flash_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    spi_flash_utils_pri_items,
};
menuinfo_t *spi_flash_utils_menup = &spi_flash_utils_menu;


/*******************************************************************************
 *
 * Function    : denverton_spi_ctrl_mm_read32
 * Description : Function to read Denverton SPI Controller memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_ctrl_mm_read32 (uint offset, uint *buf)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)(NANOOK_DENVERTON_SPI_CTRL_MM_ADDR + offset);
 
    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *buf = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_ctrl_mm_write32
 * Description : Function performs write Denverton SPI Controller memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_ctrl_mm_write32 (uint offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)(NANOOK_DENVERTON_SPI_CTRL_MM_ADDR + offset);
 
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_hw_cycle
 * Description : Function issue Denverton SPI Controller operation.
 * Inputs      : opcode  - opcode
 *                  len - length for read/write data
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_hw_cycle(uchar opcode, int len)
{
    uint val, status;
    int ret;

    ret = denverton_spi_ctrl_mm_read32(HSFSTS_CTL, &val);
    if (ret != PASSED) {
        printf("%s: Failed to read Denverton SPI Controller memory.\n", __FUNCTION__);
        return (FAILED);
    }
	
    val &= ~(HSFSTS_CTL_FCYCLE_MASK | HSFSTS_CTL_FDBC_MASK | HSFSTS_CTL_WRSDIS);

    switch (opcode) {
    case SPINOR_OP_RDID:
        val |= HSFSTS_CTL_FCYCLE_RDID;
        break;
    case SPINOR_OP_WRSR:
        val |= HSFSTS_CTL_FCYCLE_WRSR;
        break;
    case SPINOR_OP_RDSR:
        val |= HSFSTS_CTL_FCYCLE_RDSR;
        break;
    default:
        return (FAILED);
    }

    if (len > INTEL_SPI_FIFO_SZ) {
        return (FAILED);
    }

    val |= (len - 1) << HSFSTS_CTL_FDBC_SHIFT;
    val |= HSFSTS_CTL_FCERR | HSFSTS_CTL_FDONE;
    val |= HSFSTS_CTL_FGO;

    ret = denverton_spi_ctrl_mm_write32(HSFSTS_CTL, val);
    if (ret != PASSED) {
        printf("%s: Failed to write Denverton SPI Controller memory.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(SPI_WAIT_TIME);    

    ret = denverton_spi_ctrl_mm_read32(HSFSTS_CTL, &status);
    if (ret != PASSED) {
        printf("%s: Failed to read Denverton SPI Controller memory.\n", __FUNCTION__);
        return (FAILED);
    }

    if (status & HSFSTS_CTL_FCERR) {
        return (FAILED);
    } else if (status & HSFSTS_CTL_AEL) {
        return (FAILED);
    }

    return (PASSED);
	
}


/*******************************************************************************
 *
 * Function    : denverton_spi_read_block
 * Description : Function to read data from Denverton SPI Controller data buffer.
 * Inputs      : buf  - pointer to data buffer
 *                  size - size of wanted read data
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_read_block(uchar *buf, int size)
{
    int bytes;
    int ix = 0, jx = 0;
    int ret;
    uint val;

    if (size > INTEL_SPI_FIFO_SZ) {
        return (FAILED);
    }

    while (size > 0) {
        bytes = size;
        val = 0;
        ret = denverton_spi_ctrl_mm_read32((FDATA(ix)), &val);
        if (ret != PASSED) {
            printf("%s: Failed to read Denverton SPI Controller memory.\n", __FUNCTION__);
            return (FAILED);
        }
	
        for (jx = 0; jx < BYTES_READ; jx++) {
            *buf=(uchar)((val >> (jx * BYTE_SHIFT)) & BYTE_MASK);
            size -= 1;
            buf += 1;
            if (size <= 0) {
                break;
            }
        }
		
        ix++;
    }

    return (PASSED);

}


/*******************************************************************************
 *
 * Function    : denverton_spi_write_block
 * Description : Function to write data to Denverton SPI Controller data buffer.
 * Inputs      : buf  - pointer to data buffer
 *               size - size of wanted write data
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_write_block(uchar *buf, int size)
{
    int bytes;
    int ret;
    int ix = 0, jx = 0;
    uint val;

    if (size > INTEL_SPI_FIFO_SZ) {
        return (FAILED);
    }

    while (size > 0) {
        bytes = size;
        val = 0;
        for (jx = 0; jx < BYTES_READ; jx++) {
            val |= (uchar)((*buf<<(jx*BYTE_SHIFT))&BYTE_MASK);
            size -= 1;
            buf += 1;
            if (size <= 0) {
                break;
            }
        }
		
        ret = denverton_spi_ctrl_mm_write32((FDATA(ix)), val);
        if (ret != PASSED) {
            printf("%s: Failed to write Denverton SPI Controller memory.\n", __FUNCTION__);
            return (FAILED);
        }
	
        ix++;
    }

	return (PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_read_reg
 * Description : Function to read SPI Flash register via Denverton SPI Controller.
 * Inputs      : opcode  - opcode
 *               buf - pointer to data buffer
 *               len - size of wanted read data from register
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_read_reg(uchar opcode, uchar *buf, int len)
{
    int ret;
	
    ret = denverton_spi_ctrl_mm_write32(FADDR, 0);
    if (ret != PASSED) {
        printf("%s: Failed to write Denverton SPI Controller memory.\n", __FUNCTION__);
        return (FAILED);
    }   

    ret = denverton_spi_hw_cycle(opcode, len);
    if (ret != PASSED) {
        printf("%s: Failed to issue Denverton SPI Controller operation.\n", __FUNCTION__);
        return (FAILED);
    } 

    ret = denverton_spi_read_block(buf, len);
    if (ret != PASSED) {
        printf("%s: Failed to read Denverton SPI Controller data block.\n", __FUNCTION__);
        return (FAILED);
    } 

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_write_reg
 * Description : Function to write SPI Flash register via Denverton SPI Controller.
 * Inputs      : opcode  - opcode
 *               buf - pointer to data buffer
 *               len - size of wanted write data to register
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int denverton_spi_write_reg(uchar opcode, uchar *buf, int len)
{
    int ret;
	
    ret = denverton_spi_ctrl_mm_write32(FADDR, 0);
    if (ret != PASSED) {
        printf("%s: Failed to write Denverton SPI Controller memory.\n", __FUNCTION__);
        return (FAILED);
    }  

    ret = denverton_spi_write_block(buf, len);
    if (ret != PASSED) {
        printf("%s: Failed to write Denverton SPI Controller data block.\n", __FUNCTION__);
        return (FAILED);
    } 

    ret = denverton_spi_hw_cycle(opcode, len);
    if (ret != PASSED) {
        printf("%s: Failed to issue Denverton SPI Controller operation.\n", __FUNCTION__);
        return (FAILED);
    } 

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_write_status_reg
 * Description : Function to write Denverton SPI Flash status register (WRSR)
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int denverton_spi_write_status_reg (int select_flash)
{
    uchar wrsr;
    uint wr_data;
    uint reg_addr;
    reg_addr = FPGA_SPI_CONTROL_REG;
	
    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);
	 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH) { 
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
	 /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);
	 
    }

    wrsr = (uchar)gethex_answer("Enter byte to write to Denverton SPI WRSR: ", 0, 0, 0xFF);

    if (denverton_spi_write_reg(SPINOR_OP_WRSR, &wrsr, 1) == FAILED) {
        cterr('f', 0, "Unable to write Denverton SPI WRSR register (WRSR)");
	return(FAILED);
    }
    printf("Successfully wrote %#.2x to Denverton SPI WRSR register\n", wrsr);

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        /* Set FPGA bootflash mux back to default value.*/
        dash_fpga_reg_write(reg_addr, 0);
    }

    return(PASSED);
}


/*******************************************************************************
 *
 * Function    : denverton_spi_read_status_reg
 * Description : Function to read Denverton SPI Flash status register (RDSR)
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int denverton_spi_read_status_reg (int select_flash)
{
    uchar rdsr;
    uint wr_data;
    uint reg_addr;
    reg_addr = FPGA_SPI_CONTROL_REG;
	
    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);
	 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH) { 
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
	 /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);
	 
    }

    if (denverton_spi_read_reg(SPINOR_OP_RDSR, &rdsr, 1) == FAILED) {
        cterr('f', 0, "Unable to read SPI status register (RDSR)\n");
	return(FAILED);
    }
    printf("\nDenverton SPI RDSR = %#.2x\n", rdsr);

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        /* Set FPGA bootflash mux back to default value.*/
        dash_fpga_reg_write(reg_addr, 0);
    }

    return(PASSED);

}


/*******************************************************************************
 *
 * Function    : denverton_spi_read_id
 * Description : Function to read Denverton SPI Flash ID (RDID)
 * Inputs      : prom_id - pointer to SPI Flash ID data.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int denverton_spi_read_id(uint *prom_id)
{    
    int ret = 0;
    uchar data[SPI_MAX_ID_LEN];

    ret = denverton_spi_read_reg(SPINOR_OP_RDID, data, SPI_MAX_ID_LEN);
    if (ret != PASSED) {
        printf("%s: Failed to read Denverton SPI Flash ID (RDID).\n", __FUNCTION__);
        return (FAILED);
    } 

    *prom_id = data[SPI_ID_BYTE_0] << SPI_ID_BIT_16 | 
                       data[SPI_ID_BYTE_1] << SPI_ID_BIT_8 | 
                       data[SPI_ID_BYTE_2] << SPI_ID_BIT_0;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_utils
 * Description : Function to show SPI Flash utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_utils (int opt)
{
    build_primary_submenu(spi_flash_utils_tbl, SPI_FLASH_UTILS_TBL_SIZE,
                          "SPI Flash Utilities", &spi_flash_utils_menup);
    build_secondary_submenu(spi_flash_utils_tbl, SPI_FLASH_UTILS_TBL_SIZE,
                            spi_flash_utils_sec_items);

    menu(spi_flash_utils_menup, spi_flash_utils_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_get_prod_id
 * Description : Function to read SPI Flash ID (RDID)
 * Inputs      : select_flash - number of select flash
                     spi_id - pointer to spi ID data
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_get_prod_id (int select_flash, uint *spi_id) 
{
    int ret = 0;
    uint reg_addr;
    uint wr_data;
    reg_addr = FPGA_SPI_CONTROL_REG;
	
    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);
        
        ret = denverton_spi_read_id(spi_id);
        if (ret != PASSED) {
            printf("%s: Failed to read Denverton SPI Flash ID (RDID).\n", __FUNCTION__);
            return (FAILED);
        } 
	 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH) { 
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
	 /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);

        ret = denverton_spi_read_id(spi_id);
	 if (ret != PASSED) {
            printf("%s: Failed to read Denverton SPI Flash ID (RDID).\n", __FUNCTION__);
            return (FAILED);
        } 
	 
    } else if (select_flash ==FPGA_SPI_FLASH) {
        ret = fpga_spi_read_id(spi_id);
	 if (ret != PASSED) {
            printf("%s: Failed to read FPGA SPI Flash ID (RDID).\n", __FUNCTION__);
            return (FAILED);
        } 
		
    } else {
	 printf("%s: Unsupported seleceted SPI Flash number %d.\n", __FUNCTION__, select_flash);
        return (FAILED);
    }

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        /* Set FPGA bootflash mux back to default value.*/
        dash_fpga_reg_write(reg_addr, 0);
    }
    return (PASSED); 
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_get_prod_name
 * Description : Function to read SPI flash product name (RDID)
 * Inputs      : select_flash - number of select flash
                     buf - pointer to buf data
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_get_prod_name (int select_flash, char *buf) 
{
 
    uint spi_prod_id = 0;
    int ret = 0;

    ret = nanook_spi_flash_get_prod_id(select_flash, &spi_prod_id);
    if (ret != PASSED) {
        return (FAILED);
    } 

    switch (spi_prod_id) {
    case SPI_W25Q128JV_ID:
        sprintf(buf, SPI_W25Q128JV_PROD_NAME);
        break;
    case SPI_MX25L12833F_ID:
        sprintf(buf, SPI_MX25L12833F_PROD_NAME);
        break;
    case SPI_W25Q16JV_ID:
        sprintf(buf, SPI_W25Q16JV_PROD_NAME);
        break;
    case SPI_MX25V1635F_ID:
        sprintf(buf, SPI_MX25V1635F_PROD_NAME);
        break;
    case SPI_MX25L6433F_ID:
        sprintf(buf, SPI_MX25L6433F_PROD_NAME);
        break;
    case SPI_MX25L1606E_ID:
        sprintf(buf, SPI_MX25L1606E_PROD_NAME);
        break;
    case SPI_GD25Q16C_ID:
        sprintf(buf, SPI_GD25Q16C_PROD_NAME);
        break;
    default:
 	 printf("%s: Unsupported seleceted SPI Flash ID 0x%x.\n", __FUNCTION__, spi_prod_id);
 	 return (FAILED); 
    }

    return (PASSED); 
    
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_get_rdsr
 * Description : Function to read SPI flash status register (RDSR)
 * Inputs      : select_flash - number of select flash
                     rdsr_val - pointer to store rdsr data
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_get_rdsr (int select_flash, uchar * rdsr_val) 
{
    int ret = 0;
    uint reg_addr;
    uint wr_data;
    uchar rdsr = 0; 

    reg_addr = FPGA_SPI_CONTROL_REG;

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);

        ret = denverton_spi_read_reg(SPINOR_OP_RDSR, &rdsr, 1);
	 if (ret != PASSED) {
            printf("%s: Failed to read Denverton SPI Flash status register (RDSR).\n", __FUNCTION__);
            return (FAILED);
        } 
	 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH) { 
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
	 /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);

        ret = denverton_spi_read_reg(SPINOR_OP_RDSR, &rdsr, 1);
        if (ret != PASSED) {
            printf("%s: Failed to read Denverton SPI Flash status register (RDSR).\n", __FUNCTION__);
            return (FAILED);
        } 		 
	 
    } else if (select_flash ==FPGA_SPI_FLASH) {
    
        ret = read_spi_prom_status_reg(&rdsr);
        if (ret != PASSED) {
            printf("%s: Failed to read FPGA SPI Flash status register (RDSR).\n", __FUNCTION__);
            return (FAILED);
        } 
		
    } else {
	 cterr('f',0,"Unsupported seleceted SPI Flash number %d.\n", select_flash);
        return (FAILED);
    }

    *rdsr_val = rdsr; 

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        /* Set FPGA bootflash mux back to default value.*/
        dash_fpga_reg_write(reg_addr, 0);
    }
	
    return (PASSED);
    
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_set_wrsr
 * Description : Function to write SPI flash status register (WRSR)
 * Inputs      : select_flash - number of select flash
                     wrsr_val - wanted write data
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_set_wrsr (int select_flash, uchar wrsr_val) 
{
    int ret = 0;
    uint reg_addr;
    uint wr_data;

    reg_addr = FPGA_SPI_CONTROL_REG;

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);

        ret = denverton_spi_write_reg(SPINOR_OP_WRSR, &wrsr_val, 1);
        if (ret != PASSED) {
            printf("%s: Failed to write Denverton SPI Flash status register (WRSR).\n", __FUNCTION__);
            return (FAILED);
        } 
	 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH) { 
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
	 /* nanook uses fpga to mux the bootflash */
        dash_fpga_reg_write(reg_addr, wr_data);

        ret = denverton_spi_write_reg(SPINOR_OP_WRSR, &wrsr_val, 1);
        if (ret != PASSED) {
            printf("%s: Failed to write Denverton SPI Flash status register (WRSR).\n", __FUNCTION__);
            return (FAILED);
        } 
	 
    } else if (select_flash ==FPGA_SPI_FLASH) {
        prom_t *spi_prom;

        spi_prom = init_spi_prom_ds(); 
        ret = write_spi_prom_status_reg(spi_prom, wrsr_val);
        if (ret != PASSED) {
            printf("%s: Failed to write FPGA SPI Flash status register (RDSR).\n", __FUNCTION__);
            return (FAILED);
        } 
		
    } else {
	 cterr('f',0,"Unsupported seleceted SPI Flash number %d.\n", select_flash);
        return (FAILED);
    }

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        /* Set FPGA bootflash mux back to default value.*/
        dash_fpga_reg_write(reg_addr, 0);
    }
	
    return (PASSED);
    
}


/*******************************************************************************
 *
 * Function    : nanook_spi_flash_show_info
 * Description : Function to show SPI Flash information
 * Inputs      : select_flash - number of select flash
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_show_info (int select_flash) 
{
    int ret = 0;
    char prod_name[NAME_LEN];
    char flash_name[NAME_LEN];
    uchar rdsr = 0;

    if ( select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH 
		|| select_flash == FPGA_SPI_FLASH ) {
        switch (select_flash) {
        case GOLDEN_ROMMON_SPI_FLASH:
            sprintf(flash_name, GOLDEN_ROMMON_SPI_FLASH_NAME);
            break;
        case UPGRADE_ROMMON_SPI_FLASH:
            sprintf(flash_name, UPGRADE_ROMMON_SPI_FLASH_NAME);
            break;
        case FPGA_SPI_FLASH:
            sprintf(flash_name, FPGA_SPI_FLASH_NAME);
            break;
        default:
 	     cterr('f',0,"Unsupported Selected SPI flash number: %d\n", select_flash);
 	     return (FAILED);
	     break;
        }
    } else {
        cterr('f',0,"Unsupported Selected SPI flash number: %d\n", select_flash);
 	 return (FAILED); 
    }
    
    ret = nanook_spi_flash_get_prod_name(select_flash, prod_name);
    if (ret != PASSED) {
        printf("%s: Failed to get SPI Flash product name.\n", __FUNCTION__);
        return (FAILED);
    } 

    ret = nanook_spi_flash_get_rdsr(select_flash, &rdsr);
    if (ret != PASSED) {
        printf("%s: Failed to read SPI Flash status register.\n", __FUNCTION__);
        return (FAILED);
    } 

    printf("\nCurrent Flash Info\n");
    printf("Flash Name    : %s\n", flash_name);	
    printf("Product Name  : %s\n", prod_name);	
    printf("Status Register Protect (SRP0) : %s.\n",
           ((rdsr & SPI_FLASH_STATUS_REG_PROTECT_EN) == SPI_FLASH_STATUS_REG_PROTECT_EN) ?
           "Enabled" : "NOT enabled");
    return (PASSED);
    
}

#if 0
/*******************************************************************************
 *
 * Function    : nanook_spi_flash_write_protect_enable
 * Description : Function to set SPI Flash write protect enable (SRP0 & BP setting).
 * Inputs      : select_flash - number of select flash
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nanook_spi_flash_write_protect_enable (int select_flash) 
{
    uint spi_prod_id = 0;
    int ret = 0;
    uchar wrsr = 0;
    uchar rdsr = 0;

    ret = nanook_spi_flash_get_prod_id(select_flash, &spi_prod_id);
    if (ret != PASSED) {
        return (FAILED);
    } 
	
    switch (spi_prod_id) {
    case SPI_W25Q128JV_ID:
        wrsr = SPI_W25Q128JV_PROTECTED_256KB_BP_CONF;
        break;
    case SPI_MX25L12833F_ID:
        wrsr = SPI_MX25L12833F_PROTECTED_256KB_BP_CONF;
        break;
    case SPI_MX25L6433F_ID:
        wrsr = SPI_MX25L6433F_PROTECTED_1M_BP_CONF;
        break;
    case SPI_MX25L1606E_ID:
        wrsr = SPI_MX25L1606E_PROTECTED_1M_BP_CONF;
        break;
    case SPI_GD25Q16C_ID:
        wrsr = SPI_GD25Q16C_PROTECTED_1M_BP_CONF;
        break;
    default:
 	 printf("%s: Unsupported seleceted SPI Flash ID 0x%x.\n", __FUNCTION__, spi_prod_id);
 	 return (FAILED);  
    }

    ret = nanook_spi_flash_set_wrsr(select_flash, wrsr);    
    if (ret != PASSED) {
        printf("%s: Failed to write SPI Flash status register.\n", __FUNCTION__);
        return (FAILED);
    } 

    wrsr |=  SPI_FLASH_STATUS_REG_PROTECT_EN;

    ret = nanook_spi_flash_set_wrsr(select_flash, wrsr);  
    if (ret != PASSED) {
        printf("%s: Failed to write SPI Flash status register.\n", __FUNCTION__);
        return (FAILED);
    } 
 
    ret = nanook_spi_flash_get_rdsr(select_flash, &rdsr);
    if (ret != PASSED) {
        printf("%s: Failed to read SPI Flash status register.\n", __FUNCTION__);
        return (FAILED);
    } 

    if (rdsr != wrsr) {
        printf("\n%s: Failed to enable SPI Flash write protect. Write data 0x%x, read back 0x%x\n", __FUNCTION__, wrsr, rdsr);
        return (FAILED);
    } else {
        printf("\nEnable SPI Flash write protect has done.\n");
    }
	
    return (PASSED);
	
}

/******************************************************************************
 *
 * Function: diag_spi_flash_write_protect_test
 *
 * Description: SPI Flash write protect test
 *
 * Inputs      : select_flash - select bootflash number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_spi_flash_write_protect_test(int select_flash) {

    char *tname[] = {GOLDEN_ROMMON_SPI_FLASH_NAME, UPGRADE_ROMMON_SPI_FLASH_NAME, FPGA_SPI_FLASH_NAME};
    uint reg_addr;
    uint wr_data;
    uchar rdsr = 0;
#ifdef SPI_FLASH_WRSR_TEST
    uchar rdsr_bk = 0;
    uchar wrsr = 0;
#endif
    int ret;

    reg_addr = FPGA_SPI_CONTROL_REG;

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH) {	
        wr_data = ENABLE_FIRST_BOOTFLASH;
        testname("%s write protect", tname[GOLDEN_ROMMON_SPI_FLASH]); 
    } else if (select_flash == UPGRADE_ROMMON_SPI_FLASH){
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
        testname("%s write protect", tname[UPGRADE_ROMMON_SPI_FLASH]);	
    } else if (select_flash == FPGA_SPI_FLASH){
        testname("%s write protect", tname[FPGA_SPI_FLASH]);	
    } else {
        printf("Wrong index of select SPI Flash\n");
	 return (FAILED);
    }

    /* Check SPI Flash SRP0 bit First, if SRP0 not set, just failed the test and don't do actual read/write test. */
    ret = nanook_spi_flash_get_rdsr(select_flash, &rdsr);
    if (ret != PASSED) {
        printf("%s: Failed to read SPI Flash status register.\n", __FUNCTION__);
        cterr('f', 0, "SPI Flash write protect test failed.");
        return (FAILED);
    } 
	
    if ((rdsr & SPI_FLASH_STATUS_REG_PROTECT_EN) != SPI_FLASH_STATUS_REG_PROTECT_EN) {
        printf("%s: Status Register Protect (SRP0) is not set.\n", __FUNCTION__);
        cterr('f', 0, "SPI Flash write protect test failed.");
        return (FAILED);        
    } 

#ifdef SPI_FLASH_WRSR_TEST
    /* Back up current status register value. */
    rdsr_bk = rdsr;

    wrsr = SR_WRITE_PROTECT_TEST_PATTERN;

    /* Test if status register writable or not. */
    ret = nanook_spi_flash_set_wrsr(select_flash, wrsr);    
    if (ret != PASSED) {
        printf("%s: Failed to write SPI Flash status register.\n", __FUNCTION__);
        return (FAILED);
    }

    ret = nanook_spi_flash_get_rdsr(select_flash, &rdsr);
    if (ret != PASSED) {
        printf("%s: Failed to read SPI Flash status register.\n", __FUNCTION__);
        cterr('f', 0, "SPI Flash write protect test failed.");
        return (FAILED);
    }

    /* Status register still writable, failed the test and return. */
    if (rdsr == wrsr) {

        wrsr = rdsr_bk;

        printf("%s: SPI Flash status register still writeable.\n", __FUNCTION__);
        cterr('f', 0, "SPI Flash write protect test failed.");
        
        ret = nanook_spi_flash_set_wrsr(select_flash, wrsr);    
        if (ret != PASSED) {
            printf("%s: Failed to write SPI Flash status register.\n", __FUNCTION__);
            return (FAILED);
        }

        return (FAILED);
    }
#endif

    if (select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
    	 /*
         * nanook uses fpga to mux the bootflash
         */
        dash_fpga_reg_write(reg_addr, wr_data);
        prpass(testpass, "switch flash done");		
    }

    if (spi_flash_write_protect_test(select_flash) != PASSED) {
        cterr('f', 0, "SPI Flash write protect test failed.");
        return (FAILED);
    }

    prpass(testpass, "SPI Flash write protect test passed, ");
    prcomplete(testpass, errcount, (char*)0);
    dash_fpga_reg_write(reg_addr, 0);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :    spi_flash_write_protect_test
 * Description:    function for SPI Flash device path
 * Inputs     :    select_flash - select bootflash number
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int spi_flash_write_protect_test (int select_flash)
{
    char src[BUFFER_LEN];
    int retval;
    sprintf(src, BOOT_FLASH_DEV_PATH);

    if(select_flash == GOLDEN_ROMMON_SPI_FLASH || select_flash == UPGRADE_ROMMON_SPI_FLASH) {
        retval = mtd_spi_flash_write_protect_test(src);
    } else if (select_flash == FPGA_SPI_FLASH) {
        retval = fpga_spi_flash_write_protect_test();
    } else {
        return (FAILED);
    }
    return (retval);
}

/*******************************************************************************
 *
 * Function   :    mtd_spi_flash_write_protect_test
 * Description:    main test for mtd spi flash write protect area test
 * Inputs     :    file path to mtd device
 * Outputs    :    PASSED or FAILED.
 *
 *******************************************************************************
 */
int mtd_spi_flash_write_protect_test (char *src)
{
    char buf[BUFFER_LEN];
    char buf_bk[WRITE_PROTECT_TEST_LEN], buf_wr[WRITE_PROTECT_TEST_LEN];
    char buf_rd[WRITE_PROTECT_TEST_LEN]; 
    char *p1 = buf_bk;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, jx, cnt = 0;
    uint sector;

    prpass(testpass, "Access device '%s' , ", src);
    sprintf(buf, "%s", src);

    printf("\n %s \n", src);
    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    /* From Rommon SPI flash write protect from 0xFC0000 - 0xFFFFFF*/
    sector = SECTOR_252 * SPI_PROM_SECTOR_SIZE;

    for (ix = 0; ix < SPI_FLASH_DEV_OPEN_RETRY_MAX; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
        if (devfd < 0) {
            sleep(SPI_FLASH_SLEEP_SECOND);
            continue;
        } else {
            break;
        }
    }

    if (ix >= SPI_FLASH_DEV_OPEN_RETRY_MAX) {
        cterr('f', 0, "there is no device file descriptor available.");
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * back up data
     */
    prpass(testpass, "Backup data , ");

    if (lseek(devfd, sector, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("backup lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }
	
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }
	
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n backup data\n");
        for (jx = 0; jx < WRITE_PROTECT_TEST_LEN; jx++) {
            printf("%x ",buf_bk[jx]);
        }
        printf("\n num = %d\n",num);
    }

    /*
     * prepare data pattern
     */
    prpass(testpass, "Prepare data pattern , ");
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    prpass(testpass, "Write data pattern , ");
    if (lseek(devfd, sector, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("write lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) < 0) {
        /*Skip failed and keep continue check read data and backup data.*/
        printf("Unable to write data pattern to device num = %d.", num);
    }

    if (fsync(devfd) < 0) {
         /*Skip failed and keep continue check read data and backup data.*/
        printf("Unable to sync data pattern to device.");
    }

    /*Close device and re-open device to make sure the file sync.*/
    close(devfd);
    for (ix = 0; ix < SPI_FLASH_DEV_OPEN_RETRY_MAX; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
        if (devfd < 0) {
            sleep(SPI_FLASH_SLEEP_SECOND);
            continue;
        } else {
            break;
        }
    }

    if (ix >= SPI_FLASH_DEV_OPEN_RETRY_MAX) {
        cterr('f', 0, "there is no device file descriptor available.");
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * read back data for comparing
     */
    prpass(testpass, "Read back data for comparing , ");
    if (lseek(devfd, sector, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n read back data\n");
        for (jx = 0; jx < WRITE_PROTECT_TEST_LEN; jx++) {
            printf("%x ",buf_rd[jx]);
        }
        printf("\n num = %d\n",num);
    }

    /*
     * comparing data between read data and backup data, since it is write protect, the data should be the same.
     */
    prpass(testpass, "Comparing read data to backup data, ");
    for (ib = 0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            cterr('f', 0, "failed on byte %d, backup data = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");

            /*
            *  Since read data and backup-data are different, start to restore data before exit...
            */
            prpass(testpass, "Restore data , ");
            if (lseek(devfd, sector, SEEK_SET) < 0) {
                close(devfd);           /* don't need it anymore */
                cterr('f', 0, "lseek to the beginning of device failed.");
                return (FAILED);
            }

            if ((num = write(devfd, buf_bk, sizeof(buf_bk))) < 0) {
                close(devfd);           /* don't need it anymore */
                cterr('f', 0,
                      "Write restore data failed, can not write to drive.\n");
                return (FAILED);
            }

            if (num != sizeof(buf_bk)) {
                close(devfd);           /* don't need it anymore */
                cterr('f', 0, "not all the bytes are written for restore");
                return (FAILED);
            }

            if (fsync(devfd) < 0) {
                close(devfd);           /* don't need it anymore */
                cterr('f', 0, "fsync failed.");
                return (FAILED);
            }
			
            close(devfd);
            return (FAILED);
			 
        }
    }

    close(devfd);         
    return (PASSED);

}


/*******************************************************************************
 *
 * Function   :    fpga_spi_flash_write_protect_test
 * Description:    main test for fpga spi flash write protect area test
 * Inputs     :    void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int fpga_spi_flash_write_protect_test (void)
{
    int ret;
    uint sector;
    uint spi_prod_id = 0;

    ret = nanook_spi_flash_get_prod_id(FPGA_SPI_FLASH, &spi_prod_id);
    if (ret != PASSED) {
        return (FAILED);
    } 	

    /* Both configure 1024 kB for write protect, so test first and last write protect sector. */
    sector = SECTOR_00;
    ret = fpga_spi_write_protect_test(sector);
    if (ret != PASSED) {
        printf("%s: FPGA SPI Flash write protect test failed for sector: %d\n", __FUNCTION__, sector);
        return (FAILED);
    } 
    sector = SECTOR_15;
    ret = fpga_spi_write_protect_test(sector);
    if (ret != PASSED) {
        printf("%s: FPGA SPI Flash write protect test failed for sector: %d\n", __FUNCTION__, sector);
        return (FAILED);
    } 
		
    return (PASSED);
	
}
#endif

/*-------------------------------------------------------------------
 *
 * Function: fpga_spi_read_id
 *
 * Enable FPGA SPI Flash ID.
 *
 * Input: prom_id -- The pointer to store SPI Flash ID data.
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int fpga_spi_read_id (uint *prom_id)
{
    volatile uchar data[SPI_MAX_ID_LEN];
    ulong ctrl_flag, count;
    int temp_data;
    ushort ix;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    ctrl_flag = PROM_DFLT_BAUD;
    count = SPI_MAX_ID_LEN;


    /* Read the SPI PROM Read ID Register (RDID) */
    spi_prom->size        = DSWAP4(count);
    spi_prom->opcode_addr = DSWAP4(PROM_RDID_OP);
    spi_prom->control     = DSWAP4(ctrl_flag); /* Read */

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
        cterr('f',0,"%s: read operation not done", __FUNCTION__);
        return(FAILED);
    }

    for (ix = 0; ix < count; ix++) {
        temp_data = spi_prom->data;
        data[ix] = (uchar)(DSWAP4(temp_data));
    }

    *prom_id = data[SPI_ID_BYTE_0] << SPI_ID_BIT_16 |
                       data[SPI_ID_BYTE_1] << SPI_ID_BIT_8 |
                       data[SPI_ID_BYTE_2] << SPI_ID_BIT_0;

    return (PASSED);
}

/******** History ********
$Log: diag_spi_flash_util.c,v $
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/




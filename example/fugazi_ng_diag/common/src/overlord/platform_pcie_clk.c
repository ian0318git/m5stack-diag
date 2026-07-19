/* $Id: platform_pcie_clk.c,v 1.12 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pcie_clk.c,v $
 *------------------------------------------------------------------
 * platform_pcie_clk.c - Overlord PCIe CLK main function/menu
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "platform_pcie_clk.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "i2c_dev.h"
#include "linux_api.h"
#include "ich_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "i2c_address.h"
#include "goofy_i2c.h"

/*******************************************************************************
 * PCIe clock Register Table
 * Not sure the offset address, 
 * so the base address are temporary defined from 00 to 24
 * the 0xFFFFFFFF is used for break while loop on line 742
 *******************************************************************************
 */

typedef struct pcie_clk_reg_info_ {
    char     *name;
    uint32_t address;
} pcie_clk_reg_info_t;

pcie_clk_reg_info_t pcie_clk_reg_tbl[] = {
    { "Frequecy Select Register          ", 0x00 },
    { "Output Control Register1          ", 0x01 },
    { "Output Control Register2          ", 0x02 },
    { "Output Enable readback1           ", 0x03 },
    { "Output Enable readback2           ", 0x04 },
    { "Vender And Revisvion ID Register  ", 0x05 },
    { "Device ID                         ", 0x06 },
    { "Byte Control Register             ", 0x07 },
    { ""                                  , 0xFFFFFFFF }
};

/*******************************************************************************
 *                            Function proto
 *******************************************************************************
 */
static int init_pcie_clk_i2c(n2g_i2c_if_t *);
static int dump_reg(int);
static int check_device_id (void);
static int write_reg (void);
static int get_pcie_clk_buf_i2c_struct(n2g_i2c_if_t *);

void build_pcie_clk_util_menu(void);
int platform_pcie_clk_reg_test(void);

extern uint32_t ich_i2c_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
extern uint32_t ich_i2c_write(n2g_i2c_dev_t *,  ulong, uint8_t, char *);

/* 9DBV0841 (neptune) device id is 0x48 */
/* 9ZXL1231 (utah) device id is 0xE7 */
/* 9ZX21901 (o2)   device id is 0xDB */
/* 9DBU0231 (GB)   device id is 0x42 */
#define NEPTUNE_PCIE_CLK_DEVICE_ID 0x48
#define UTAH_PCIE_CLK_DEVICE_ID 0xE7 
#define O2_PCIE_CLK_DEVICE_ID 0xDB
#define GB_PCIE_CLK_DEVICE_ID 0x42
/*******************************************************************************
 *                                 Globals 
 *******************************************************************************
 */


/*******************************************************************************
 *                                   Menus
 *******************************************************************************
 */
/*********************************************************************
 *		I2C devices characteristics tables. 
 *    this not the full table from i2c_api.c
 *    and it should follow the order (enum) defined in platform_i2c.h
 *********************************************************************
 */
/*
I will probe file /dev/i2c-0, address 0x6c, mode smbus block
Continue? [Y/n] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 39 ff ff 83 00 11 db 08                            9..?.???
(none)Overlord-diag#
(none)Overlord-diag# ./i2cdump 0 0x6c s
*/
static n2g_i2c_if_t pcie_clk[] = {
    {
        .offset = 0,
        .i2c_bus_type = CPU_I2C1,
        .i2c_dev = MB_I2C_ADDR_PCIE_CLK,
        .size = sizeof(uint32_t), /* doesn't matter; it's a block access */
        .sub_addr_len = 0,
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = 0,
        .buf = NULL,
    },
    {
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C, 
        .i2c_dev = MB_I2C_ADDR_CLK_BUFFER,
        .i2c_ctrl = I2C_CTRL_FIVE,
        .size = sizeof(uint32_t), /* doesn't matter; it's a block access */
        .sub_addr_len = 0,
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = 1,
        .buf = NULL,
    },

};

 
/*
 * PCIe CLK utility menu tables
 */
static submenu_xtable_t pcie_clk_util_table[] = {
    {"Dump all PCIe CLK regs",   (PFT)dump_reg,             0,  
     0, (type_t(*)())0, 0,       (type_t(*)())0, 0},
    {"Write PCIe CLK regs",   (PFT)write_reg,             0,  
     0, (type_t(*)())0, 0,       (type_t(*)())0, 0},
};

#define PCIE_CLK_UTIL_TABLE_SZ \
        (sizeof(pcie_clk_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pcie_clk_util_primary_items[PCIE_CLK_UTIL_TABLE_SZ +
                                           MAX_BASE_ITEMS];
static mitem_t pcie_clk_util_secondary_items[PCIE_CLK_UTIL_TABLE_SZ +
                                             MAX_BASE_ITEMS];

static menuinfo_t pcie_clk_util_menu = {
    "PCIe CLK Utility Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    pcie_clk_util_primary_items,
};

menuinfo_t *pcie_clk_util_submenup = &pcie_clk_util_menu;

/*
 * submenu for Overlord PCIe CLK tests
 */
static submenu_xtable_t pcie_clk_test_table[] = {
    {"PCIe CLK utilites",  (PFT)build_pcie_clk_util_menu,      0,
     0, 
     (type_t(*)())0, 0,    (type_t(*)())0,                     0},
    {"PCIe CLK Check Device ID",  (PFT)check_device_id,        0, 
     0, 
     (type_t(*)())0, 0,    (type_t(*)())0,                     0}
};

#define PCIE_CLK_TEST_TABLE_SIZE  \
    (sizeof(pcie_clk_test_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pcie_clk_tests_primary_items[PCIE_CLK_TEST_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t pcie_clk_tests_secondary_items[PCIE_CLK_TEST_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

menuinfo_t pcie_clk_testmenu = {
    "PCIe Clock Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    pcie_clk_tests_primary_items,
};

menuinfo_t *pcie_clk_testmenup = &pcie_clk_testmenu;



/*******************************************************************************
 * Function   : build_pcie_clk_util_menu
 * Description: To build Overlord PCIe CLK utility submenu.
 * Inputs     : None.
 * Outputs    : None.
 *******************************************************************************
 */
void build_pcie_clk_util_menu (void)
{
    build_primary_submenu(pcie_clk_util_table, PCIE_CLK_UTIL_TABLE_SZ,
			                    "PCIe CLK Utility Submenu", &pcie_clk_util_submenup);
    build_secondary_submenu(pcie_clk_util_table, PCIE_CLK_UTIL_TABLE_SZ,
			                      pcie_clk_util_secondary_items);
    menu(pcie_clk_util_submenup, pcie_clk_util_secondary_items, 0);
}


/*******************************************************************************
 * Function   : build_pcie_clk_test_menu
 *
 * Description: To build Overlord PCIe CLK Tests Main menu.
 *
 * Inputs     : None.
 *
 * Outputs    : None.
 *******************************************************************************
 */
void build_pcie_clk_test_menu (void)
{
    build_primary_submenu(pcie_clk_test_table, PCIE_CLK_TEST_TABLE_SIZE,
			                    "PCIe CLK Tests Menu", &pcie_clk_testmenup);
    build_secondary_submenu(pcie_clk_test_table, PCIE_CLK_TEST_TABLE_SIZE,
			                      pcie_clk_tests_secondary_items);
    menu(pcie_clk_testmenup, pcie_clk_tests_secondary_items, 0);
}

/*******************************************************************************
 * Function   : init_pcie_clk_i2c
 *
 * Description: To init the i2c and dev_addr bus num for i2c_read/write. 
 *              
 * Inputs     : None.
 *
 * Outputs    : None.
 *******************************************************************************
 */
int init_pcie_clk_i2c (n2g_i2c_if_t *i2c_if)
{

    if (is_neptune() || is_proteus() || is_triton() || is_neso() || is_vg450()) {
        memcpy(i2c_if, &pcie_clk[1], sizeof(n2g_i2c_if_t));
    } else {
        memcpy(i2c_if, &pcie_clk[0], sizeof(n2g_i2c_if_t));
    }

    if (is_goldbeach()) { 
        i2c_if->i2c_dev = GB_MB_I2C_ADDR_PCIE_CLK;
    }

    return(PASSED);
}
/*******************************************************************************
 * Function   :	platform_pcie_clk_reg_test
 *
 * Description:	PCIe CLK register test.
 *              For each register from reg_ptr, this function checks for 
 *              accessibility and does a ripple 1 and a ripple 0 test if 
 *              applicable (not all registers are W/R register).
 *
 * Inputs     :	None
 *
 * Outputs    : PASSED/FAILED.
 *******************************************************************************
 */
int platform_pcie_clk_reg_test (void)
{
    pcie_clk_reg_info_t *reg_ptr;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_dev_t dev;
    char ori_tmp[PCIE_CLK_BUF_SIZE], reg_tmp[PCIE_CLK_BUF_SIZE];
    char chk_tmp[PCIE_CLK_BUF_SIZE];
    char *original_data, *reg_data, *chk_data;
    uint  ix, ii, rc;
    uint temp, data, mask;

    /* initial i2c for i2c_read/write*/
    init_pcie_clk_i2c(&i2c_if);
    reg_ptr = &pcie_clk_reg_tbl[0];
    original_data = &ori_tmp[0];
    reg_data = &reg_tmp[0];
    chk_data = &chk_tmp[0];

    printf("%s : fix me for neptune\n", __FUNCTION__); 
    while (reg_ptr->address != 0xFFFFFFFF) {
        i2c_if.offset = reg_ptr->address;
        i2c_if.buf = original_data;
        
        rc = ich_i2c_read(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
            /* check result of read data */
            if (rc != PASSED) {
                printf("Ripple one FAILED to read from %s.",reg_ptr->name);
                return (FAILED);
            }
            /* 
             * ripple 1 test
             */
            for (ix = 0; ix < (sizeof(i2c_if.size) * 8); ix++) {
                temp = (1 << ix);
                if (!temp)
                    continue;
        *reg_data = temp;
        i2c_if.buf = reg_data;
        rc = ich_i2c_write(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Ripple one FAILED to write to %s.",reg_ptr->name);
            return (FAILED);
        }

        i2c_if.buf = chk_data;
        rc = ich_i2c_read(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Ripple one FAILED to read back %s.",reg_ptr->name);
            return (FAILED);
        }

		if ((*chk_data) != (*reg_data)) {
        printf("Ripple one FAILED, not the same value\n");
		    return(FAILED);
                }
            }

            /* 
             * ripple 0 test
             */
            for (ix = 0; ix < (sizeof(i2c_if.size) * 8); ix++) {
                temp = (1 << ix);
                if (!temp)
                    continue;
                temp = (~(1 << ix));

        *reg_data = temp;
        i2c_if.buf = reg_data;
        rc = ich_i2c_write(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Ripple zero FAILED to write to %s.",reg_ptr->name);
            return (FAILED);
        }

        i2c_if.buf = chk_data;
        rc = ich_i2c_read(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Ripple zero FAILED to read back %s.",reg_ptr->name);
            return (FAILED);
        }

		if ((*chk_data) != (*reg_data)) {
        printf("Ripple zero FAILED, not the same value\n");
		    return(FAILED);
                }
            }

	    /*
	     * pattern test
	     */
	    data = PATTERN;
	    for (ix = 0; ix < 2; ix++){
	    	/* build mask of size for pattern */
	    	for (ii = 0; ii < (sizeof(i2c_if.size)*8); ii++){
	    	  mask |= (1 << ii);
	    		}
		temp = data & mask;
		
		*reg_data = temp;
		i2c_if.buf = reg_data;
		if (!temp) {
		    continue;
		}
		    rc = ich_i2c_write(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Pattern test FAILED to write to %s.",reg_ptr->name);
            return (FAILED);
        }

        i2c_if.buf = chk_data;
        rc = ich_i2c_read(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            printf("Pattern test FAILED to read back %s.",reg_ptr->name);
            return (FAILED);
        }

		if ((*chk_data) != (*reg_data)) {
        printf("Pattern test FAILED, not the same value\n");
		    return(FAILED);
                }
   	data = ~PATTERN; /* complement data pattern */
        }
	    
	    /*
	     * restore reset value
	     */
	    i2c_if.buf = original_data;
	    rc = ich_i2c_write(&dev, i2c_if.offset, i2c_if.size, i2c_if.buf);

	    if (rc != PASSED) {
    printf("Restore value FAILED %s.",reg_ptr->name);
		return(FAILED);
	    }

	reg_ptr++;
    }
    return(PASSED);

}

/*******************************************************************************
 * Function   :	read_pcie_clk_reg
 *
 * Description:	read block of data
 *
 * Inputs     :	offset ->always 0
 *
 * Outputs    : buf: contains register values
 *              return PASSED/FAILED.
 *******************************************************************************
 */   
int 
read_pcie_clk_reg (unsigned char *buf) 
{
    int result = FAILED, shift = 0;
    n2g_i2c_if_t i2c_if;
    unsigned char reg_tmp[PCIE_CLK_BUF_SIZE+1];

    if (is_ntpn_machines() || is_vg450() || is_curie_1ru() || is_curie_2ru()) {
        result = get_pcie_clk_buf_i2c_struct(&i2c_if);
        if (result != PASSED) {
            printf("%s: Failed to get Power Sequencer I2C structure.\n",
               __FUNCTION__);
            return (result);
        }
        /* Setup I2C API parameter struct */
        i2c_if.buf = (char *)&reg_tmp;
        i2c_if.offset = 0; 
        i2c_if.size = sizeof(reg_tmp);
        result = n2g_i2c_read(&i2c_if);
        if (result != RC_I2C_OP_OK) {
            /* Unable to read data */
            cterr('f', 0, "%s: Failed to read Reg. ", __FUNCTION__);
            return (FAILED);
        }

        shift = 1;  /* clk chip need to shift 1 byte which is data length */

    } else { 
        /* initial i2c for i2c_read/write. */
        init_pcie_clk_i2c(&i2c_if);
        i2c_if.offset = 0;
        i2c_if.buf = (char *)reg_tmp;
        result = i2c_dev_rd((void *)&i2c_if);
        if (result < 0) {
            cterr('f' , 0, "%s: FAILED to read PCIe clock registers.(rc = %d)",
                            __FUNCTION__, result);
            return (FAILED);
        }
    }

    memcpy(buf, reg_tmp + shift, PCIE_CLK_BUF_SIZE);

    return (PASSED);
}

static int 
dump_reg (int reg_type) 
{
    unsigned char reg_tmp[PCIE_CLK_BUF_SIZE];

    if (read_pcie_clk_reg(reg_tmp) == FAILED) {
        cterr('f', 0, "unable to read pcie clk registers");
        return(FAILED);
    }

    printf("\n%#x; %#x; %#x; %#x; %#x; %#x; %#x; %#x\n",
           reg_tmp[0], reg_tmp[1], reg_tmp[2], reg_tmp[3],
           reg_tmp[4], reg_tmp[5], reg_tmp[6], reg_tmp[7]);

    printf("\n");
	
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_pcie_clk_i2c_scan_test
 * Description: This function to check Overlord PCIe Clock
 *              by reading register through I2C interface.
 * Inputs     : errbuf - buffer to put error messages
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_pcie_clk_i2c_scan_test (char *errbuf)
{
    n2g_i2c_if_t i2c_if;
    char reg_val[PCIE_CLK_BUF_SIZE];
    int result = FAILED;

    /* Init device structure */
    init_pcie_clk_i2c(&i2c_if);

    /* Get Registers value */
    i2c_if.buf = &reg_val[0];

    result = i2c_dev_rd((void *)&i2c_if);
    if (result < 0) {
        sprintf(errbuf, "%s: FAILED to read PCIe clock registers.(rc = %d)",
                        __FUNCTION__, result);
        return (FAILED);
    }

    return (PASSED);
}

static int get_pcie_clk_buf_i2c_struct (n2g_i2c_if_t *pcie_clk_buf_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FIVE, I2C_MUX_ZERO,
                                        (0xD6 >> 1));
    if (tmp == NULL) {
        printf("%s: Failed to get PCIe clk buffer I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(pcie_clk_buf_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}

/*******************************************************************************
 * Function   : check_device_id
 * Description: Check PCIe CLK Device ID.
 * Inputs     : none
 * Outputs    : PASSED/FAILED.
 *******************************************************************************
 */
static int check_device_id (void)
{
    uchar read_id;
    unsigned char reg_tmp[PCIE_CLK_BUF_SIZE];

    if (read_pcie_clk_reg(reg_tmp) == FAILED) {
        cterr('f', 0, "unable to read pcie clk registers");
        return(FAILED);
    } 
    
    read_id = reg_tmp[6];  /* offset6 is device id */

    if (read_id == O2_PCIE_CLK_DEVICE_ID) {
        printf("Devicd ID 0x%x for chip 9ZX21901 on O2/Juno\n", read_id);
    } else if (read_id == UTAH_PCIE_CLK_DEVICE_ID) {
        printf("Devicd ID 0x%x for chip 9ZXL1231 on Utah\n", read_id);
    } else if (read_id == NEPTUNE_PCIE_CLK_DEVICE_ID) {
        printf("Devicd ID 0x%x for chip 9DBV0841 on Neptune and Curie\n", read_id);
    } else if (read_id == GB_PCIE_CLK_DEVICE_ID) {
        printf("Devicd ID 0x%x for chip 9DBU0231 on Goldbeach\n", read_id);
    } else {
        printf("Devicd ID 0x%x of PCIe CLK is incorrect!!\n", read_id);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : write_reg
 * Description: write PCIe CLK Regs utility.
 * Inputs     : none
 * Outputs    : PASSED/FAILED.
 *******************************************************************************
 */
static int write_reg (void)
{
    int result = FAILED;
    char val;
    int offset;

    /* initial i2c for i2c_read/write*/

    offset = gethex_answer("Enter reg offset to be written", 0x0, 0, 0x7);
    val = gethex_answer("Enter reg value", 0x0, 0, 0xFF);
    result = write_pcie_clk_reg(offset, val);
    /* Write to the selected register */
    if (result != PASSED) {
        cterr('f', 0, "%s:FAILED to write data to PCIe clock (result = %#x).",
                      __FUNCTION__, result);
        return (FAILED);
    }

    return (dump_reg(0));
}


/****************************************************************************
 * Function   : write_pcie_clk_reg
 *
 * Description: write data to pcie clock register 
 *
 * Inputs     : offset :
 *
 * Outputs    : buf: contains register values
 *              return PASSED/FAILED.
 ****************************************************************************
 */
int 
write_pcie_clk_reg (int offset, unsigned char val)
{
    int result = FAILED;
    n2g_i2c_if_t i2c_if;
    char data[2]; 

    if (is_ntpn_machines() || is_vg450() || is_curie_1ru() || is_curie_2ru()) {
        result = get_pcie_clk_buf_i2c_struct(&i2c_if);
        if (result != PASSED) {
            printf("%s: Failed to get Power Sequencer I2C structure.\n",
               __FUNCTION__);
            return (result);
        }

        /* Setup I2C API parameter struct */
        i2c_if.offset = offset;
        i2c_if.size = 2;
        data[0] = 0;  /* dummy byte specific for chip */
        data[1] = val; 

        i2c_if.buf = data; 

        result = n2g_i2c_write(&i2c_if);
        if (result != RC_I2C_OP_OK) {
            printf("unable to write i2c.\n");
            return (FAILED);
        } else {
            return (PASSED);
        }
    } else { 
        /* initial i2c for write*/
        init_pcie_clk_i2c(&i2c_if);
        i2c_if.offset = offset;
        i2c_if.buf = (char *)&val;

        /* Write to the selected register */
        result = i2c_dev_wr((void *)&i2c_if, 1);
        if (result != PASSED) {
            cterr('f', 0, "%s:%d FAILED to write to PCIe clock (result = %#x).",
                           __FUNCTION__, __LINE__, result);
            return (FAILED);
        }
    } 
    return result;    
}

/******** History ********
*----------------------------------------------------
$Log: platform_pcie_clk.c,v $
Revision 1.12  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.11  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.10.2.1  2018/10/26 09:05:54  alpeng
fixed bug for pcie clk

Revision 1.10  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.9  2017/08/14 06:28:30  iachang
Add GB PCIe CLK Device ID

Revision 1.8  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.7.32.6  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.7.32.5  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.7.32.4  2017/03/13 07:49:18  leschen
Support Triton system.

Revision 1.7.32.3  2016/10/10 17:02:41  alpeng
support NIM module for neptune

Revision 1.7.32.2  2016/10/06 20:23:45  leschen
Modify for Neptune PCIe clk buf.

Revision 1.7.32.1  2016/08/30 07:15:40  leschen
Support Neptune PCIe clk utility.

Revision 1.7  2014/08/25 23:15:11  mcharon
disable/eanble pci clock during init

Revision 1.6  2014/08/21 23:37:25  mcharon
change reg_tmp from char to unsigned char so the display shows 0xff instead of 0xffffffff

Revision 1.5  2014/08/21 07:06:54  danchung
add PCIe CLK register write utility

Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/06/28 10:49:36  alpeng
support pcie clock util

Revision 1.2  2013/06/19 13:19:14  danchung
Add Device ID checking in PCIe CLK utilities.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.5  2012/09/19 22:30:50  palin2
Add I2C scan test support those I2C devices that are connected to Cavecreek.

Revision 1.4  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

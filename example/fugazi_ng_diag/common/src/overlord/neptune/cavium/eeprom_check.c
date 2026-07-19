/* $Id: eeprom_check.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/eeprom_check.c,v $
 *------------------------------------------------------------------
 *
 * eeprom_check.c : Check the configuration settings of EEPROM from Cavium
 *
 *
 * Copyright (c) 2007-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "cvmx.h"
#include "queryflags.h"

/*
 *  define
 */
#define PEM0_CFG_RD_ADDR              0x80011800c0000030ull
#define PEM0_CFG_RD_DATA_FLD_SHFT     32
#define PCIEEP_CFG_OFST_MAX           0x814 /* Max config reg offset
					       address */
#define PCIEEP0_CFG004_ADDR    0x10
#define PCIEEP0_CFG005_ADDR    0x14
#define PCIEEP0_CFG006_ADDR    0x18
#define PCIEEP0_CFG007_ADDR    0x1c
#define PCIEEP0_CFG008_ADDR    0x20
#define PCIEEP0_CFG009_ADDR    0x24
#define PCIEEP0_CFG031_ADDR    0x7c

#define PCIE_BAR0_LOW           PCIEEP0_CFG004_ADDR
#define PCIE_BAR0_HIGH          PCIEEP0_CFG005_ADDR
#define PCIE_BAR1_LOW           PCIEEP0_CFG006_ADDR
#define PCIE_BAR1_HIGH          PCIEEP0_CFG007_ADDR
#define PCIE_BAR2_LOW           PCIEEP0_CFG008_ADDR
#define PCIE_BAR2_HIGH          PCIEEP0_CFG009_ADDR
#define PCIE_LINK_CAP_REG       PCIEEP0_CFG031_ADDR

#define EXPECTED_LINK_CAP_VAL   0x00036c42
#define EXPECTED_BAR0_LOW       0x0400000c
#define EXPECTED_BAR0_HIGH      0x00000005
#define EXPECTED_BAR1_LOW       0x0000000c
#define EXPECTED_BAR1_HIGH      0x00000005
#define BAR2_LOW_MASK           0xfffffff0

/**********************************************************************
 *
 * Function: check_cavium_eeprom_loaded
 *
 * Description: Check a few key registers to see if the Cavium PCIe
 * configuration is loaded from the EEPROM.
 *
 * Input: None
 *
 * Return: pass/fail
 */

int 
check_cavium_eeprom_loaded (void)
{
    uint64_t reg_val64;
    uint32_t rd_data, bar2;
    int result = PASS;

    /* Check BAR0 low (CFG004) and high (CFG005)
     */
    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR0_LOW);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (rd_data != EXPECTED_BAR0_LOW) {
        printf("BAR0 low reg (CFG004) = %#.8x, expected= %#.8x\n",
	       rd_data, EXPECTED_BAR0_LOW);
	result = FAIL;
    }

    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR0_HIGH);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (rd_data != EXPECTED_BAR0_HIGH) {
        printf("BAR0 high reg (CFG005) = %#.8x, expected= %#.8x\n",
	       rd_data, EXPECTED_BAR0_HIGH);
	result = FAIL;
    }

    /* Check BAR1 low (CFG006) and high (CFG007)
     */
    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR1_LOW);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (rd_data != EXPECTED_BAR1_LOW) {
        printf("BAR1 low reg (CFG006) = %#.8x, expected= %#.8x\n",
	       rd_data, EXPECTED_BAR1_LOW);
	result = FAIL;
    }

    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR1_HIGH);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (rd_data != EXPECTED_BAR1_HIGH) {
        printf("BAR1 high reg (CFG007) = %#.8x, expected= %#.8x\n",
	       rd_data, EXPECTED_BAR1_HIGH);
	result = FAIL;
    }

    /* Check BAR2 low (CFG008) and high (CFG009)
     * Software team is considering to widen the BAR2 range.
     * Just to check it is enable and non-zero instead of checking
     * the actual value.
     */
    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR2_LOW);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;

    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_BAR2_HIGH);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    bar2 = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (((rd_data & BAR2_LOW_MASK) == 0) && (bar2 == 0)) {
        printf("BAR2 low and high reg (CFG008 & CFG009) address fields are zero\n");
	result = FAIL;
    }

    /* Check the Link capability reg (CFG031) (EEPROM word 14 and 15)
     */
    cvmx_write_csr(PEM0_CFG_RD_ADDR, PCIE_LINK_CAP_REG);
    reg_val64 = cvmx_read_csr(PEM0_CFG_RD_ADDR);
    rd_data = reg_val64 >> PEM0_CFG_RD_DATA_FLD_SHFT;
    if (rd_data != EXPECTED_LINK_CAP_VAL) {
        printf("Link capability reg (CFG031) = %#.8x, expected= %#.8x\n",
	       rd_data, EXPECTED_LINK_CAP_VAL);
	result = FAIL;
    }
    return(result);
}

/**********************************************************************
 *
 * Function: pcie_cfg_reg_dump_util
 *
 * Description: Utility to dump PCIE config register
 *
 * Input: None
 *
 * Return: pass/fail
 */
int 
pcie_cfg_reg_dump_util(void)
{
    uint64_t reg_val64[1];
    uint32_t *regP;
    uint32_t ofst_start = 0, ofst_end, ix;

    do {
        printf("Please enter offset in modulo of 4 (0, 4, 8, c, etc)\n");
	ofst_start = gethex_answer("Enter PCIe config reg offset start", ofst_start, 0, PCIEEP_CFG_OFST_MAX);
	ofst_end = gethex_answer("Enter offset end", ofst_start, ofst_start, PCIEEP_CFG_OFST_MAX);
    } while ((ofst_start % 4) || (ofst_end % 4));

    for (ix=ofst_start; ix <= ofst_end; ix+=4) {
        cvmx_write_csr(PEM0_CFG_RD_ADDR, ix);
	reg_val64[0]=cvmx_read_csr(PEM0_CFG_RD_ADDR);
	regP=(uint32_t *)reg_val64;
	printf("0x%04x= 0x%08x\n", ix, *regP);
    }
    return PASSED;
}

/* end of file */

/*
 *------------------------------------------------------------------
 * $Log: eeprom_check.c,v $
 * Revision 1.2  2018/05/18 09:24:56  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/06 05:58:50  xiaoyizh
 * Initial Check-in for Neptune Data Plane diags.
 *
 * Revision 1.3  2012/11/01 19:17:50  ptong
 * Support checking Cavium PCIe BAR 0-2 setting loaded from EEPROM
 *
 * Revision 1.2  2012/10/03 11:50:31  danchung
 * Change the representation of the setting value to hexadecimal.
 *
 * Revision 1.1  2012/09/21 09:50:51  danchung
 * Check the contect of cavium eeprom.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


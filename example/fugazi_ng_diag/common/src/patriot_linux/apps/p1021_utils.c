/* $Id: p1021_utils.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: p1021_utils.c
 *
 * Description: Ported from mpc8500_utils.c
 *
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "patriot_main.h"
#include "p1021_immap.h"
#include "p1021_espi.h"



void display_por_registers(void);
void display_laccs_registers(void);
void display_law_registers(void);
void display_ecm_registers(void);
void display_ddr1_registers(void);
void display_ddr2_registers(void);
void display_lbus_registers(void);
void display_pcie_registers(int);
void display_i2c1_registers(void);
void display_i2c2_registers(void);
void display_gpio_registers(void);
void display_pic_registers(void);
void display_l2cache_registers(void);
void display_msi_registers(void);
void display_espi_registers(void);
void display_qe_iram_registers(void);
void display_qe_irq_registers(void);
void display_qe_cp_registers(void);
void display_qe_mux_registers(void);
void display_qe_timer_registers(void);
void display_qe_spi1_registers(void);
void display_qe_brg_registers(void);
void display_qe_si_registers(void);
void display_qe_sirt_registers(void);
void display_qe_ucc1_registers(void);
void display_qe_ucc3_registers(void);
void display_qe_ucc5_registers(void);
void display_qe_ucc7_registers(void);
void display_qe_utopia_registers(void);
void display_qe_sdma_registers(void);
void display_qe_muram_registers(void);


void
display_por_registers (void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "POR PLL ratio status                     ",
					REGB->im_gur.porpllsr, 
				        &REGB->im_gur.porpllsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "POR boot mode status                     ",
					REGB->im_gur.porbmsr, 
				        &REGB->im_gur.porbmsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "POR I/O impedance status and control     ",
					REGB->im_gur.porimpscr, 
				        &REGB->im_gur.porimpscr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "POR I/O device status register           ",
					REGB->im_gur.pordevsr, 
				        &REGB->im_gur.pordevsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "POR debug mode status register           ",
					REGB->im_gur.pordbgmsr, 
				        &REGB->im_gur.pordbgmsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "General purpose POR configuration        ",
					REGB->im_gur.gpporcr, 
				        &REGB->im_gur.gpporcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "alternate function signal mux control    ",
					REGB->im_gur.pmuxcr, 
				        &REGB->im_gur.pmuxcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "device disable control                   ",
					REGB->im_gur.devdisr, 
				        &REGB->im_gur.devdisr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "machine check summary register           ",
					REGB->im_gur.mcpsumr, 
				        &REGB->im_gur.mcpsumr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Reset request status and control register",
					REGB->im_gur.rstrscr, 
				        &REGB->im_gur.rstrscr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Exception reset control register         ",
					REGB->im_gur.ectrstcr, 
				        &REGB->im_gur.ectrstcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Automatic reset status register          ",
					REGB->im_gur.autorstsr, 
				        &REGB->im_gur.autorstsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Processor Version register               ",
					REGB->im_gur.pvr, 
				        &REGB->im_gur.pvr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "System version register                  ",
					REGB->im_gur.svr, 
				        &REGB->im_gur.svr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Reset control register                   ",
					REGB->im_gur.rstcr, 
				        &REGB->im_gur.rstcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC voltage select control register      ",
					REGB->im_gur.lbcvselcr, 
				        &REGB->im_gur.lbcvselcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR clock disable register               ",
					REGB->im_gur.ddrclkdr, 
				        &REGB->im_gur.ddrclkdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "clock out control register               ",
					REGB->im_gur.clkocr, 
				        &REGB->im_gur.clkocr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "TLU target control register              ",
					REGB->im_gur.tlutrgcr, 
				        &REGB->im_gur.tlutrgcr);

}

void display_laccs_registers(void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "CCSRBAR                                  ",
					REGB->im_local_ecm.ccsrbar, 
				        &REGB->im_local_ecm.ccsrbar);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Alternate Configuration Base             ",
					REGB->im_local_ecm.altcbar, 
				        &REGB->im_local_ecm.altcbar);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Alternate Configuration Attribute        ",
					REGB->im_local_ecm.altcar, 
				        &REGB->im_local_ecm.altcar);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Boot Page Translation                    ",
					REGB->im_local_ecm.bptr, 
				        &REGB->im_local_ecm.bptr);
}

void display_law_registers(void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 0 Base               ",
					REGB->im_local_ecm.lawbar0, 
				        &REGB->im_local_ecm.lawbar0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 0 Attr               ",
					REGB->im_local_ecm.lawar0, 
				        &REGB->im_local_ecm.lawar0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 1 Base               ",
					REGB->im_local_ecm.lawbar1, 
				        &REGB->im_local_ecm.lawbar1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 1 Attr               ",
					REGB->im_local_ecm.lawar1, 
				        &REGB->im_local_ecm.lawar1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 2 Base               ",
					REGB->im_local_ecm.lawbar2, 
				        &REGB->im_local_ecm.lawbar2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 2 Attr               ",
					REGB->im_local_ecm.lawar2, 
				        &REGB->im_local_ecm.lawar2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 3 Base               ",
					REGB->im_local_ecm.lawbar3, 
				        &REGB->im_local_ecm.lawbar3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 3 Attr               ",
					REGB->im_local_ecm.lawar3, 
				        &REGB->im_local_ecm.lawar3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 4 Base               ",
					REGB->im_local_ecm.lawbar4, 
				        &REGB->im_local_ecm.lawbar4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 4 Attr               ",
					REGB->im_local_ecm.lawar4, 
				        &REGB->im_local_ecm.lawar4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 5 Base               ",
					REGB->im_local_ecm.lawbar5, 
				        &REGB->im_local_ecm.lawbar5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 5 Attr               ",
					REGB->im_local_ecm.lawar5, 
				        &REGB->im_local_ecm.lawar5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 6 Base               ",
					REGB->im_local_ecm.lawbar6, 
				        &REGB->im_local_ecm.lawbar6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 6 Attr               ",
					REGB->im_local_ecm.lawar6, 
				        &REGB->im_local_ecm.lawar6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 7 Base               ",
					REGB->im_local_ecm.lawbar7, 
				        &REGB->im_local_ecm.lawbar7);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 7 Attr               ",
					REGB->im_local_ecm.lawar7, 
				        &REGB->im_local_ecm.lawar7);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 8 Base               ",
					REGB->im_local_ecm.lawbar8, 
				        &REGB->im_local_ecm.lawbar8);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 8 Attr               ",
					REGB->im_local_ecm.lawar8, 
				        &REGB->im_local_ecm.lawar8);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 9 Base               ",
					REGB->im_local_ecm.lawbar9, 
				        &REGB->im_local_ecm.lawbar9);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Local Access Window 9 Attr               ",
					REGB->im_local_ecm.lawar9, 
				        &REGB->im_local_ecm.lawar9);
}

void display_ecm_registers(void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM CCB Address Configuration Register   ",
					REGB->im_local_ecm.eebacr, 
				        &REGB->im_local_ecm.eebacr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM CCB Port Configuration Register      ",
					REGB->im_local_ecm.eebpcr, 
				        &REGB->im_local_ecm.eebpcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM Error Detect Register                ",
					REGB->im_local_ecm.eedr, 
				        &REGB->im_local_ecm.eedr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM Error Enable Register                ",
					REGB->im_local_ecm.eeer, 
				        &REGB->im_local_ecm.eeer);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM Error Attributes Capture Register    ",
					REGB->im_local_ecm.eeatr, 
				        &REGB->im_local_ecm.eeatr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM Error Address Capture Register       ",
					REGB->im_local_ecm.eeadr, 
				        &REGB->im_local_ecm.eeadr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "ECM Error High Address Capture Register  ",
					REGB->im_local_ecm.eehadr, 
				        &REGB->im_local_ecm.eehadr);
}

void display_ddr1_registers(void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Chip Select 0 Memory Bounds         ",
					REGB->im_ddr1.cs0_bnds, 
				        &REGB->im_ddr1.cs0_bnds);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Chip Select 1 Memory Bounds         ",
					REGB->im_ddr1.cs1_bnds, 
				        &REGB->im_ddr1.cs1_bnds);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Chip Select 2 Memory Bounds         ",
					REGB->im_ddr1.cs2_bnds, 
				        &REGB->im_ddr1.cs2_bnds);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Chip Select 0 Configuration         ",
					REGB->im_ddr1.cs0_config, 
				        &REGB->im_ddr1.cs0_config);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Chip Select 1 Configuration         ",
					REGB->im_ddr1.cs1_config, 
				        &REGB->im_ddr1.cs1_config);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 SDRAM Timing Configuration 1        ",
					REGB->im_ddr1.timing_cfg_1, 
				        &REGB->im_ddr1.timing_cfg_1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 SDRAM Timing Configuration 2        ",
					REGB->im_ddr1.timing_cfg_2, 
				        &REGB->im_ddr1.timing_cfg_2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 SDRAM Control Configuration         ",
					REGB->im_ddr1.sdram_cfg, 
				        &REGB->im_ddr1.sdram_cfg);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 SDRAM Mode Configuration            ",
					REGB->im_ddr1.sdram_mode, 
				        &REGB->im_ddr1.sdram_mode);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 SDRAM Interval Configuration        ",
					REGB->im_ddr1.sdram_interval, 
				        &REGB->im_ddr1.sdram_interval);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Err Inject Mask HIGH                ",
					REGB->im_ddr1.data_err_inject_hi, 
				        &REGB->im_ddr1.data_err_inject_hi);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Err Inject Mask LOW                 ",
					REGB->im_ddr1.data_err_inject_lo, 
				        &REGB->im_ddr1.data_err_inject_lo);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Err Inject Mask ECC                 ",
					REGB->im_ddr1.ecc_err_inject, 
				        &REGB->im_ddr1.ecc_err_inject);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Read Capture HIGH                   ",
					REGB->im_ddr1.capture_data_hi, 
				        &REGB->im_ddr1.capture_data_hi);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Read Capture LOW                    ",
					REGB->im_ddr1.capture_data_lo, 
				        &REGB->im_ddr1.capture_data_lo);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Capture ECC                         ",
					REGB->im_ddr1.capture_ecc, 
				        &REGB->im_ddr1.capture_ecc);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Detect                 ",
					REGB->im_ddr1.err_detect, 
				        &REGB->im_ddr1.err_detect);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Disable                ",
					REGB->im_ddr1.err_disable, 
				        &REGB->im_ddr1.err_disable);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Interrupt enable       ",
					REGB->im_ddr1.err_int_en, 
				        &REGB->im_ddr1.err_int_en);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Attributes Capture     ",
					REGB->im_ddr1.capture_attributes, 
				        &REGB->im_ddr1.capture_attributes);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Address Capture        ",
					REGB->im_ddr1.capture_address, 
				        &REGB->im_ddr1.capture_address);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Error Ext Address Capture    ",
					REGB->im_ddr1.capture_ext_address, 
				        &REGB->im_ddr1.capture_ext_address);
    printf(" %20s : 0x%08x @ 0x%08x\n", "DDR1 Memory Single-Bit ECC Error Mgmt    ",
					REGB->im_ddr1.err_sbe, 
				        &REGB->im_ddr1.err_sbe);

}


void display_pcie_registers(int pcie_port)
{
    int   port_index=MAX_PCIE_PORT_NUM - pcie_port;

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "Config Address                                 ",
		      REGB->im_pcie[pcie_port].pex_config_addr, 
		      &REGB->im_pcie[pcie_port].pex_config_addr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "Config Data                                    ",
		      REGB->im_pcie[pcie_port].pex_config_data, 
		      &REGB->im_pcie[pcie_port].pex_config_data);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound completion timeout                    ",
		      REGB->im_pcie[pcie_port].pex_otb_cpl_tor, 
		      &REGB->im_pcie[pcie_port].pex_otb_cpl_tor);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "configuration retry timeout                    ",
		      REGB->im_pcie[pcie_port].pex_conf_rty_tor, 
		      &REGB->im_pcie[pcie_port].pex_conf_rty_tor);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "configuration                                  ",
		      REGB->im_pcie[pcie_port].pex_config, 
		      &REGB->im_pcie[pcie_port].pex_config);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "PME & message detect                           ",
		      REGB->im_pcie[pcie_port].pex_pme_mes_dr, 
		      &REGB->im_pcie[pcie_port].pex_pme_mes_dr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "PME & message disable                          ",
		      REGB->im_pcie[pcie_port].pex_pme_mes_disr, 
		      &REGB->im_pcie[pcie_port].pex_pme_mes_disr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "PME & message interrupt enable                 ",
		      REGB->im_pcie[pcie_port].pex_pme_mes_ier, 
		      &REGB->im_pcie[pcie_port].pex_pme_mes_ier);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "power management command                       ",
		      REGB->im_pcie[pcie_port].pex_pmcr, 
		      &REGB->im_pcie[pcie_port].pex_pmcr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation address 0                 ",
		      REGB->im_pcie[pcie_port].pexotar0, 
		      &REGB->im_pcie[pcie_port].pexotar0);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation extended address 0        ",
		      REGB->im_pcie[pcie_port].pexotear0, 
		      &REGB->im_pcie[pcie_port].pexotear0);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window attributes 0                   ",
		      REGB->im_pcie[pcie_port].pexowar0, 
		      &REGB->im_pcie[pcie_port].pexowar0);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation address 1                 ",
		      REGB->im_pcie[pcie_port].pexotar1, 
		      &REGB->im_pcie[pcie_port].pexotar1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation extended address 1        ",
		      REGB->im_pcie[pcie_port].pexotear1, 
		      &REGB->im_pcie[pcie_port].pexotear1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window base address 1                 ",
		      REGB->im_pcie[pcie_port].pexowbar1, 
		      &REGB->im_pcie[pcie_port].pexowbar1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window attributes 1                   ",
		      REGB->im_pcie[pcie_port].pexowar1, 
		      &REGB->im_pcie[pcie_port].pexowar1);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation address 2                 ",
		      REGB->im_pcie[pcie_port].pexotar2, 
		      &REGB->im_pcie[pcie_port].pexotar2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation extended address 2        ",
		      REGB->im_pcie[pcie_port].pexotear2, 
		      &REGB->im_pcie[pcie_port].pexotear2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window base address 2                 ",
		      REGB->im_pcie[pcie_port].pexowbar2, 
		      &REGB->im_pcie[pcie_port].pexowbar2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window attributes 2                   ",
		      REGB->im_pcie[pcie_port].pexowar2, 
		      &REGB->im_pcie[pcie_port].pexowar2);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation address 3                 ",
		      REGB->im_pcie[pcie_port].pexotar3, 
		      &REGB->im_pcie[pcie_port].pexotar3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation extended address 3        ",
		      REGB->im_pcie[pcie_port].pexotear3, 
		      &REGB->im_pcie[pcie_port].pexotear3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window base address 3                 ",
		      REGB->im_pcie[pcie_port].pexowbar3, 
		      &REGB->im_pcie[pcie_port].pexowbar3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window attributes 3                   ",
		      REGB->im_pcie[pcie_port].pexowar3, 
		      &REGB->im_pcie[pcie_port].pexowar3);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation address 4                 ",
		      REGB->im_pcie[pcie_port].pexotar4, 
		      &REGB->im_pcie[pcie_port].pexotar4);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound translation extended address 4        ",
		      REGB->im_pcie[pcie_port].pexotear4, 
		      &REGB->im_pcie[pcie_port].pexotear4);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window base address 4                 ",
		      REGB->im_pcie[pcie_port].pexowbar4, 
		      &REGB->im_pcie[pcie_port].pexowbar4);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "outbound window attributes 4                   ",
		      REGB->im_pcie[pcie_port].pexowar4, 
		      &REGB->im_pcie[pcie_port].pexowar4);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound translation address 1                  ",
		      REGB->im_pcie[pcie_port].pexitar1, 
		      &REGB->im_pcie[pcie_port].pexitar1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window base address 1                  ",
		      REGB->im_pcie[pcie_port].pexiwbar1, 
		      &REGB->im_pcie[pcie_port].pexiwbar1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window attributes 1                    ",
		      REGB->im_pcie[pcie_port].pexiwar1, 
		      &REGB->im_pcie[pcie_port].pexiwar1);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound translation address 2                  ",
		      REGB->im_pcie[pcie_port].pexitar2, 
		      &REGB->im_pcie[pcie_port].pexitar2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window base address 2                  ",
		      REGB->im_pcie[pcie_port].pexiwbar2, 
		      &REGB->im_pcie[pcie_port].pexiwbar2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window base extended address 2         ",
		      REGB->im_pcie[pcie_port].pexiwbear2, 
		      &REGB->im_pcie[pcie_port].pexiwbear2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window attributes 2                    ",
		      REGB->im_pcie[pcie_port].pexiwar2, 
		      &REGB->im_pcie[pcie_port].pexiwar2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound translation address 3                  ",
		      REGB->im_pcie[pcie_port].pexitar3, 
		      &REGB->im_pcie[pcie_port].pexitar3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window base address 3                  ",
		      REGB->im_pcie[pcie_port].pexiwbar3, 
		      &REGB->im_pcie[pcie_port].pexiwbar3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window base extended address 3         ",
		      REGB->im_pcie[pcie_port].pexiwbear3, 
		      &REGB->im_pcie[pcie_port].pexiwbear3);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "inbound window attributes 3                    ",
		      REGB->im_pcie[pcie_port].pexiwar3, 
		      &REGB->im_pcie[pcie_port].pexiwar3);

    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error detect                                   ",
		      REGB->im_pcie[pcie_port].pex_err_dr, 
		      &REGB->im_pcie[pcie_port].pex_err_dr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error interrupt enable                         ",
		      REGB->im_pcie[pcie_port].pex_err_en, 
		      &REGB->im_pcie[pcie_port].pex_err_en);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error disable                                  ",
		      REGB->im_pcie[pcie_port].pex_err_disr, 
		      &REGB->im_pcie[pcie_port].pex_err_disr);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "capture status                                 ",
		      REGB->im_pcie[pcie_port].pex_err_cap_stat, 
		      &REGB->im_pcie[pcie_port].pex_err_cap_stat);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error capture register 0                       ",
		      REGB->im_pcie[pcie_port].pex_err_cap_r0, 
		      &REGB->im_pcie[pcie_port].pex_err_cap_r0);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error capture register 1                       ",
		      REGB->im_pcie[pcie_port].pex_err_cap_r1, 
		      &REGB->im_pcie[pcie_port].pex_err_cap_r1);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error capture register 2                       ",
		      REGB->im_pcie[pcie_port].pex_err_cap_r2, 
		      &REGB->im_pcie[pcie_port].pex_err_cap_r2);
    printf(" PCIe %d %20s : 0x%08x @ 0x%08x\n", port_index, 
		     "error capture register 3                       ",
		      REGB->im_pcie[pcie_port].pex_err_cap_r3, 
		      &REGB->im_pcie[pcie_port].pex_err_cap_r3);

}

void
display_i2c1_registers (void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Address Register                ",
					REGB->im_i2c1.i2cadr, 
				       &REGB->im_i2c1.i2cadr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Frequency Divider Register      ",
					REGB->im_i2c1.i2cfdr, 
				       &REGB->im_i2c1.i2cfdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Control Register                ",
					REGB->im_i2c1.i2ccr, 
				       &REGB->im_i2c1.i2ccr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Status Register                 ",
					REGB->im_i2c1.i2csr, 
				       &REGB->im_i2c1.i2csr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Data Register                   ",
					REGB->im_i2c1.i2cdr, 
				       &REGB->im_i2c1.i2cdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 1 Digital Filtering Sampling Rate ",
					REGB->im_i2c1.i2cdfsrr, 
				       &REGB->im_i2c1.i2cdfsrr);

}

void
display_i2c2_registers (void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Address Register                ",
					REGB->im_i2c2.i2cadr, 
				       &REGB->im_i2c2.i2cadr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Frequency Divider Register      ",
					REGB->im_i2c2.i2cfdr, 
				       &REGB->im_i2c2.i2cfdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Control Register                ",
					REGB->im_i2c2.i2ccr, 
				       &REGB->im_i2c2.i2ccr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Status Register                 ",
					REGB->im_i2c2.i2csr, 
				       &REGB->im_i2c2.i2csr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Data Register                   ",
					REGB->im_i2c2.i2cdr, 
				       &REGB->im_i2c2.i2cdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "I2C 2 Digital Filtering Sampling Rate ",
					REGB->im_i2c2.i2cdfsrr, 
				       &REGB->im_i2c2.i2cdfsrr);

}


void display_lbus_registers (void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 0                   ",
					REGB->im_lbc.br0, 
				       &REGB->im_lbc.br0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 0                ",
					REGB->im_lbc.or0, 
				       &REGB->im_lbc.or0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 1                   ",
					REGB->im_lbc.br1, 
				       &REGB->im_lbc.br1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 1                ",
					REGB->im_lbc.or1, 
				       &REGB->im_lbc.or1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 2                   ",
					REGB->im_lbc.br2, 
				       &REGB->im_lbc.br2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 2                ",
					REGB->im_lbc.or2, 
				       &REGB->im_lbc.or2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 3                   ",
					REGB->im_lbc.br3, 
				       &REGB->im_lbc.br3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 3                ",
					REGB->im_lbc.or3, 
				       &REGB->im_lbc.or3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 4                   ",
					REGB->im_lbc.br4, 
				       &REGB->im_lbc.br4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 4                ",
					REGB->im_lbc.or4, 
				       &REGB->im_lbc.or4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 5                   ",
					REGB->im_lbc.br5, 
				       &REGB->im_lbc.br5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 5                ",
					REGB->im_lbc.or5, 
				       &REGB->im_lbc.or5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 6                   ",
					REGB->im_lbc.br6, 
				       &REGB->im_lbc.br6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 6                ",
					REGB->im_lbc.or6, 
				       &REGB->im_lbc.or6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Base Register 7                   ",
					REGB->im_lbc.br7, 
				       &REGB->im_lbc.br7);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Options Register 7                ",
					REGB->im_lbc.or7, 
				       &REGB->im_lbc.or7);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPM Address Register              ",
					REGB->im_lbc.mar, 
				       &REGB->im_lbc.mar);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPMA Mode Register                ",
					REGB->im_lbc.mamr, 
				       &REGB->im_lbc.mamr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPMB Mode Register                ",
					REGB->im_lbc.mbmr, 
				       &REGB->im_lbc.mbmr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPMC Mode Register                ",
					REGB->im_lbc.mcmr, 
				       &REGB->im_lbc.mcmr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Memory Refresh Timer Prescaler    ",
					REGB->im_lbc.mrtpr, 
				       &REGB->im_lbc.mrtpr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPM Data Register                 ",
					REGB->im_lbc.mdr, 
				       &REGB->im_lbc.mdr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC UPM Refresh Timer                 ",
					REGB->im_lbc.lurt, 
				       &REGB->im_lbc.lurt);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Transfer Error Status Register    ",
					REGB->im_lbc.ltesr, 
				       &REGB->im_lbc.ltesr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Transfer Error Disable Register   ",
					REGB->im_lbc.ltedr, 
				       &REGB->im_lbc.ltedr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Transfer Error Interrupt Register ",
					REGB->im_lbc.lteir, 
				       &REGB->im_lbc.lteir);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Transfer Error Attribute Register ",
					REGB->im_lbc.lteatr, 
				       &REGB->im_lbc.lteatr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Transfer Error Address Register   ",
					REGB->im_lbc.ltear, 
				       &REGB->im_lbc.ltear);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Configuration Register            ",
					REGB->im_lbc.lbcr, 
				       &REGB->im_lbc.lbcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "LBC Clock Ratio Register              ",
					REGB->im_lbc.lcrr, 
				       &REGB->im_lbc.lcrr);

}


void display_gpio_registers (void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO open drain register A              ",
					REGB->im_gur.cpodra, 
				       &REGB->im_gur.cpodra);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO data register A                    ",
					REGB->im_gur.cpddata, 
				       &REGB->im_gur.cpddata);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 1A              ",
					REGB->im_gur.cpdir1a, 
				       &REGB->im_gur.cpdir1a);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 2A              ",
					REGB->im_gur.cpdir2a, 
				       &REGB->im_gur.cpdir2a);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 1A         ",
					REGB->im_gur.cppar1a, 
				       &REGB->im_gur.cppar1a);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 2A         ",
					REGB->im_gur.cppar2a, 
				       &REGB->im_gur.cppar2a);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO open drain register B              ",
					REGB->im_gur.cpodrb, 
				       &REGB->im_gur.cpodrb);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO data register B                    ",
					REGB->im_gur.cpddatb, 
				       &REGB->im_gur.cpddatb);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 1B              ",
					REGB->im_gur.cpdir1b, 
				       &REGB->im_gur.cpdir1b);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 2B              ",
					REGB->im_gur.cpdir2b, 
				       &REGB->im_gur.cpdir2b);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 1B         ",
					REGB->im_gur.cppar1b, 
				       &REGB->im_gur.cppar1b);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 2B         ",
					REGB->im_gur.cppar2b, 
				       &REGB->im_gur.cppar2b);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO data register C                    ",
					REGB->im_gur.cpddatc, 
				       &REGB->im_gur.cpddatc);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 1C              ",
					REGB->im_gur.cpdir1c, 
				       &REGB->im_gur.cpdir1c);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO direction register 2C              ",
					REGB->im_gur.cpdir2c, 
				       &REGB->im_gur.cpdir2c);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 1C         ",
					REGB->im_gur.cppar1c, 
				       &REGB->im_gur.cppar1c);
    printf(" %20s : 0x%08x @ 0x%08x\n", "GPIO pin assignment register 2C         ",
					REGB->im_gur.cppar2c, 
				       &REGB->im_gur.cppar2c);    
}


void display_pic_registers (void)
{
    unsigned int i, *rptr;

    printf(" %20s : 0x%08x @ 0x%08x\n", "Current Task Priority Register           ",
					REGB->im_pic.ctpr, 
				       &REGB->im_pic.ctpr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Who Am I Register                        ",
					REGB->im_pic.whoami, 
				       &REGB->im_pic.whoami);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Interrupt Acknowledge Register           ",
					REGB->im_pic.iack, 
				       &REGB->im_pic.iack);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Feature Reporting Register               ",
					REGB->im_pic.frr, 
				       &REGB->im_pic.frr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Configurrrion Register            ",
					REGB->im_pic.gcr, 
				       &REGB->im_pic.gcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Vendor Identification Register           ",
					REGB->im_pic.vir, 
				       &REGB->im_pic.vir);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Processor Initialization Register        ",
					REGB->im_pic.pir, 
				       &REGB->im_pic.pir);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IPI Vector/Priority Register 0           ",
					REGB->im_pic.ipivpr0, 
				       &REGB->im_pic.ipivpr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IPI Vector/Priority Register 1           ",
					REGB->im_pic.ipivpr1, 
				       &REGB->im_pic.ipivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IPI Vector/Priority Register 2           ",
					REGB->im_pic.ipivpr2, 
				       &REGB->im_pic.ipivpr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IPI Vector/Priority Register 3           ",
					REGB->im_pic.ipivpr3, 
				       &REGB->im_pic.ipivpr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Spurious Vector Register                 ",
					REGB->im_pic.svr, 
				       &REGB->im_pic.svr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer Frequency Reporting Register       ",
					REGB->im_pic.tfrr, 
				       &REGB->im_pic.tfrr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Current Count Register 0    ",
					REGB->im_pic.gtccr0, 
				       &REGB->im_pic.gtccr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Base Count Register 0       ",
					REGB->im_pic.gtbcr0, 
				       &REGB->im_pic.gtbcr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Vector/Priority Register 0  ",
					REGB->im_pic.gtvpr0, 
				       &REGB->im_pic.gtvpr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Destination Register 0      ",
					REGB->im_pic.gtdr0, 
				       &REGB->im_pic.gtdr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Current Count Register 1    ",
					REGB->im_pic.gtccr1, 
				       &REGB->im_pic.gtccr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Base Count Register 1       ",
					REGB->im_pic.gtbcr1, 
				       &REGB->im_pic.gtbcr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Vector/Priority Register 1  ",
					REGB->im_pic.gtvpr1, 
				       &REGB->im_pic.gtvpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Destination Register 1      ",
					REGB->im_pic.gtdr1, 
				       &REGB->im_pic.gtdr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Current Count Register 2    ",
					REGB->im_pic.gtccr2, 
				       &REGB->im_pic.gtccr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Base Count Register 2       ",
					REGB->im_pic.gtbcr2, 
				       &REGB->im_pic.gtbcr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Vector/Priority Register 2  ",
					REGB->im_pic.gtvpr2, 
				       &REGB->im_pic.gtvpr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Destination Register 2      ",
					REGB->im_pic.gtdr2, 
				       &REGB->im_pic.gtdr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Current Count Register 3    ",
					REGB->im_pic.gtccr3, 
				       &REGB->im_pic.gtccr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Base Count Register 3       ",
					REGB->im_pic.gtbcr3, 
				       &REGB->im_pic.gtbcr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Vector/Priority Register 3  ",
					REGB->im_pic.gtvpr3, 
				       &REGB->im_pic.gtvpr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Global Timer Destination Register 3      ",
					REGB->im_pic.gtdr3, 
				       &REGB->im_pic.gtdr3);


    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer Control Register                   ",
					REGB->im_pic.tcr, 
				       &REGB->im_pic.tcr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Summary Register      ",
					REGB->im_pic.erqsr, 
				       &REGB->im_pic.erqsr);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IRQ_OUT Summary Register 0               ",
					REGB->im_pic.irqsr0, 
				       &REGB->im_pic.irqsr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "IRQ_OUT Summary Register 1               ",
					REGB->im_pic.irqsr1, 
				       &REGB->im_pic.irqsr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Critical Interrupt Summary Register 0    ",
					REGB->im_pic.cisr0, 
				       &REGB->im_pic.cisr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Critical Interrupt Summary Register 1    ",
					REGB->im_pic.cisr1, 
				       &REGB->im_pic.cisr1);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Register 0                       ",
					REGB->im_pic.msgr0, 
				       &REGB->im_pic.msgr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Register 1                       ",
					REGB->im_pic.msgr1, 
				       &REGB->im_pic.msgr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Register 2                       ",
					REGB->im_pic.msgr2, 
				       &REGB->im_pic.msgr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Register 3                       ",
					REGB->im_pic.msgr3, 
				       &REGB->im_pic.msgr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Enable Register                  ",
					REGB->im_pic.mer, 
				       &REGB->im_pic.mer);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Status Register                  ",
					REGB->im_pic.msr, 
				       &REGB->im_pic.msr);

    display_msi_registers();

    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 0  ",
					REGB->im_pic.eivpr0, 
				       &REGB->im_pic.eivpr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 0     ",
					REGB->im_pic.eidr0, 
				       &REGB->im_pic.eidr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 1  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 1     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 2  ",
					REGB->im_pic.eivpr2, 
				       &REGB->im_pic.eivpr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 2     ",
					REGB->im_pic.eidr2, 
				       &REGB->im_pic.eidr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 3  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 3     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 4  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 4     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 5  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 5     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 6  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 6     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 7  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 7     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 8  ",
					REGB->im_pic.eivpr1, 
				       &REGB->im_pic.eivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 8     ",
					REGB->im_pic.eidr1, 
				       &REGB->im_pic.eidr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 9  ",
					REGB->im_pic.eivpr9, 
				       &REGB->im_pic.eivpr9);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 9     ",
					REGB->im_pic.eidr9, 
				       &REGB->im_pic.eidr9);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 10 ",
					REGB->im_pic.eivpr10, 
				       &REGB->im_pic.eivpr10);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 10    ",
					REGB->im_pic.eidr10, 
				       &REGB->im_pic.eidr10);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Vector/Priorit Reg 11 ",
					REGB->im_pic.eivpr11, 
				       &REGB->im_pic.eivpr11);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External Interrupt Destination Reg 11    ",
					REGB->im_pic.eidr11, 
				       &REGB->im_pic.eidr11);

    rptr = &(REGB->im_pic.iivpr0);
    for (i = 0; i < P1021_PIC_INT_INTR_NO; i++) {
        printf(" %s %d %6s : 0x%08x @ 0x%08x\n", "Internal Interrupt Destination Reg ",
					i,
					"     ",
					*rptr, 
				        rptr);
        rptr += (&(REGB->im_pic.iivpr1) - &(REGB->im_pic.iivpr0));
    }
}


void display_l2cache_registers(void)
{
    printf(" %20s : 0x%08x @ 0x%08x\n", "L2 Control register                         ",
					REGB->im_l2cache.l2ctl, 
				       &REGB->im_l2cache.l2ctl);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write address register 0           ",
					REGB->im_l2cache.l2cewar0, 
				       &REGB->im_l2cache.l2cewar0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write control register 0           ",
					REGB->im_l2cache.l2cewcr0, 
				       &REGB->im_l2cache.l2cewcr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write address register 1           ",
					REGB->im_l2cache.l2cewar1, 
				       &REGB->im_l2cache.l2cewar1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write control register 1           ",
					REGB->im_l2cache.l2cewcr1, 
				       &REGB->im_l2cache.l2cewcr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write address register 2           ",
					REGB->im_l2cache.l2cewar2, 
				       &REGB->im_l2cache.l2cewar2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write control register 2           ",
					REGB->im_l2cache.l2cewcr2, 
				       &REGB->im_l2cache.l2cewcr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write address register 3           ",
					REGB->im_l2cache.l2cewar3, 
				       &REGB->im_l2cache.l2cewar3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "External write control register 3           ",
					REGB->im_l2cache.l2cewcr3, 
				       &REGB->im_l2cache.l2cewcr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Memory-mapped SRAM base address register 0  ",
					REGB->im_l2cache.l2srbar0, 
				       &REGB->im_l2cache.l2srbar0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Memory-mapped SRAM base address register 1  ",
					REGB->im_l2cache.l2srbar1, 
				       &REGB->im_l2cache.l2srbar1);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error injection mask high register          ",
					REGB->im_l2cache.l2errinjhi, 
				       &REGB->im_l2cache.l2errinjhi);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Erro injection mask low register            ",
					REGB->im_l2cache.l2errinjlo, 
				       &REGB->im_l2cache.l2errinjlo);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error injection mask control register       ",
					REGB->im_l2cache.l2errinjctl, 
				       &REGB->im_l2cache.l2errinjctl);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error data high capture register            ",
					REGB->im_l2cache.l2captdatahi, 
				       &REGB->im_l2cache.l2captdatahi);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error data low capture register             ",
					REGB->im_l2cache.l2captdatalo, 
				       &REGB->im_l2cache.l2captdatalo);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error syndrome register                     ",
					REGB->im_l2cache.l2captecc, 
				       &REGB->im_l2cache.l2captecc);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error detect register                       ",
					REGB->im_l2cache.l2errdet, 
				       &REGB->im_l2cache.l2errdet);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error disable register                      ",
					REGB->im_l2cache.l2errdis, 
				       &REGB->im_l2cache.l2errdis);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error interrupt enable register             ",
					REGB->im_l2cache.l2errinten, 
				       &REGB->im_l2cache.l2errinten);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error attributes capture register           ",
					REGB->im_l2cache.l2errattr, 
				       &REGB->im_l2cache.l2errattr);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error address capture high register         ",
					REGB->im_l2cache.l2erraddrhi, 
				       &REGB->im_l2cache.l2erraddrhi);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Error address capture low register          ",
					REGB->im_l2cache.l2erraddrlo, 
				       &REGB->im_l2cache.l2erraddrlo);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Error control register                      ",
					REGB->im_l2cache.l2errctl, 
				       &REGB->im_l2cache.l2errctl);
}

void
display_msi_registers (void)
{

    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Signaled Interrupt Status Register ",
                                        REGB->im_pic.msisr,
                                       &REGB->im_pic.msisr);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 0 Register      ",
                                        REGB->im_pic.msir0,
                                       &REGB->im_pic.msir0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 1 Register      ",
                                        REGB->im_pic.msir1,
                                       &REGB->im_pic.msir1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 2 Register      ",
                                        REGB->im_pic.msir2,
                                       &REGB->im_pic.msir2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 3 Register      ",
                                        REGB->im_pic.msir3,
                                       &REGB->im_pic.msir3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 4 Register      ",
                                        REGB->im_pic.msir4,
                                       &REGB->im_pic.msir4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 5 Register      ",
                                        REGB->im_pic.msir5,
                                       &REGB->im_pic.msir5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 6 Register      ",
                                        REGB->im_pic.msir6,
                                       &REGB->im_pic.msir6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt 7 Register      ",
                                        REGB->im_pic.msir7,
                                       &REGB->im_pic.msir7);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 0",
                                        REGB->im_pic.msivpr0,
                                       &REGB->im_pic.msivpr0);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 1",
                                        REGB->im_pic.msivpr1,
                                       &REGB->im_pic.msivpr1);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 2",
                                        REGB->im_pic.msivpr2,
                                       &REGB->im_pic.msivpr2);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 3",
                                        REGB->im_pic.msivpr3,
                                       &REGB->im_pic.msivpr3);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 4",
                                        REGB->im_pic.msivpr4,
                                       &REGB->im_pic.msivpr4);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 5",
                                        REGB->im_pic.msivpr5,
                                       &REGB->im_pic.msivpr5);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 6",
                                        REGB->im_pic.msivpr6,
                                       &REGB->im_pic.msivpr6);
    printf(" %20s : 0x%08x @ 0x%08x\n", "Message Shared Interrupt Vector/Priority Reg 7",
                                        REGB->im_pic.msivpr7,
                                       &REGB->im_pic.msivpr7);

}


/**************************************************************************
 * Function: display_espi_registers
 *
 * Description: This function display ESPI registers
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_espi_registers(void)
{

    volatile ccsr_espi_t *espi = &(REGB->im_espi);

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPMODE SPI mode                  ",
         espi->spmode, (uint32_t)&(espi->spmode) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPIE SPI event                   ",
         espi->spie, (uint32_t)&(espi->spie) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPIM SPI mask                    ",
         espi->spim, (uint32_t)&(espi->spim) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPCOM SPI command                ",
         espi->spcom, (uint32_t)&(espi->spcom) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPITF SPI transmit FIFO          ",
         espi->spitf, (uint32_t)&(espi->spitf) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPIRF SPI receive FIFO           ",
         espi->spirf, (uint32_t)&(espi->spirf) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPMODE0 SPI mode 0               ",
         espi->spmode0, (uint32_t)&(espi->spmode0) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPMODE1 SPI mode 1               ",
         espi->spmode1, (uint32_t)&(espi->spmode1) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPMODE2 SPI mode 2               ",
         espi->spmode2, (uint32_t)&(espi->spmode2) - (uint32_t)&(espi->spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPMODE3 SPI mode 3               ",
         espi->spmode3, (uint32_t)&(espi->spmode3) - (uint32_t)&(espi->spmode));

}


/**************************************************************************
 * Function: display_qe_iram_registers
 *
 * Description: This function display qe instructions ram registers
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_iram_registers(void)
{
    volatile ccsr_qe_t *qe_iram = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE I-RAM address                  ",
          qe_iram->iram.iadd,
          (uint32_t)&(qe_iram->iram.iadd) - (uint32_t)&(qe_iram->iram.iadd));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE I-RAM data                     ",
          qe_iram->iram.idata,
          (uint32_t)&(qe_iram->iram.idata) - (uint32_t)&(qe_iram->iram.iadd));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE I-RAM ready                    ",
          qe_iram->iram.iready,
          (uint32_t)&(qe_iram->iram.iready) - (uint32_t)&(qe_iram->iram.iadd));
}


/**************************************************************************
 * Function: display_qe_irq_registers
 *
 * Description: This function display qe interrupt controller
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_irq_registers(void)
{
    volatile ccsr_qe_t *qe_irq = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE system interrupt config        ",
           qe_irq->irq.cicr,
          (uint32_t)&(qe_irq->irq.cicr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE system interrupt vector        ",
          qe_irq->irq.civec,
          (uint32_t)&(qe_irq->irq.civec) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE RISC interrupt pending         ",
           qe_irq->irq.cripnr,
           (uint32_t)&(qe_irq->irq.cripnr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE systerm interrupt pending      ",
           qe_irq->irq.cipnr,
           (uint32_t)&(qe_irq->irq.cipnr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE interrupt priority (cipxcc)    ",
           qe_irq->irq.cipxcc,
           (uint32_t)&(qe_irq->irq.cipxcc) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE interrupt priority (cipycc)    ",
           qe_irq->irq.cipycc,
           (uint32_t)&(qe_irq->irq.cipycc) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE interrupt priority (cipwcc)    ",
           qe_irq->irq.cipwcc,
          (uint32_t)&(qe_irq->irq.cipwcc) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE interrupt priority (cipzcc)    ",
          qe_irq->irq.cipzcc,
          (uint32_t)&(qe_irq->irq.cipzcc) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE system interrupt mask          ",
           qe_irq->irq.cimr,
           (uint32_t)&(qe_irq->irq.cimr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE RISC interrupt mask            ",
           qe_irq->irq.crimr,
           (uint32_t)&(qe_irq->irq.crimr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE system intrt control           ",
           qe_irq->irq.cicnr,
           (uint32_t)&(qe_irq->irq.cicnr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE sys intr prio for RISC tasks A ",
           qe_irq->irq.ciprta,
           (uint32_t)&(qe_irq->irq.ciprta) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE sys intr prio for RISC tasks B ",
           qe_irq->irq.ciprtb,
           (uint32_t)&(qe_irq->irq.ciprtb) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE system RISC intr control       ",
           qe_irq->irq.cricr,
           (uint32_t)&(qe_irq->irq.cricr) - (uint32_t)&(qe_irq->irq.cicr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE high system interrupt vec      ",
           qe_irq->irq.chivec,
           (uint32_t)&(qe_irq->irq.chivec) - (uint32_t)&(qe_irq->irq.cicr));
}


/**************************************************************************
 * Function: display_qe_cp_registers
 *
 * Description: This function display qe communication processor
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_cp_registers(void)
{
    volatile ccsr_qe_t *qe_cp = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE command                       ",
           qe_cp->cp.cecr,
          (uint32_t)&(qe_cp->cp.cecr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE controller configuration      ",
           qe_cp->cp.ceccr,
          (uint32_t)&(qe_cp->cp.ceccr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE command data                  ",
           qe_cp->cp.cecdr,
          (uint32_t)&(qe_cp->cp.cecdr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE timer event                   ",
           qe_cp->cp.ceter,
          (uint32_t)&(qe_cp->cp.ceter) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE timers mask                   ",
           qe_cp->cp.cetmr,
          (uint32_t)&(qe_cp->cp.cetmr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE time-stamp timer control      ",
           qe_cp->cp.cetscr,
          (uint32_t)&(qe_cp->cp.cetscr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE time-stamp 1                  ",
           qe_cp->cp.cetsr1,
          (uint32_t)&(qe_cp->cp.cetsr1) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE time-stamp 2                  ",
           qe_cp->cp.cetsr2,
          (uint32_t)&(qe_cp->cp.cetsr2) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE virtual tasks event           ",
           qe_cp->cp.cevter,
          (uint32_t)&(qe_cp->cp.cevter) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE virtual tasks mask            ",
           qe_cp->cp.cevtmr,
          (uint32_t)&(qe_cp->cp.cevtmr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE RAM control                   ",
           qe_cp->cp.cercr,
          (uint32_t)&(qe_cp->cp.cercr) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 1 event      ",
           qe_cp->cp.ceexe1,
          (uint32_t)&(qe_cp->cp.ceexe1) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 1 mask       ",
           qe_cp->cp.ceexm1,
          (uint32_t)&(qe_cp->cp.ceexm1) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 2 event      ",
           qe_cp->cp.ceexe2,
          (uint32_t)&(qe_cp->cp.ceexe2) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 2 mask       ",
           qe_cp->cp.ceexm2,
          (uint32_t)&(qe_cp->cp.ceexm2) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 3 event      ",
           qe_cp->cp.ceexe3,
          (uint32_t)&(qe_cp->cp.ceexe3) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 3 mask       ",
           qe_cp->cp.ceexm3,
          (uint32_t)&(qe_cp->cp.ceexm3) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 4 event      ",
           qe_cp->cp.ceexe4,
          (uint32_t)&(qe_cp->cp.ceexe4) - (uint32_t)&(qe_cp->cp.cecr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "QE external request 4 mask       ",
           qe_cp->cp.ceexm4,
          (uint32_t)&(qe_cp->cp.ceexm4) - (uint32_t)&(qe_cp->cp.cecr));
}


/**************************************************************************
 * Function: display_qe_mux_registers
 *
 * Description: This function display qe multiplexer
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_mux_registers(void)
{
    volatile ccsr_qe_t *qe_mux = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX general clock route         ",
           qe_mux->mux.cmxgcr,
          (uint32_t)&(qe_mux->mux.cmxgcr) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX SI1 clock route low         ",
           qe_mux->mux.cmxsi1cl_l,
          (uint32_t)&(qe_mux->mux.cmxsi1cl_l) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX SI1 clock route high        ",
           qe_mux->mux.cmxsi1cr_h,
          (uint32_t)&(qe_mux->mux.cmxsi1cr_h) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX SI1 SYNC route              ",
           qe_mux->mux.cmxsi1syr,
          (uint32_t)&(qe_mux->mux.cmxsi1syr) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX UCC1, UCC3 clock route      ",
           qe_mux->mux.cmxucr1,
          (uint32_t)&(qe_mux->mux.cmxucr1) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX UCC5 clock route            ",
           qe_mux->mux.cmxucr2,
          (uint32_t)&(qe_mux->mux.cmxucr2) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX UCC2, UCC4 clock route      ",
           qe_mux->mux.cmxucr3,
          (uint32_t)&(qe_mux->mux.cmxucr3) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX UCC2, UCC4 clock route      ",
           qe_mux->mux.cmxucr4,
          (uint32_t)&(qe_mux->mux.cmxucr4) - (uint32_t)&(qe_mux->mux.cmxgcr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "CMX UPC clock route             ",
           qe_mux->mux.cmxupcr,
          (uint32_t)&(qe_mux->mux.cmxupcr) - (uint32_t)&(qe_mux->mux.cmxgcr));
}


/**************************************************************************
 * Function: display_qe_timer_registers
 *
 * Description: This function display qe timer
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_timer_registers(void)
{
    volatile ccsr_qe_t *qe_timer = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Tmr 1 and tmr 2 global config     ",
     qe_timer->timer.gtcfr1,
    (uint32_t)&(qe_timer->timer.gtcfr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Tmr 3 and tmr 4 global config     ",
     qe_timer->timer.gtcfr2,
    (uint32_t)&(qe_timer->timer.gtcfr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 1 mode                      ",
     qe_timer->timer.gtmdr1,
    (uint32_t)&(qe_timer->timer.gtmdr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 2 mode                      ",
     qe_timer->timer.gtmdr2,
    (uint32_t)&(qe_timer->timer.gtmdr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 1 reference                 ",
     qe_timer->timer.gtrfr1,
    (uint32_t)&(qe_timer->timer.gtrfr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 2 reference                 ",
     qe_timer->timer.gtrfr2,
    (uint32_t)&(qe_timer->timer.gtrfr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 1 capture                   ",
     qe_timer->timer.gtcpr1,
    (uint32_t)&(qe_timer->timer.gtcpr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 2 capture                   ",
     qe_timer->timer.gtcpr2,
    (uint32_t)&(qe_timer->timer.gtcpr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 1 counter                   ",
     qe_timer->timer.gtcnr1,
    (uint32_t)&(qe_timer->timer.gtcnr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 2 counter                   ",
     qe_timer->timer.gtcnr2,
    (uint32_t)&(qe_timer->timer.gtcnr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 3 mode                      ",
     qe_timer->timer.gtmdr3,
    (uint32_t)&(qe_timer->timer.gtmdr3) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 4 mode                      ",
     qe_timer->timer.gtmdr4,
    (uint32_t)&(qe_timer->timer.gtmdr4) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 3 reference                 ",
     qe_timer->timer.gtrfr3,
    (uint32_t)&(qe_timer->timer.gtrfr3) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 4 reference                 ",
     qe_timer->timer.gtrfr4,
    (uint32_t)&(qe_timer->timer.gtrfr4) - (uint32_t)&(qe_timer->timer.gtcfr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 3 capture                   ",
      qe_timer->timer.gtcpr3,
     (uint32_t)&(qe_timer->timer.gtcpr3) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 4 capture                  ",
      qe_timer->timer.gtcpr4,
     (uint32_t)&(qe_timer->timer.gtcpr4) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 3 counter                  ",
      qe_timer->timer.gtcnr3,
     (uint32_t)&(qe_timer->timer.gtcnr3) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 4 counter                  ",
      qe_timer->timer.gtcnr4,
     (uint32_t)&(qe_timer->timer.gtcnr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 1 event                    ",
      qe_timer->timer.gtevr1,
     (uint32_t)&(qe_timer->timer.gtevr1) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 2 event                    ",
      qe_timer->timer.gtevr2,
     (uint32_t)&(qe_timer->timer.gtevr2) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 3 event                    ",
      qe_timer->timer.gtevr3,
     (uint32_t)&(qe_timer->timer.gtevr3) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer 4 event                    ",
      qe_timer->timer.gtevr4,
     (uint32_t)&(qe_timer->timer.gtevr4) - (uint32_t)&(qe_timer->timer.gtcfr1));

     printf(" %20s : 0x%08x @ 0x%08x\n", "Timer prescale                   ",
      qe_timer->timer.gtps,
     (uint32_t)&(qe_timer->timer.gtps) - (uint32_t)&(qe_timer->timer.gtcfr1));
}


/**************************************************************************
 * Function: display_qe_spi1_registers
 *
 * Description: This function display qe spi 1
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_spi1_registers(void)
{
    volatile ccsr_qe_t *qe_spi1 = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 mode                   ",
     qe_spi1->spi1.spmode,
    (uint32_t)&(qe_spi1->spi1.spmode) - (uint32_t)&(qe_spi1->spi1.spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 event                  ",
     qe_spi1->spi1.spie,
    (uint32_t)&(qe_spi1->spi1.spie) - (uint32_t)&(qe_spi1->spi1.spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 mask                   ",
     qe_spi1->spi1.spim,
    (uint32_t)&(qe_spi1->spi1.spim) - (uint32_t)&(qe_spi1->spi1.spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 command                ",
     qe_spi1->spi1.spcom,
    (uint32_t)&(qe_spi1->spi1.spcom) - (uint32_t)&(qe_spi1->spi1.spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 transmit data          ",
     qe_spi1->spi1.spitd,
    (uint32_t)&(qe_spi1->spi1.spitd) - (uint32_t)&(qe_spi1->spi1.spmode));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI 1 receive data           ",
     qe_spi1->spi1.spird,
    (uint32_t)&(qe_spi1->spi1.spird) - (uint32_t)&(qe_spi1->spi1.spmode));
}


/**************************************************************************
 * Function: display_qe_brg_registers
 *
 * Description: This function display qe baud rate generator
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_brg_registers(void)
{
    volatile ccsr_qe_t *qe_brg = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  1 configuration         ",
     qe_brg->brg.brgc1,
    (uint32_t)&(qe_brg->brg.brgc1) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  2 configuration         ",
     qe_brg->brg.brgc2,
    (uint32_t)&(qe_brg->brg.brgc2) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  3 configuration         ",
     qe_brg->brg.brgc3,
    (uint32_t)&(qe_brg->brg.brgc3) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  4 configuration         ",
     qe_brg->brg.brgc4,
    (uint32_t)&(qe_brg->brg.brgc4) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  5 configuration         ",
     qe_brg->brg.brgc5,
    (uint32_t)&(qe_brg->brg.brgc5) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  6 configuration         ",
     qe_brg->brg.brgc6,
    (uint32_t)&(qe_brg->brg.brgc6) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  7 configuration         ",
     qe_brg->brg.brgc7,
    (uint32_t)&(qe_brg->brg.brgc7) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  8 configuration         ",
     qe_brg->brg.brgc8,
    (uint32_t)&(qe_brg->brg.brgc8) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  9 configuration         ",
     qe_brg->brg.brgc9,
    (uint32_t)&(qe_brg->brg.brgc9) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  10 configuration        ",
     qe_brg->brg.brgc10,
    (uint32_t)&(qe_brg->brg.brgc10) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  11 configuration        ",
     qe_brg->brg.brgc11,
    (uint32_t)&(qe_brg->brg.brgc11) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  12 configuration        ",
     qe_brg->brg.brgc12,
    (uint32_t)&(qe_brg->brg.brgc12) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  13 configuration        ",
     qe_brg->brg.brgc13,
    (uint32_t)&(qe_brg->brg.brgc13) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  14 configuration        ",
     qe_brg->brg.brgc14,
    (uint32_t)&(qe_brg->brg.brgc14) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  15 configuration        ",
     qe_brg->brg.brgc15,
    (uint32_t)&(qe_brg->brg.brgc15) - (uint32_t)&(qe_brg->brg.brgc1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "BRG  16 configuration        ",
     qe_brg->brg.brgc16,
    (uint32_t)&(qe_brg->brg.brgc16) - (uint32_t)&(qe_brg->brg.brgc1));
}



/**************************************************************************
 * Function: display_qe_si_registers
 *
 * Description: This function display qe si
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_si_registers(void)
{
    volatile ccsr_qe_t *qe_si = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 TDMA mode                ",
     qe_si->si.siamr1,
    (uint32_t)&(qe_si->si.siamr1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 TDMB mode                ",
     qe_si->si.sibmr1,
    (uint32_t)&(qe_si->si.sibmr1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 TDMC mode                ",
     qe_si->si.sicmr1,
    (uint32_t)&(qe_si->si.sicmr1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 TDMD mode                ",
     qe_si->si.sidmr1,
    (uint32_t)&(qe_si->si.sidmr1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 global mode high         ",
     qe_si->si.siglmr1_h,
    (uint32_t)&(qe_si->si.siglmr1_h) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 command high             ",
     qe_si->si.sicmdr1_h,
    (uint32_t)&(qe_si->si.sicmdr1_h) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 status high              ",
     qe_si->si.sistr1_h,
    (uint32_t)&(qe_si->si.sistr1_h) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM shadow address high  ",
     qe_si->si.sirsr1_h,
    (uint32_t)&(qe_si->si.sirsr1_h) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Tx TDMA      ",
     qe_si->si.sitarc1,
    (uint32_t)&(qe_si->si.sitarc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Tx TDMB      ",
     qe_si->si.sitbrc1,
    (uint32_t)&(qe_si->si.sitbrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Tx TDMC      ",
     qe_si->si.sitcrc1,
    (uint32_t)&(qe_si->si.sitcrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Tx TDMD      ",
     qe_si->si.sitdrc1,
    (uint32_t)&(qe_si->si.sitdrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Rx TDMA      ",
     qe_si->si.sirarc1,
    (uint32_t)&(qe_si->si.sirarc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Rx TDMB      ",
     qe_si->si.sirbrc1,
    (uint32_t)&(qe_si->si.sirbrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Rx TDMC      ",
     qe_si->si.sircrc1,
    (uint32_t)&(qe_si->si.sircrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM counter Rx TDMD      ",
     qe_si->si.sirdrc1,
    (uint32_t)&(qe_si->si.sirdrc1) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 global mode low          ",
     qe_si->si.siglmr1_l,
    (uint32_t)&(qe_si->si.siglmr1_l) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 command low              ",
     qe_si->si.sicmdr1_l,
    (uint32_t)&(qe_si->si.sicmdr1_l) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 status low               ",
     qe_si->si.sistr1_l,
    (uint32_t)&(qe_si->si.sistr1_l) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 RAM shadow address low   ",
     qe_si->si.sirsr1_l,
    (uint32_t)&(qe_si->si.sirsr1_l) - (uint32_t)&(qe_si->si.siamr1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 speed mode               ",
     qe_si->si.sispd,
    (uint32_t)&(qe_si->si.sispd) - (uint32_t)&(qe_si->si.siamr1));
}


/**************************************************************************
 * Function: display_qe_sirt_registers
 *
 * Description: This function display qe si routing table
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_sirt_registers(void)
{
    volatile ccsr_qe_t *qe_sirt = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 Tx routing table         ",
     qe_sirt->sirt.sitxram,
    (uint32_t)&(qe_sirt->sirt.sitxram) - (uint32_t)&(qe_sirt->sirt.sitxram));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SI1 Rx routing table         ",
     qe_sirt->sirt.sirxram,
    (uint32_t)&(qe_sirt->sirt.sirxram) - (uint32_t)&(qe_sirt->sirt.sitxram));
}


/**************************************************************************
 * Function: display_qe_ucc1_registers
 *
 * Description: This function display qe ucc 1
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_ucc1_registers(void)
{
    volatile ccsr_qe_t *qe_ucc1 = &(REGB->qe);

    printf("\n----------------------- SLOW MODE --------------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow general mode (low)             ",
     qe_ucc1->ucc1.mode.slow.gumr_l,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow general mode (high)            ",
     qe_ucc1->ucc1.mode.slow.gumr_h,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_h) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow protocol-specific mode         ",
     qe_ucc1->ucc1.mode.slow.upsmr,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.upsmr) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow transmit on demand             ",
     qe_ucc1->ucc1.mode.slow.utodr,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.utodr) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow data synchronization           ",
     qe_ucc1->ucc1.mode.slow.udsr,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.udsr) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow event                          ",
     qe_ucc1->ucc1.mode.slow.ucce,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.ucce) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow mask                           ",
     qe_ucc1->ucc1.mode.slow.uccm,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.uccm) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow status                         ",
     qe_ucc1->ucc1.mode.slow.uccs,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.uccs) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow transmit polling timer         ",
     qe_ucc1->ucc1.mode.slow.utpt,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.utpt) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Slow general extended mode register ",
     qe_ucc1->ucc1.mode.slow.guemr,
    (uint32_t)&(qe_ucc1->ucc1.mode.slow.guemr) - (uint32_t)&(qe_ucc1->ucc1.mode.slow.gumr_l));

    printf("\n------------------- FAST MODE GENERIC ---------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast general mode register          ",
     qe_ucc1->ucc1.mode.fast.reg.gumr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast protocol-specific mode         ",
     qe_ucc1->ucc1.mode.fast.reg.upsmr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.upsmr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast transmit on demand             ",
     qe_ucc1->ucc1.mode.fast.reg.utodr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utodr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast data synchronization           ",
     qe_ucc1->ucc1.mode.fast.reg.udsr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.udsr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast event                          ",
     qe_ucc1->ucc1.mode.fast.reg.ucce,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.ucce) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast mask                           ",
     qe_ucc1->ucc1.mode.fast.reg.uccm,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.uccm) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast status                         ",
     qe_ucc1->ucc1.mode.fast.reg.uccs,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.uccs) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast receive FIFO base              ",
     qe_ucc1->ucc1.mode.fast.reg.urfb,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.urfb) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast receive FIFO size              ",
     qe_ucc1->ucc1.mode.fast.reg.urfs,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.urfs) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx FIFO emergency threshold    ",
     qe_ucc1->ucc1.mode.fast.reg.urfet,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.urfet) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx FIFO special emergency thrsh",
     qe_ucc1->ucc1.mode.fast.reg.urfset,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.urfset) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast transmit FIFO base             ",
     qe_ucc1->ucc1.mode.fast.reg.utfb,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utfb) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast transmit FIFO size             ",
     qe_ucc1->ucc1.mode.fast.reg.utfs,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utfs) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx FIFO emergency              ",
     qe_ucc1->ucc1.mode.fast.reg.utfet,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utfet) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast transmit FIFO tx threshold     ",
     qe_ucc1->ucc1.mode.fast.reg.utftt,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utftt) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast transmit polling timer         ",
     qe_ucc1->ucc1.mode.fast.reg.utpt,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.utpt) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast retry counter                  ",
     qe_ucc1->ucc1.mode.fast.reg.urtry,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.urtry) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast general extended mode          ",
     qe_ucc1->ucc1.mode.fast.reg.guemr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.guemr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.reg.gumr));

    printf("\n------------------- FAST MODE Ethernet general configuration-------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MAC configuration 1            ",
     qe_ucc1->ucc1.mode.fast.eth.maccfg1,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MAC configuration 2            ",
     qe_ucc1->ucc1.mode.fast.eth.maccfg2,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg2) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Interframe gap                 ",
     qe_ucc1->ucc1.mode.fast.eth.ipgifg,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.ipgifg) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Half-duplex                    ",
     qe_ucc1->ucc1.mode.fast.eth.hafdup,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.hafdup) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Ethernet MAC test              ",
     qe_ucc1->ucc1.mode.fast.eth.emtr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.emtr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt configuration         ",
     qe_ucc1->ucc1.mode.fast.eth.miimcfg,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimcfg) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt command               ",
     qe_ucc1->ucc1.mode.fast.eth.miimcom,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimcom) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt address               ",
     qe_ucc1->ucc1.mode.fast.eth.miimadd,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimadd) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt control               ",
     qe_ucc1->ucc1.mode.fast.eth.miimcon,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimcon) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt status                ",
     qe_ucc1->ucc1.mode.fast.eth.miimstat,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimstat) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast MII mgmt indication            ",
     qe_ucc1->ucc1.mode.fast.eth.miimind,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.miimind) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Interface control              ",
     qe_ucc1->ucc1.mode.fast.eth.ifctl,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.ifctl) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Interface status               ",
     qe_ucc1->ucc1.mode.fast.eth.ifstat,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.ifstat) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Station address part 1         ",
     qe_ucc1->ucc1.mode.fast.eth.macstnaddr1,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.macstnaddr1) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Station address part 2         ",
     qe_ucc1->ucc1.mode.fast.eth.macstnaddr2,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.macstnaddr2) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast UCC Ethernet MAC parameter     ",
     qe_ucc1->ucc1.mode.fast.eth.uempr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.uempr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast UCC Ethernet statistics control",
     qe_ucc1->ucc1.mode.fast.eth.uescr,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.uescr) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.eth.maccfg1));

    printf("\n------------------- FAST MODE Ethernet Statistics Counters -------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx/Rx 64 byte frame counter    ",
     qe_ucc1->ucc1.mode.fast.mib.tx64,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx/Rx 65-127 byte frame counter",
     qe_ucc1->ucc1.mode.fast.mib.tx127,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx127) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast 128-255 byte frame counter     ",
     qe_ucc1->ucc1.mode.fast.mib.tx255,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx255) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx/Rx 64 byte frame counter    ",
     qe_ucc1->ucc1.mode.fast.mib.rx64,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rx64) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx/Rx 65-127 byte frame counter",
     qe_ucc1->ucc1.mode.fast.mib.rx127,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rx127) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx/Rx 128-255 byte frame counte",
     qe_ucc1->ucc1.mode.fast.mib.rx255,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rx255) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx good bytes counter          ",
     qe_ucc1->ucc1.mode.fast.mib.txok,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.txok) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx control frame counter       ",
     qe_ucc1->ucc1.mode.fast.mib.txcf,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.txcf) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx mcast control frame counter ",
     qe_ucc1->ucc1.mode.fast.mib.tmca,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tmca) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Tx broadcast packet counter    ",
     qe_ucc1->ucc1.mode.fast.mib.tbca,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tbca) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx frame OK counter            ",
     qe_ucc1->ucc1.mode.fast.mib.rxfok,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rxfok) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx good and bad bytes counter  ",
     qe_ucc1->ucc1.mode.fast.mib.rbyt,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rbyt) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx bytes OK counter            ",
     qe_ucc1->ucc1.mode.fast.mib.rxbok,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rxbok) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx multicast packet counter    ",
     qe_ucc1->ucc1.mode.fast.mib.rmca,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rmca) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Rx broadcast packet counter    ",
     qe_ucc1->ucc1.mode.fast.mib.rbca,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.rbca) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Statistics carry               ",
     qe_ucc1->ucc1.mode.fast.mib.scar,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.scar) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 1 Mode Fast Statistics carry mask          ",
     qe_ucc1->ucc1.mode.fast.mib.scam,
    (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.scam) - (uint32_t)&(qe_ucc1->ucc1.mode.fast.mib.tx64));
}


/**************************************************************************
 * Function: display_qe_ucc3_registers
 *
 * Description: This function display qe ucc 3
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_ucc3_registers(void)
{
    volatile ccsr_qe_t *qe_ucc3 = &(REGB->qe);

    printf("\n----------------------- SLOW MODE --------------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow general mode (low)             ",
     qe_ucc3->ucc3.mode.slow.gumr_l,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow general mode (high)            ",
     qe_ucc3->ucc3.mode.slow.gumr_h,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_h) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow protocol-specific mode         ",
     qe_ucc3->ucc3.mode.slow.upsmr,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.upsmr) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow transmit on demand             ",
     qe_ucc3->ucc3.mode.slow.utodr,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.utodr) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow data synchronization           ",
     qe_ucc3->ucc3.mode.slow.udsr,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.udsr) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow event                          ",
     qe_ucc3->ucc3.mode.slow.ucce,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.ucce) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow mask                           ",
     qe_ucc3->ucc3.mode.slow.uccm,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.uccm) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow status                         ",
     qe_ucc3->ucc3.mode.slow.uccs,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.uccs) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow transmit polling timer         ",
     qe_ucc3->ucc3.mode.slow.utpt,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.utpt) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Slow general extended mode register ",
     qe_ucc3->ucc3.mode.slow.guemr,
    (uint32_t)&(qe_ucc3->ucc3.mode.slow.guemr) - (uint32_t)&(qe_ucc3->ucc3.mode.slow.gumr_l));

    printf("\n------------------- FAST MODE GENERIC ---------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast general mode register          ",
     qe_ucc3->ucc3.mode.fast.reg.gumr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast protocol-specific mode         ",
     qe_ucc3->ucc3.mode.fast.reg.upsmr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.upsmr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast transmit on demand             ",
     qe_ucc3->ucc3.mode.fast.reg.utodr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utodr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast data synchronization           ",
     qe_ucc3->ucc3.mode.fast.reg.udsr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.udsr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast event                          ",
     qe_ucc3->ucc3.mode.fast.reg.ucce,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.ucce) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast mask                           ",
     qe_ucc3->ucc3.mode.fast.reg.uccm,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.uccm) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast status                         ",
     qe_ucc3->ucc3.mode.fast.reg.uccs,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.uccs) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast receive FIFO base              ",
     qe_ucc3->ucc3.mode.fast.reg.urfb,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.urfb) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast receive FIFO size              ",
     qe_ucc3->ucc3.mode.fast.reg.urfs,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.urfs) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx FIFO emergency threshold    ",
     qe_ucc3->ucc3.mode.fast.reg.urfet,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.urfet) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx FIFO special emergency thrsh",
     qe_ucc3->ucc3.mode.fast.reg.urfset,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.urfset) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast transmit FIFO base             ",
     qe_ucc3->ucc3.mode.fast.reg.utfb,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utfb) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast transmit FIFO size             ",
     qe_ucc3->ucc3.mode.fast.reg.utfs,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utfs) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Tx FIFO emergency              ",
     qe_ucc3->ucc3.mode.fast.reg.utfet,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utfet) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast transmit FIFO tx threshold     ",
     qe_ucc3->ucc3.mode.fast.reg.utftt,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utftt) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast transmit polling timer         ",
     qe_ucc3->ucc3.mode.fast.reg.utpt,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.utpt) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast retry counter                  ",
     qe_ucc3->ucc3.mode.fast.reg.urtry,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.urtry) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast general extended mode          ",
     qe_ucc3->ucc3.mode.fast.reg.guemr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.guemr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.reg.gumr));

    printf("\n------------------- FAST MODE Ethernet general configuration-------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MAC configuration 1            ",
     qe_ucc3->ucc3.mode.fast.eth.maccfg1,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MAC configuration 2            ",
     qe_ucc3->ucc3.mode.fast.eth.maccfg2,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg2) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Interframe gap                 ",
     qe_ucc3->ucc3.mode.fast.eth.ipgifg,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.ipgifg) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Half-duplex                    ",
     qe_ucc3->ucc3.mode.fast.eth.hafdup,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.hafdup) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Ethernet MAC test              ",
     qe_ucc3->ucc3.mode.fast.eth.emtr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.emtr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt configuration         ",
     qe_ucc3->ucc3.mode.fast.eth.miimcfg,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimcfg) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt command               ",
     qe_ucc3->ucc3.mode.fast.eth.miimcom,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimcom) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt address               ",
     qe_ucc3->ucc3.mode.fast.eth.miimadd,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimadd) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt control               ",
     qe_ucc3->ucc3.mode.fast.eth.miimcon,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimcon) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt status                ",
     qe_ucc3->ucc3.mode.fast.eth.miimstat,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimstat) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast MII mgmt indication            ",
     qe_ucc3->ucc3.mode.fast.eth.miimind,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.miimind) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Interface control              ",
     qe_ucc3->ucc3.mode.fast.eth.ifctl,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.ifctl) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Interface status               ",
     qe_ucc3->ucc3.mode.fast.eth.ifstat,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.ifstat) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Station address part 1         ",
     qe_ucc3->ucc3.mode.fast.eth.macstnaddr1,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.macstnaddr1) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Station address part 2         ",
     qe_ucc3->ucc3.mode.fast.eth.macstnaddr2,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.macstnaddr2) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast UCC Ethernet MAC parameter     ",
     qe_ucc3->ucc3.mode.fast.eth.uempr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.uempr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast UCC Ethernet statistics control",
     qe_ucc3->ucc3.mode.fast.eth.uescr,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.uescr) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.eth.maccfg1));

    printf("\n------------------- FAST MODE Ethernet Statistics Counters -------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Tx/Rx 64 byte frame counter    ",
     qe_ucc3->ucc3.mode.fast.mib.tx64,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Tx/Rx 65-127 byte frame counter",
     qe_ucc3->ucc3.mode.fast.mib.tx127,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx127) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast 128-255 byte frame counter     ",
     qe_ucc3->ucc3.mode.fast.mib.tx255,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx255) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx/Rx 64 byte frame counter    ",
     qe_ucc3->ucc3.mode.fast.mib.rx64,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rx64) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx/Rx 65-127 byte frame counter",
     qe_ucc3->ucc3.mode.fast.mib.rx127,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rx127) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx/Rx 128-255 byte frame counte",
     qe_ucc3->ucc3.mode.fast.mib.rx255,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rx255) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Transmit good bytes counter    ",
     qe_ucc3->ucc3.mode.fast.mib.txok,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.txok) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Transmit control frame counter ",
     qe_ucc3->ucc3.mode.fast.mib.txcf,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.txcf) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Tx mcast control frame counter ",
     qe_ucc3->ucc3.mode.fast.mib.tmca,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tmca) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Tx broadcast packet counter    ",
     qe_ucc3->ucc3.mode.fast.mib.tbca,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tbca) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx frame OK counter            ",
     qe_ucc3->ucc3.mode.fast.mib.rxfok,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rxfok) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx good and bad bytes counter  ",
     qe_ucc3->ucc3.mode.fast.mib.rbyt,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rbyt) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx bytes OK counter            ",
     qe_ucc3->ucc3.mode.fast.mib.rxbok,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rxbok) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx multicast packet counter    ",
     qe_ucc3->ucc3.mode.fast.mib.rmca,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rmca) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Rx broadcast packet counter    ",
     qe_ucc3->ucc3.mode.fast.mib.rbca,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.rbca) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Statistics carry               ",
     qe_ucc3->ucc3.mode.fast.mib.scar,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.scar) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 3 Mode Fast Statistics carry mask          ",
     qe_ucc3->ucc3.mode.fast.mib.scam,
    (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.scam) - (uint32_t)&(qe_ucc3->ucc3.mode.fast.mib.tx64));
}


/**************************************************************************
 * Function: display_qe_ucc5_registers
 *
 * Description: This function display qe ucc 5
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_ucc5_registers(void)
{
    volatile ccsr_qe_t *qe_ucc5 = &(REGB->qe);

    printf("\n----------------------- SLOW MODE --------------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow general mode (low)             ",
     qe_ucc5->ucc5.mode.slow.gumr_l,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow general mode (high)            ",
     qe_ucc5->ucc5.mode.slow.gumr_h,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_h) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow protocol-specific mode         ",
     qe_ucc5->ucc5.mode.slow.upsmr,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.upsmr) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow transmit on demand             ",
     qe_ucc5->ucc5.mode.slow.utodr,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.utodr) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow data synchronization           ",
     qe_ucc5->ucc5.mode.slow.udsr,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.udsr) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow event                          ",
     qe_ucc5->ucc5.mode.slow.ucce,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.ucce) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow mask                           ",
     qe_ucc5->ucc5.mode.slow.uccm,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.uccm) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow status                         ",
     qe_ucc5->ucc5.mode.slow.uccs,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.uccs) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow transmit polling timer         ",
     qe_ucc5->ucc5.mode.slow.utpt,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.utpt) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Slow general extended mode register ",
     qe_ucc5->ucc5.mode.slow.guemr,
    (uint32_t)&(qe_ucc5->ucc5.mode.slow.guemr) - (uint32_t)&(qe_ucc5->ucc5.mode.slow.gumr_l));

    printf("\n------------------- FAST MODE GENERIC ---------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast general mode register          ",
     qe_ucc5->ucc5.mode.fast.reg.gumr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast protocol-specific mode         ",
     qe_ucc5->ucc5.mode.fast.reg.upsmr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.upsmr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast transmit on demand             ",
     qe_ucc5->ucc5.mode.fast.reg.utodr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utodr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast data synchronization           ",
     qe_ucc5->ucc5.mode.fast.reg.udsr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.udsr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast event                          ",
     qe_ucc5->ucc5.mode.fast.reg.ucce,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.ucce) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast mask                           ",
     qe_ucc5->ucc5.mode.fast.reg.uccm,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.uccm) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast status                         ",
     qe_ucc5->ucc5.mode.fast.reg.uccs,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.uccs) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast receive FIFO base              ",
     qe_ucc5->ucc5.mode.fast.reg.urfb,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.urfb) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast receive FIFO size              ",
     qe_ucc5->ucc5.mode.fast.reg.urfs,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.urfs) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx FIFO emergency threshold    ",
     qe_ucc5->ucc5.mode.fast.reg.urfet,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.urfet) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx FIFO special emergency thrsh",
     qe_ucc5->ucc5.mode.fast.reg.urfset,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.urfset) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast transmit FIFO base             ",
     qe_ucc5->ucc5.mode.fast.reg.utfb,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utfb) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast transmit FIFO size             ",
     qe_ucc5->ucc5.mode.fast.reg.utfs,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utfs) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Tx FIFO emergency              ",
     qe_ucc5->ucc5.mode.fast.reg.utfet,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utfet) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast transmit FIFO tx threshold     ",
     qe_ucc5->ucc5.mode.fast.reg.utftt,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utftt) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast transmit polling timer         ",
     qe_ucc5->ucc5.mode.fast.reg.utpt,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.utpt) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast retry counter                  ",
     qe_ucc5->ucc5.mode.fast.reg.urtry,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.urtry) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast general extended mode          ",
     qe_ucc5->ucc5.mode.fast.reg.guemr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.guemr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.reg.gumr));

    printf("\n------------------- FAST MODE Ethernet general configuration-------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MAC configuration 1            ",
     qe_ucc5->ucc5.mode.fast.eth.maccfg1,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MAC configuration 2            ",
     qe_ucc5->ucc5.mode.fast.eth.maccfg2,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg2) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Interframe gap                 ",
     qe_ucc5->ucc5.mode.fast.eth.ipgifg,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.ipgifg) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Half-duplex                    ",
     qe_ucc5->ucc5.mode.fast.eth.hafdup,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.hafdup) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Ethernet MAC test              ",
     qe_ucc5->ucc5.mode.fast.eth.emtr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.emtr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt configuration         ",
     qe_ucc5->ucc5.mode.fast.eth.miimcfg,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimcfg) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt command               ",
     qe_ucc5->ucc5.mode.fast.eth.miimcom,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimcom) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt address               ",
     qe_ucc5->ucc5.mode.fast.eth.miimadd,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimadd) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt control               ",
     qe_ucc5->ucc5.mode.fast.eth.miimcon,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimcon) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt status                ",
     qe_ucc5->ucc5.mode.fast.eth.miimstat,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimstat) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast MII mgmt indication            ",
     qe_ucc5->ucc5.mode.fast.eth.miimind,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.miimind) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Interface control              ",
     qe_ucc5->ucc5.mode.fast.eth.ifctl,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.ifctl) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Interface status               ",
     qe_ucc5->ucc5.mode.fast.eth.ifstat,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.ifstat) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Station address part 1         ",
     qe_ucc5->ucc5.mode.fast.eth.macstnaddr1,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.macstnaddr1) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Station address part 2         ",
     qe_ucc5->ucc5.mode.fast.eth.macstnaddr2,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.macstnaddr2) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast UCC Ethernet MAC parameter     ",
     qe_ucc5->ucc5.mode.fast.eth.uempr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.uempr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast UCC Ethernet statistics control",
     qe_ucc5->ucc5.mode.fast.eth.uescr,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.uescr) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.eth.maccfg1));

    printf("\n------------------- FAST MODE Ethernet Statistics Counters -------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Tx/Rx 64 byte frame counter    ",
     qe_ucc5->ucc5.mode.fast.mib.tx64,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Tx/Rx 65-127 byte frame counter",
     qe_ucc5->ucc5.mode.fast.mib.tx127,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx127) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast 128-255 byte frame counter     ",
     qe_ucc5->ucc5.mode.fast.mib.tx255,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx255) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx/Rx 64 byte frame counter    ",
     qe_ucc5->ucc5.mode.fast.mib.rx64,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rx64) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx/Rx 65-127 byte frame counter",
     qe_ucc5->ucc5.mode.fast.mib.rx127,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rx127) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx/Rx 128-255 byte frame counte",
     qe_ucc5->ucc5.mode.fast.mib.rx255,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rx255) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Transmit good bytes counter    ",
     qe_ucc5->ucc5.mode.fast.mib.txok,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.txok) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Transmit control frame counter ",
     qe_ucc5->ucc5.mode.fast.mib.txcf,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.txcf) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Tx mcast control frame counter ",
     qe_ucc5->ucc5.mode.fast.mib.tmca,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tmca) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Tx broadcast packet counter    ",
     qe_ucc5->ucc5.mode.fast.mib.tbca,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tbca) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx frame OK counter            ",
     qe_ucc5->ucc5.mode.fast.mib.rxfok,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rxfok) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx good and bad bytes counter  ",
     qe_ucc5->ucc5.mode.fast.mib.rbyt,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rbyt) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx bytes OK counter            ",
     qe_ucc5->ucc5.mode.fast.mib.rxbok,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rxbok) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx multicast packet counter    ",
     qe_ucc5->ucc5.mode.fast.mib.rmca,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rmca) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Rx broadcast packet counter    ",
     qe_ucc5->ucc5.mode.fast.mib.rbca,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.rbca) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Statistics carry               ",
     qe_ucc5->ucc5.mode.fast.mib.scar,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.scar) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 5 Mode Fast Statistics carry mask          ",
     qe_ucc5->ucc5.mode.fast.mib.scam,
    (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.scam) - (uint32_t)&(qe_ucc5->ucc5.mode.fast.mib.tx64));
}


/**************************************************************************
 * Function: display_qe_ucc7_registers
 *
 * Description: This function display qe ucc 7
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_ucc7_registers(void)
{
    volatile ccsr_qe_t *qe_ucc7 = &(REGB->qe);

    printf("\n----------------------- SLOW MODE --------------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow general mode (low)             ",
     qe_ucc7->ucc7.mode.slow.gumr_l,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow general mode (high)            ",
     qe_ucc7->ucc7.mode.slow.gumr_h,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_h) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow protocol-specific mode         ",
     qe_ucc7->ucc7.mode.slow.upsmr,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.upsmr) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow transmit on demand             ",
     qe_ucc7->ucc7.mode.slow.utodr,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.utodr) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow data synchronization           ",
     qe_ucc7->ucc7.mode.slow.udsr,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.udsr) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow event                          ",
     qe_ucc7->ucc7.mode.slow.ucce,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.ucce) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow mask                           ",
     qe_ucc7->ucc7.mode.slow.uccm,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.uccm) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow status                         ",
     qe_ucc7->ucc7.mode.slow.uccs,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.uccs) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow transmit polling timer         ",
     qe_ucc7->ucc7.mode.slow.utpt,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.utpt) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Slow general extended mode register ",
     qe_ucc7->ucc7.mode.slow.guemr,
    (uint32_t)&(qe_ucc7->ucc7.mode.slow.guemr) - (uint32_t)&(qe_ucc7->ucc7.mode.slow.gumr_l));

    printf("\n------------------- FAST MODE GENERIC ---------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast general mode register          ",
     qe_ucc7->ucc7.mode.fast.reg.gumr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast protocol-specific mode         ",
     qe_ucc7->ucc7.mode.fast.reg.upsmr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.upsmr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast transmit on demand             ",
     qe_ucc7->ucc7.mode.fast.reg.utodr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utodr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast data synchronization           ",
     qe_ucc7->ucc7.mode.fast.reg.udsr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.udsr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast event                          ",
     qe_ucc7->ucc7.mode.fast.reg.ucce,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.ucce) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast mask                           ",
     qe_ucc7->ucc7.mode.fast.reg.uccm,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.uccm) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast status                         ",
     qe_ucc7->ucc7.mode.fast.reg.uccs,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.uccs) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast receive FIFO base              ",
     qe_ucc7->ucc7.mode.fast.reg.urfb,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.urfb) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast receive FIFO size              ",
     qe_ucc7->ucc7.mode.fast.reg.urfs,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.urfs) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx FIFO emergency threshold    ",
     qe_ucc7->ucc7.mode.fast.reg.urfet,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.urfet) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx FIFO special emergency thrsh",
     qe_ucc7->ucc7.mode.fast.reg.urfset,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.urfset) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast transmit FIFO base             ",
     qe_ucc7->ucc7.mode.fast.reg.utfb,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utfb) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast transmit FIFO size             ",
     qe_ucc7->ucc7.mode.fast.reg.utfs,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utfs) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx FIFO emergency              ",
     qe_ucc7->ucc7.mode.fast.reg.utfet,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utfet) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast transmit FIFO tx threshold     ",
     qe_ucc7->ucc7.mode.fast.reg.utftt,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utftt) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast transmit polling timer         ",
     qe_ucc7->ucc7.mode.fast.reg.utpt,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.utpt) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast retry counter                  ",
     qe_ucc7->ucc7.mode.fast.reg.urtry,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.urtry) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast general extended mode          ",
     qe_ucc7->ucc7.mode.fast.reg.guemr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.guemr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.reg.gumr));

    printf("\n------------------- FAST MODE Ethernet general configuration-------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MAC configuration 1            ",
     qe_ucc7->ucc7.mode.fast.eth.maccfg1,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MAC configuration 2            ",
     qe_ucc7->ucc7.mode.fast.eth.maccfg2,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg2) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Interframe gap                 ",
     qe_ucc7->ucc7.mode.fast.eth.ipgifg,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.ipgifg) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Half-duplex                    ",
     qe_ucc7->ucc7.mode.fast.eth.hafdup,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.hafdup) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Ethernet MAC test              ",
     qe_ucc7->ucc7.mode.fast.eth.emtr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.emtr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt configuration         ",
     qe_ucc7->ucc7.mode.fast.eth.miimcfg,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimcfg) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt command               ",
     qe_ucc7->ucc7.mode.fast.eth.miimcom,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimcom) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt address               ",
     qe_ucc7->ucc7.mode.fast.eth.miimadd,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimadd) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt control               ",
     qe_ucc7->ucc7.mode.fast.eth.miimcon,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimcon) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt status                ",
     qe_ucc7->ucc7.mode.fast.eth.miimstat,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimstat) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast MII mgmt indication            ",
     qe_ucc7->ucc7.mode.fast.eth.miimind,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.miimind) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Interface control              ",
     qe_ucc7->ucc7.mode.fast.eth.ifctl,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.ifctl) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Interface status               ",
     qe_ucc7->ucc7.mode.fast.eth.ifstat,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.ifstat) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Station address part 1         ",
     qe_ucc7->ucc7.mode.fast.eth.macstnaddr1,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.macstnaddr1) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Station address part 2         ",
     qe_ucc7->ucc7.mode.fast.eth.macstnaddr2,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.macstnaddr2) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast UCC Ethernet MAC parameter     ",
     qe_ucc7->ucc7.mode.fast.eth.uempr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.uempr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast UCC Ethernet statistics control",
     qe_ucc7->ucc7.mode.fast.eth.uescr,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.uescr) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.eth.maccfg1));

    printf("\n------------------- FAST MODE Ethernet Statistics Counters -------------------\n");
    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx/Rx 64 byte frame counter    ",
     qe_ucc7->ucc7.mode.fast.mib.tx64,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx/Rx 65-127 byte frame counter",
     qe_ucc7->ucc7.mode.fast.mib.tx127,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx127) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast 128-255 byte frame counter     ",
     qe_ucc7->ucc7.mode.fast.mib.tx255,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx255) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx/Rx 64 byte frame counter    ",
     qe_ucc7->ucc7.mode.fast.mib.rx64,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rx64) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx/Rx 65-127 byte frame counter",
     qe_ucc7->ucc7.mode.fast.mib.rx127,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rx127) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx/Rx 128-255 byte frame counte",
     qe_ucc7->ucc7.mode.fast.mib.rx255,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rx255) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx good bytes counter          ",
     qe_ucc7->ucc7.mode.fast.mib.txok,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.txok) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx control frame counter       ",
     qe_ucc7->ucc7.mode.fast.mib.txcf,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.txcf) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx mcast control frame counter ",
     qe_ucc7->ucc7.mode.fast.mib.tmca,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tmca) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Tx broadcast packet counter    ",
     qe_ucc7->ucc7.mode.fast.mib.tbca,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tbca) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx frame OK counter            ",
     qe_ucc7->ucc7.mode.fast.mib.rxfok,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rxfok) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx good and bad bytes counter  ",
     qe_ucc7->ucc7.mode.fast.mib.rbyt,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rbyt) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx bytes OK counter            ",
     qe_ucc7->ucc7.mode.fast.mib.rxbok,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rxbok) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx multicast packet counter    ",
     qe_ucc7->ucc7.mode.fast.mib.rmca,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rmca) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Rx broadcast packet counter    ",
     qe_ucc7->ucc7.mode.fast.mib.rbca,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.rbca) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Statistics carry               ",
     qe_ucc7->ucc7.mode.fast.mib.scar,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.scar) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC 7 Mode Fast Statistics carry mask          ",
     qe_ucc7->ucc7.mode.fast.mib.scam,
    (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.scam) - (uint32_t)&(qe_ucc7->ucc7.mode.fast.mib.tx64));
}


/**************************************************************************
 * Function: display_qe_utopia_registers
 *
 * Description: This function display qe multi phy controller
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_utopia_registers(void)
{
    printf("\nNot support UTOPIA\n");
}


/**************************************************************************
 * Function: display_qe_sdma_registers
 *
 * Description: This function display qe sdma
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_sdma_registers(void)
{
    volatile ccsr_qe_t *qe_sdma = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "Serial DMA status               ",
     qe_sdma->sdma.sdsr,
    (uint32_t)&(qe_sdma->sdma.sdsr) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Serial DMA mode                 ",
     qe_sdma->sdma.sdmr,
    (uint32_t)&(qe_sdma->sdma.sdmr) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA system bus threshold       ",
     qe_sdma->sdma.sdtr1,
    (uint32_t)&(qe_sdma->sdma.sdtr1) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA secondary bus threshold    ",
     qe_sdma->sdma.sdtr2,
    (uint32_t)&(qe_sdma->sdma.sdtr2) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA system bus hysteresis      ",
     qe_sdma->sdma.sdhy1,
    (uint32_t)&(qe_sdma->sdma.sdhy1) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA secondary bus hysteresis   ",
     qe_sdma->sdma.sdhy2,
    (uint32_t)&(qe_sdma->sdma.sdhy2) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA sytem bus address          ",
     qe_sdma->sdma.sdta1,
    (uint32_t)&(qe_sdma->sdma.sdta1) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA secondary bus address      ",
     qe_sdma->sdma.sdta2,
    (uint32_t)&(qe_sdma->sdma.sdta2) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA system bus MSNUM           ",
     qe_sdma->sdma.sdtm1,
    (uint32_t)&(qe_sdma->sdma.sdtm1) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA secondary bus MSNUM        ",
     qe_sdma->sdma.sdtm2,
    (uint32_t)&(qe_sdma->sdma.sdtm2) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA address bus quality        ",
     qe_sdma->sdma.sdaqr,
    (uint32_t)&(qe_sdma->sdma.sdaqr) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA address bus quality mask   ",
     qe_sdma->sdma.sdaqmr,
    (uint32_t)&(qe_sdma->sdma.sdaqmr) - (uint32_t)&(qe_sdma->sdma.sdsr));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SDMA CAM entries base           ",
     qe_sdma->sdma.sdebcr,
    (uint32_t)&(qe_sdma->sdma.sdebcr) - (uint32_t)&(qe_sdma->sdma.sdsr));
}


/**************************************************************************
 * Function: display_qe_muram_registers
 *
 * Description: This function display qe multi user ram
 *
 * Input: None
 *
 * Output: None
 *************************************************************************/
void
display_qe_muram_registers(void)
{
    volatile ccsr_qe_t *qe_muram = &(REGB->qe);

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC5  PARAM  (0x13000 - 0x130FF)  ",
     qe_muram->muram.ucc5,
    (uint32_t)&(qe_muram->muram.ucc5) - (uint32_t)&(qe_muram->muram.ucc5));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC7  PARAM  (0x13200 - 0x132FF)  ",
     qe_muram->muram.ucc7,
    (uint32_t)&(qe_muram->muram.ucc7) - (uint32_t)&(qe_muram->muram.ucc5));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC1  PARAM  (0x13400 - 0x134FF)  ",
     qe_muram->muram.ucc1,
    (uint32_t)&(qe_muram->muram.ucc1) - (uint32_t)&(qe_muram->muram.ucc5));

    printf(" %20s : 0x%08x @ 0x%08x\n", "UCC3  PARAM  (0x13600 - 0x136FF)  ",
     qe_muram->muram.ucc3,
    (uint32_t)&(qe_muram->muram.ucc3) - (uint32_t)&(qe_muram->muram.ucc5));

    printf(" %20s : 0x%08x @ 0x%08x\n", "SPI1  PARAM  (0x13900 - 0x1397F)  ",
     qe_muram->muram.spi1,
    (uint32_t)&(qe_muram->muram.spi1) - (uint32_t)&(qe_muram->muram.ucc5));

    printf(" %20s : 0x%08x @ 0x%08x\n", "Timer PARAM  (0x13A00 - 0x13A3F)  ",
     qe_muram->muram.timer,
    (uint32_t)&(qe_muram->muram.timer) - (uint32_t)&(qe_muram->muram.ucc5));
}


/******** History ******** 
/*------------------------------------------------------------------------------
 * $Log: p1021_utils.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2012/08/21 01:17:09  huanngo
 * Adding the printing for External Interrupt Summary Register when dumping CPU registers
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.5  2011/09/12 08:29:48  steja
 * Update P1021 Display Register QE code
 *
 * Revision 1.1.4.4  2011/08/30 10:23:03  steja
 * Update P1021 display QE register code
 *
 * Revision 1.1.4.3  2011/08/26 14:44:56  steja
 * Update p1021 code to display SPI registers
 *
 * Revision 1.1.4.2  2011/08/18 19:43:24  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.2  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.1  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

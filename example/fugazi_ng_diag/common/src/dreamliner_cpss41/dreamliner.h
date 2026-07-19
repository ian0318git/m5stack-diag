/* $Id: dreamliner.h,v 1.2 2019/12/11 10:10:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner.h,v $
 *------------------------------------------------------------------
 *
 * dreamliner.h - This file contains all the definitions for Dreamliner NGWIC.
 *
 * Christine Wen -- Nov. 2013
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#define PHY_ADDR              0x00
#define FPGA_ADDR             0x00

#define DREAMLINER_GE_BP_PACKET_NO    1
#define DREAMLINER_4GE_PHY_PORTS      4
#define DREAMLINER_8GE_PHY_PORTS      8

#define ONE_SECOND 1000      /* second in unit of milliseconds */

extern uint32_t xcat2_dev_num[3];

extern int get_port_num();
extern int get_slot_num();
extern boolean is_poe_sku(); 
extern int sgmii_lpbk_util(int, int);
extern boolean is_daughter_card_present();
extern int dl_read_i2c(uchar, uchar, uint, uchar *);
extern int dl_write_i2c(uchar, uchar, uint, uchar *);
extern int dl_write_spi(uint spi_addr, uchar opcode, uint size, uint32_t mem_data, boolean addr_incl);
extern int dl_read_spi(uint spi_addr, uchar opcode, uint size, uint32_t *mem_data, boolean addr_incl);
extern int led_class_config(uint32_t led_class, boolean inv_ena, boolean blk_ena, int blk_sel, boolean force_ena, uint32_t force_data);
extern int xcat2_display_counter(boolean display);
extern void cterr_setup();
/* Define for GOLDBEACH */
#define PCI_RESCAN          "echo 1 > /sys/bus/pci/rescan"
#define BUFFER_LENGTH       128
#define COMMAND_LENGTH      80
#define FIND_DEVICE_CMD     "find /sys -name *"
#define ECHO_ONE_CMD        "echo 1 > "
#define LSPCI_FILE          "/tmp/lspci_dreamliner_bus"
#define PCI_BUS_FILE        "/tmp/find_dreamliner_bus"
#define LSPCI_CMD           "lspci -nn | grep -i 11ab: | cut -c 0-7 > %s"
#define PCI_DEV_CMP         "/sys/devices/pci0000:00/"
#define REMOVE_STR          "/remove"


/*
 *------------------------------------------------------------------
 * $Log: dreamliner.h,v $
 * Revision 1.2  2019/12/11 10:10:24  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */

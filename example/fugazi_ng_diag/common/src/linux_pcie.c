/* $Id: linux_pcie.c,v 1.7 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_pcie.c,v $
 *------------------------------------------------------------------
 *
 *
 * 5/2008
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "types.h"
#include "common.h"
#include "common_utils.h"
#include "pci.h"

extern uint32_t pci_config_read(uint32_t, uint16_t, uint32_t, int);
extern uint32_t pci_domain_config_read(uint32_t, uint32_t, uint16_t, uint32_t, int);
extern uint32_t pci_config_write(uint32_t, uint16_t, uint32_t, int, uint32_t);
extern uint32_t get_ngio_pcie_bus_num (void);

int get_pcie_cap_struct_ptr_with_domain (uint32_t domain, uint32_t bus, uint16_t dev, int fn, uint reg)
{  
    uint32_t val;
    uint32_t offset = reg;
    int get_pci_cap_cnt;
    int max_cnt = 15;
    
    val = pci_domain_config_read(domain, bus, dev, fn, offset);
    val = val & PCI_EXP_CAP_ID_MASK;
    if (val == PCI_EXP_CAP_ID) {
        return val;
    } else {
        offset = val;
        val = pci_domain_config_read(domain, bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            return val;
        } 
    }

    /* 
     * PCI cap pointer should be found within few steps, to avoid incorrect
     * bus number to casue infinite loop, set max loop number is 15     
     */
    for (get_pci_cap_cnt = 1; get_pci_cap_cnt <= max_cnt; get_pci_cap_cnt++) {
        offset += PCI_EXP_CAP_NEXTPTR_OFFSET;
        val = pci_domain_config_read(domain, bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        offset = val;
        val = pci_domain_config_read(domain, bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            break;
        } 

        if (get_pci_cap_cnt == max_cnt) {
            return (FAILED);
        }
    }

    return offset;
}

int get_pcie_link_cap_with_domain (uint32_t domain, uint32_t bus, uint16_t dev, int fn, uint reg)
{
    uint32_t val;
    uint32_t offset = reg;

    /* Get link capability struct - offset + 0xc  */
    offset += PCI_EXP_LINK_CAP_OFFSET;

    /* Read the values - bit 0~9 */
    val = pci_domain_config_read(domain, bus, dev, fn, offset);
    val = val & PCI_EXP_SPD_WID_MASK;

    return val;
}

int get_pcie_link_status_with_domain (uint32_t domain, uint32_t bus, uint16_t dev, int fn, uint reg)
{
    uint32_t val;
    uint32_t offset = reg;

    /* Get link capability struct - offset + 0x12  */
    offset += PCI_EXP_LINK_STATUS_OFFSET;

    /* Read the values - bit 0~9 */
    val = pci_domain_config_read(domain, bus, dev, fn, offset);
    val = val & PCI_EXP_SPD_WID_MASK;

    return val;
}


int get_pcie_cap_struct_ptr (uint32_t bus, uint16_t dev, int fn, uint reg)
{  
    uint32_t val;
    uint32_t offset = reg;
    int get_pci_cap_cnt;
    int max_cnt = 15;
    
    val = pci_config_read(bus, dev, fn, offset);
    val = val & PCI_EXP_CAP_ID_MASK;
    if (val == PCI_EXP_CAP_ID) {
        return val;
    } else {
        offset = val;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            return val;
        } 
    }

    /* 
     * PCI cap pointer should be found within few steps, to avoid incorrect
     * bus number to casue infinite loop, set max loop number is 15     
     */
    for (get_pci_cap_cnt = 1; get_pci_cap_cnt <= max_cnt; get_pci_cap_cnt++) {
        offset += PCI_EXP_CAP_NEXTPTR_OFFSET;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        offset = val;
        val = pci_config_read(bus, dev, fn, offset);
        val = val & PCI_EXP_CAP_ID_MASK;
        if (val == PCI_EXP_CAP_ID) {
            break;
        } 

        if (get_pci_cap_cnt == max_cnt) {
            return (FAILED);
        }
    }

    return offset;
}

int get_pcie_link_cap (uint32_t bus, uint16_t dev, int fn, uint reg)
{
    uint32_t val;
    uint32_t offset = reg;

    /* Get link capability struct - offset + 0xc  */
    offset += PCI_EXP_LINK_CAP_OFFSET;

    /* Read the values - bit 0~9 */
    val = pci_config_read(bus, dev, fn, offset);
    val = val & PCI_EXP_SPD_WID_MASK;

    return val;
}

int get_pcie_link_status (uint32_t bus, uint16_t dev, int fn, uint reg)
{
    uint32_t val;
    uint32_t offset = reg;

    /* Get link capability struct - offset + 0x12  */
    offset += PCI_EXP_LINK_STATUS_OFFSET;

    /* Read the values - bit 0~9 */
    val = pci_config_read(bus, dev, fn, offset);
    val = val & PCI_EXP_SPD_WID_MASK;

    return val;
}

/*
 * Function: pcie_config_read
 *
 * Input .
 * 
 * Output: .
 */
uint
pcie_config_read (uint32_t pcie_port, uint32_t bus, uint16_t dev, uint fn, uint reg)
{
    uint32 data;
    data = pci_config_read(bus, dev, fn, reg);
    return data;
}


/*******************************************************************************
 *
 * Function   :	pcie_conf_read_util
 * Description:	Utility to read PCIe configuration space register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pcie_conf_read_util (void)
{
    uint32_t reg_val = 0, reg_addr = 0;
    uint32_t bus;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);

    bus = get_ngio_pcie_bus_num();
    reg_val = pcie_config_read(0, bus, 0, 0, reg_addr);
    printf("\n\n Value of 0x%05X is 0x%08X.\n\n", reg_addr, reg_val);

    return (PASSED);
}



/*
 * Function: pcie_config_write
 *
 * Input .
 * 
 * Output: .
*/
void
pcie_config_write(uint32_t pcie_port, uint32_t bus, uint16_t dev, uint fn, uint reg, uint32_t val)
{
    pci_config_write(bus, dev, fn, reg, val);
}

/*******************************************************************************
 *
 * Function   :	pcie_conf_write_util
 * Description:	Utility to write PCIe configuration space register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pcie_conf_write_util (void)
{
    uint32_t reg_addr = 0, write_in = 0;
    uint32_t bus;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);
    write_in = gethex_answer("Enter the data:", 0, 0, 0xFFFFFFFF);

    bus = get_ngio_pcie_bus_num();
    pcie_config_write(0, bus, 0, 0, reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}


void config_pcie_rc(uint pcie_port)
{
    printf("configuring pcie root complex: NOT Supported:\n");
}
/************************************************************************
 *
 * Function: get_pci_exp_cap_id
 *
 * Description:	Read the capability id field in a pci capability structure
 *
 * Inputs:
 * bus     - bus number on which the device exists from which we want
 *           to read a configuration data.
 * dev     - device number on that bus.
 * func    - function number of the configuration cycle.
 * addr_base - the location of the data structure in the pci conf space
 *
 * Outputs: Returns the byte in the capability id field
 * 
 *
 ************************************************************************
 */
char
get_pci_exp_cap_id (uint32_t bus, uint16_t dev, uint32_t func, uint32_t addr_base)
{
    uint32_t data;

    data = pci_config_read(bus, dev, func, addr_base+PCI_EXP_CAP_ID_OFFSET /*0x00 */);
    return(data & PCI_EXP_CAP_ID_MASK);
}


/************************************************************************
 *
 * Function: get_next_pci_exp_cap_ptr
 *
 * Description:	Read the next pointer field in a pci capability structure
 *
 * Inputs:
 * bus     - bus number on which the device exists from which we want
 *           to read a configuration data.
 * dev     - device number on that bus.
 * func    - function number of the configuration cycle.
 * addr_base - the location of the data structure in the pci conf space
 *
 * Outputs: Returns the byte in the next pointer
 * 
 *
 ************************************************************************
 */
uchar
get_next_pci_exp_cap_ptr (uint32_t bus, uint16_t dev, uint32_t func, uint32_t addr_base)
{
    uint32_t data;
    uchar next_ptr;

    //    data = pci_config_read(bus, dev, func, addr_base+PCI_EXP_CAP_NEXTPTR_OFFSET);
    data = pci_config_read(bus, dev, func, addr_base);
    next_ptr = (data & PCI_EXP_CAP_NEXTPTR_MASK) >> PCI_EXP_CAP_NEXTPTR_SHIFT;
//    printf("get_next_pci_exp_cap_ptr bus %d, dev %d, func %d, @%#x=%#x\n",
//           bus, dev, func, addr_base, data);

    return(next_ptr);
}

/************************************************************************
 *
 * Function: display_pci_type0_hdr
 *
 * Description:	Display the contents of a PCI type 0 header
 *
 * Inputs:
 * bus     - bus number on which the device exists from which we want
 *           to read a configuration data.
 * dev     - device number on that bus.
 * func    - function number of the configuration cycle.
 *
 * Outputs: Returns the byte in the Capabilities Pointer
 * 
 *
 ************************************************************************
 */
void
display_pci_type0_hdr (uint32_t bus, uint16_t dev, uint32_t func)
{
    uint32 data;
    uchar cap_ptr;

    printf("\n+++ PCI-Compatible Configuration Reg:\n");

    data = pci_config_read(bus, dev, func, 0);
    printf("Device, Vendor ID                 = %#.4x, %#.4x\n",
       (data & 0xffff0000) >> 16, (data & 0xffff));

    data = pci_config_read(bus, dev, func, 0x4);
    printf("Status, Command                   = %#.4x, %#.4x\n",
       (data & 0xffff0000) >> 16, (data & 0xffff));

    data = pci_config_read(bus, dev, func, 0x8);
    printf("Class code                        = %#.6x\n",
           (data & 0xffffff00) >> 8);

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Cache Line Size                   = %#.2x\n",
           (data & 0x000000ff));

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Primary Latency                   = %#.2x\n",
           (data & 0x0000ff00) >> 8);

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Header type                       = %#.2x\n",
           (data & 0xff0000) >> 16);

    data = pci_config_read(bus, dev, func, 0x10);
    printf("BAR0                              = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x14);
    printf("BAR1                              = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x28);
    printf("prefetchable base upper 32 bits   = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x2C);
    printf("prefetchable limit upper 32 bits  = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, PCI_CAP_PTR_OFFSET);
    cap_ptr = data & 0xff;
    printf("Capability pointer                = %#.2x\n", cap_ptr);

}

/************************************************************************
 *
 * Function: display_pci_type1_hdr
 *
 * Description:	Display the contents of a PCI type 1 header (bridge)
 *
 * Inputs:
 * bus     - bus number on which the device exists from which we want
 *           to read a configuration data.
 * dev     - device number on that bus.
 * func    - function number of the configuration cycle.
 *
 * Outputs: Returns the byte in the Capabilities Pointer
 * 
 *
 ************************************************************************
 */
void
display_pci_type1_hdr (uint32_t bus, uint16_t dev, uint32_t func)
{
    uint32_t data;
    uchar cap_ptr;

    printf("\n+++ PCI-Compatible Configuration Reg:\n");

data = pci_config_read(bus, dev, func, 0);
    printf("Device, Vendor ID                 = %#.4x, %#.4x\n",
       (data & 0xffff0000) >> 16, (data & 0xffff));

    data = pci_config_read(bus, dev, func, 0x4);
    printf("Status, Command                   = %#.4x, %#.4x\n",
       (data & 0xffff0000) >> 16, (data & 0xffff));

    data = pci_config_read(bus, dev, func, 0x8);
    printf("Class code                        = %#.6x\n",
           (data & 0xffffff00) >> 8);

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Cache Line Size                   = %#.2x\n",
           (data & 0x000000ff));

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Primary Latency                   = %#.2x\n",
           (data & 0x0000ff00) >> 8);

    data = pci_config_read(bus, dev, func, 0xc);
    printf("Header type                       = %#.2x\n",
           (data & 0xff0000) >> 16);

    data = pci_config_read(bus, dev, func, 0x10);
    printf("BAR0                              = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x14);
    printf("BAR1                              = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x18);
    printf("Primary, 2nd, sub bus             = %#.2x, %#.2x, %#.2x\n",
       data & 0xff, (data & 0xff00) >> 8, (data & 0xff0000) >> 16);

    printf("Secondary latency %#x\n", (data & 0xff000000) >> 24);

    data = pci_config_read(bus, dev, func, 0x1c);
    printf("Secondary status                  = %#.4x\n",
           (data & 0xffff0000) >> 16);

    data = pci_config_read(bus, dev, func, 0x20);
    printf("Memory limit & base               = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, PCI_CAP_PTR_OFFSET);
    cap_ptr = data & 0xff;
    printf("Capability pointer                = %#.2x\n", cap_ptr);
    
    data = pci_config_read(bus, dev, func, 0x28);
    printf("prefetchable base upper 32 bits   = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, 0x2C);
    printf("prefetchable limit upper 32 bits  = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, PCI_BRIDGE_CTRL_OFFSET);
    printf("Bridge ctrl, intr pin, intr line  = %#.4x, %#.2x, %#.2x\n",
       (data & 0xffff0000) >> 16, (data & 0xff00) >> 8, (data & 0xff));

    data = pci_config_read(bus, dev, func, PCI_MSI_CAP_ID_OFFSET);
    printf("MSI: @ %#x = %#x; MSI cap id = %#x; MSI ctl = %#x\n",
           PCI_MSI_CAP_ID_OFFSET, data,
           data & 0x0000FFFF, (data & 0xFFFF0000) >> PCI_EXP_CAP_SHIFT);

    data = pci_config_read(bus, dev, func, PCI_MSI_MSG_ADDR_OFFSET);
    printf("MSI adddress:      @ %#x = %#x\n", PCI_MSI_MSG_ADDR_OFFSET, data);

    data = pci_config_read(bus, dev, func, PCI_MSI_MSG_UPPER_ADDR_OFFSET);
    printf("MSI upper address: @ %#x = %#x\n", PCI_MSI_MSG_UPPER_ADDR_OFFSET,
                                               data);

    data = pci_config_read(bus, dev, func, PCI_MSI_MSG_DATA_OFFSET);
    printf("MSI data address:  @ %#x = %#x\n", PCI_MSI_MSG_DATA_OFFSET, data);

}

/************************************************************************
 *
 * Function: display_pci_exp_cap
 *
 * Description:	Display the contents of PCI express capability structure
 *
 * Inputs:
 * bus     - bus number on which the device exists from which we want
 *           to read a configuration data.
 * dev     - device number on that bus.
 * func    - function number of the configuration cycle.
 * addr_base - the location of the data structure in the pci conf space
 *
 * Outputs: NONE
 * 
 *
 ************************************************************************
 */
static void
display_pci_exp_cap (uint32_t bus, uint16_t dev, uint32_t func,
		     uint32_t addr_base)
{
    uint32_t data;
    uchar next_ptr, exp_cap;


    exp_cap = get_pci_exp_cap_id(bus, dev, func, addr_base);
    if (exp_cap == PCI_MSI_CAP_ID) {
        printf("\n+++ PCI Message Signaling Interrupt");
    } else if (exp_cap == PCI_POWER_MAN_CAP_ID) {
        printf("\n+++ PCI Power Management");
    } else {
        printf("\n+++ PCI Express");
    }
    printf(" Capability:\n");
    printf(" bus %d, dev %d, func %d, addr_base %#x\n",
        bus, dev, func, addr_base);

    data = pci_config_read(bus, dev, func, addr_base+PCI_EXP_CAP_ID_OFFSET); 

    next_ptr = (data & PCI_EXP_CAP_NEXTPTR_MASK) >> PCI_EXP_CAP_NEXTPTR_SHIFT;

    printf("PCIe cap, next ptr, ID = %#.4x, %#.2x, %#.2x,\n",
	   (data & PCI_EXP_CAP_MASK) >> PCI_EXP_CAP_SHIFT, next_ptr,
	   (data & PCI_EXP_CAP_ID_MASK));

    if (exp_cap == PCI_MSI_CAP_ID) {
        data = pci_config_read(bus, dev, func, addr_base +
                                PCI_EXP_MSI_MSG_ADR_OFFSET);
        printf("Message Address        = %#.8x\n", data);
        data = pci_config_read(bus, dev, func, addr_base +
                                PCI_EXP_MSI_MSG_UADR_OFFSET);
        printf("Upper Message Address  = %#.8x\n", data);
        data = pci_config_read(bus, dev, func, addr_base +
                                PCI_EXP_MSI_MSG_DATA_OFFSET);
        printf("Message Data           = %#.8x\n",
            data & PCI_EXP_MSI_MSG_DATA_MASK);
    } else if (exp_cap == PCI_POWER_MAN_CAP_ID) {
        data = pci_config_read(bus, dev, func, addr_base +
                                PCI_EXP_PM_STAT_CTRL_OFFSET);
        printf("Status and Control     = %#.8x\n",
            data & PCI_EXP_PM_STAT_CTRL_MASK);
        printf("Bridge Extension       = %#.8x\n",
            (data & PCI_EXP_PM_CS_BR_EXT_MASK) >> PCI_EXP_PM_CS_BR_EXT_SHIFT);
        printf("Data                   = %#.8x\n",
            (data & PCI_EXP_PM_DATA_MASK) >> PCI_EXP_PM_DATA_SHIFT);
    } else {
        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_DEV_CAP_OFFSET); 
        printf("Device capability      = %#.8x\n", data);

        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_DEV_CTRL_OFFSET); 
        printf("Device status, control = %#.4x, %#.4x\n",
	   (data & PCI_EXP_DEV_STATUS_MASK) >> PCI_EXP_DEV_STATUS_SHIFT,
	   (data & PCI_EXP_DEV_CTRL_MASK));

        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_LINK_CAP_OFFSET); 
        printf("Link capability        = %#.8x\n", data);

        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_LINK_CTRL_OFFSET); 
        printf("Link status, control   = %#.4x, %#.4x\n",
	   (data & PCI_EXP_LINK_STATUS_MASK) >> PCI_EXP_LINK_STATUS_SHIFT,
	   (data & PCI_EXP_LINK_CTRL_MASK));

        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_SLOT_CAP_OFFSET); 
        printf("Slot capability        = %#.8x\n", data);

        data = pci_config_read(bus, dev, func,
				addr_base + PCI_EXP_SLOT_CTRL_OFFSET); 
        printf("Slot status, control   = %#.4x, %#.4x\n",
	       (data & PCI_EXP_SLOT_STATUS_MASK) >> PCI_EXP_SLOT_STATUS_SHIFT,
	       (data & PCI_EXP_SLOT_CTRL_MASK));

        if (bus == 0) {
            /* root complex specific registers */
	    data = pci_config_read(bus, dev, func,
				   addr_base + PCI_EXP_ROOT_CTRL_OFFSET); 
	    printf("Root control           = %#.8x\n",
		   data & PCI_EXP_ROOT_CTRL_MASK);

	    data = pci_config_read(bus, dev, func,
				   addr_base + PCI_EXP_ROOT_STATUS_OFFSET); 
	    printf("Root status            = %#.8x\n", data);
	}
    }

}

/************************************************************************
 *
 * Function: display_pci_exp_ext_cap
 *
 * Description:	Display the contents of PCI express advanced error
 *              reporting capability structure
 *
 * Inputs:
 * bus       - bus number on which the device exists from which we want
 *             to read a configuration data.
 * dev       - device number on that bus.
 * func      - function number of the configuration cycle.
 * addr_base - the location of the data structure in the pci conf space
 *
 * Outputs: NONE
 *
 ************************************************************************
 */
void
display_pci_exp_ext_cap (uint32_t bus, uint16_t dev, uint32_t func,
			 uint32_t addr_base)
{
    uint32_t data;
    uint16_t next_ptr;

    printf("\n+++ PCIe Extended Configuration\n");
    printf(" bus %d, dev %d, func %d, config reg offset %#x\n",
        bus, dev, func, addr_base);

    data = pci_config_read(bus, dev, func, addr_base);
    next_ptr = (data & 0xffff0000) >> 16;
    printf("Next capability offset, advance error reporting = %#.4x, %#.4x\n",
	   next_ptr, (data & 0xffff));

    data = pci_config_read(bus, dev, func, 
			   addr_base + PCI_AER_UNCORR_ERR_STAT_OFFSET); 
    printf("Uncorrectable error status   = %#.8x\n", data);

    data = pci_config_read(bus, dev, func,
			   addr_base + PCI_AER_UNCORR_ERR_MASK_OFFSET); 
    printf("Uncorrectable error mask     = %#.8x\n", data);

    data = pci_config_read(bus, dev, func,
			   addr_base + PCI_AER_UNCORR_ERR_SEV_OFFSET); 
    printf("Uncorrectable error severity = %#.8x\n", data);

    data = pci_config_read(bus, dev, func,
			   addr_base + PCI_AER_CORR_ERR_STAT_OFFSET); 
    printf("Correctable error status     = %#.8x\n", data);

    data = pci_config_read(bus, dev, func,
			   addr_base + PCI_AER_CORR_ERR_MASK_OFFSET); 
    printf("Correctable error mask       = %#.8x\n", data);

    data = pci_config_read(bus, dev, func,
			   addr_base + PCI_AER_CAP_AND_CTRL_OFFSET); 
    printf("Error capability and control = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, addr_base + PCI_AER_HDR_LOG1_OFFSET); 
    printf("Error header log word 1      = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, addr_base + PCI_AER_HDR_LOG2_OFFSET); 
    printf("Error header log word 2      = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, addr_base + PCI_AER_HDR_LOG3_OFFSET); 
    printf("Error header log word 3      = %#.8x\n", data);

    data = pci_config_read(bus, dev, func, addr_base + PCI_AER_HDR_LOG4_OFFSET); 
    printf("Error header log word 4      = %#.8x\n", data);

    /* the following registers are only applicable for the root complex */
    if (bus == 0) {
        data = pci_config_read(bus, dev, func,
			       addr_base + PCI_AER_ROOT_ERR_CMD_OFFSET); 
        printf("Root error command           = %#.8x\n", data);

        data = pci_config_read(bus, dev, func,
			       addr_base + PCI_AER_ROOT_ERR_STAT_OFFSET); 
        printf("Root error status            = %#.8x\n", data);

        data = pci_config_read(bus, dev, func,
			       addr_base + PCI_AER_ERR_SRC_ID_OFFSET); 
        printf("Error source ID, correctable error source ID = %#.4x, %#.4x\n",
	       (data & 0xffff0000)>>16, (data & 0xffff));
    }
}

/* display type0 pci header...this can also be done under linux (use "lspci -vvvv") */
void
display_pci_header (uint bus, unsigned short  dev, uint func)
{
    //    uint data;
    uint vendor_id;
    uchar cap_ptr, next_ptr;

    /* get description of the bridge */
    printf("display_pci_header: bus %d dev %d func %d\n", bus, dev, func);
    /* need to check for endianess */
    vendor_id = pci_config_read(bus, dev, func, 0);

    printf("display_pci_header: vendor id %#x\n", vendor_id);

    /* Display the type 1 header bridge info */
    display_pci_type0_hdr(bus, dev, func);

    /* get first capability pointer */
    next_ptr = cap_ptr = (pci_config_read(bus, dev, func, PCI_CAP_PTR_OFFSET) & 0xFF);

    /* Display the PCI Express capability header;
     * stop when next_ptr = 0
     */
    while (next_ptr != 0) {
        display_pci_exp_cap(bus, dev, func, next_ptr);
        next_ptr = get_next_pci_exp_cap_ptr(bus, dev, func, next_ptr);
    }

    /* Display the PCI Express Extention header */
    display_pci_exp_ext_cap(bus, dev, func, 0x800);

    return;
}


/******** History ********
$Log: linux_pcie.c,v $
Revision 1.7  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.6  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.5.68.2  2017/03/09 02:51:57  leschen
Set counter to get PCI cap pointer to avoid wrong bus to casue infinite loop.

Revision 1.5.68.1  2016/12/15 08:49:36  leschen
Add functions to get pcie cap struct offset, link cap and link status.

Revision 1.5  2013/02/23 07:31:55  ptong
Fixed the problem due to new ROMMON changed the PCIe bus numbering and bumped diag version to 6.3

Revision 1.4  2012/11/12 09:59:53  alpeng
support show pcie speed on overdrive


$Endlog$
*/


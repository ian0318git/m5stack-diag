 /* $Id: plat_defs.h,v 1.2 2019/12/11 10:10:33 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Nanook platform defines.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_

#include "types.h"

#define NANOOK_FPGA_REG_WIDTH    4

/* define sleep seconds */
#define LENGTH100             100
#define LENGTH1000            1000
#define LENGTH1024            1024

#define LENGTH32               32
#define LENGTH64               64 

/* define usb register */
#define USB_PORT_CTRL_BACK		      "0480"
#define USB_PORT_CTRL_TEST_BACK	              "0484"
#define USB_PORT_CTRL_FRONT                   "0490"
#define USB_PORT_CTRL_TEST_FRONT              "0494"


#define NANOOK_NIM_MAX_ETH_PORT   1


/* Common definitions */
#define OVLD_PROCESSOR_NUM      8
#define OVLD_SYS_PRESS_THRE    70
#define OVLD_BUF_SIZE         256
#define OVLD_X86_DIMM_NUM       2
#define GB_X86_DIMM_NUM         1

#define DAGGER_PROCESSOR_NUM      4

/* Platform SKU defines.
 * Old definition: 4441, 42 are Omaha. 4451, 52 are Overlord.
 * New definition: Overlord 4451-X, Juno 4431, Utah 4351, Sword 4331,
 * Dagger 4321, Goldbeach 4221
 */
#define SKU_ISR4321_STR         "ISR4321" /* Dagger */
#define SKU_ISR4331_STR         "ISR4331" /* Sword */
#define SKU_ISR4351_STR         "ISR4351" /* Utah */
#define SKU_ISR4431_STR         "ISR4431" /* Juno */
#define SKU_ISR4451_STR         "ISR4451" /* Overlord */

#define SKU_ISR4461_STR         "ISR4461" /* Neptune */
#define SKU_ISR4452E_STR        "ISR4452E" /* Triton */
#define SKU_ISR4452_STR         "ISR4452" /* Proteus */
#define SKU_ISR4432_STR         "ISR4432" /* Neso */
#define SKU_VG450_STR           "VG450" /* VG450 */

#define SKU_ISR4221_STR         "ISR4221" /* Goldbeach */
#define SKU_ISR4451_STR         "ISR4451"
#define SKU_ISR4221_STR         "ISR4221" /* Goldbeach */

enum {
  SKU_ISR4321,
  SKU_ISR4331,
  SKU_ISR4351,
  SKU_ISR4431,
  SKU_ISR4451,
  SKU_CROSS1,  /* Cross Platform TACHI_ENTRY */
  SKU_CROSS2,  /* Cross Platform TACHI_HIGH */
  SKU_ISR4221,
  SKU_ISR4461,    /* neptune */ 
  SKU_ISR4452E,  /* triton */
  SKU_ISR4452,  /* proteus */
  SKU_ISR4432,  /* neso */
  SKU_VG450,  /* VG450 */
  SKU_ISR9451, /* uranium */
  SKU_ISR9351, /* thorium */
  SKU_CE9331SX, /* radium */
  SKU_CE9331S, /* thallium */
  SKU_INVALID = 0xff
};

/*
 * Platform IO configuration information
 */  
#define MAX_NUM_NGVM_SLOTS    	1
#define MAX_NUM_NGSM_SLOTS	2
#define MAX_NUM_NGWIC_SLOTS	3
#define MAX_CPU_SGMII_PORTS     4

#define MAX_NGIO_LOCAL_GE_PORTS    2
#define MAX_NGIO_LOCAL_XE_PORTS    1
#define MAX_NGIO_LOCAL_ETH_PORTS   (MAX_NGIO_LOCAL_GE_PORTS + MAX_NGIO_LOCAL_XE_PORTS)

#define MAX_NUM_PSU             2
#define MAX_NUM_POE_PSU         2

/* 
 * Platform rootfs information
 */
#define OVLD_VER_FILE "/overlord/x86/version_5.9b"
#define NEP_O2_VER_FILE "/diag_utils/overlord/x86/version_5.9b"
 
/* NGIO module local GE port numbering used in the port mapping
 * table in bcm_gesw_api.c
 * NGIO module local GE port bit mask defines
 */
enum ngio_port_num {
  NGIO_GE0 = 0,
  NGIO_GE1,
  NGIO_XAUI,
};
#define NGIO_GE0_BITMASK     0x1
#define NGIO_GE1_BITMASK     0x2
#define NGIO_XAUI_BITMASK    0x4

/* WIC and SM slot number starts from 1
 */
#define NGWIC1_SLOT		1
#define NGWIC2_SLOT		2
#define NGWIC3_SLOT		3

#define NGSM1_SLOT		1
#define NGSM2_SLOT		2
#define NGSM3_SLOT		3
#define NGSM4_SLOT		4 /* Neptune has a pseudo SM4 which
				     needed to supoort double wide SM3 */

#define NGVM1_SLOT		1

#define SKIP_VM     (1) 
#define SKIP_SM1    (2) 
#define SKIP_SM2    (4) 
#define SKIP_SM3    (8) 
#define SKIP_SM4    (16) 

/* SATA (HDD) number */
#define SATA_NUM_ONE        1
#define SATA_NUM_TWO        2

/* Host CPU GE ports defines
 * O2 and Juno:* Only have port 0-3. port-0 is mgnt port, 1-3 connected to GESW.
 * USD: port 0-2 are front panel GE ports. port-3 is connected to GESW, port-4 (i211 PHY) is mgnt port.
 * Neptune: port-0 (i211 PHY) is mgnt port, 1-2 connects to GESW (port-1 is 10G-KR).
 */
 
#if 0
#define CPU_SGMII_PORT0         0
#define CPU_SGMII_PORT1         1
#define CPU_SGMII_PORT2         2
#define CPU_SGMII_PORT3         3
#define CPU_SGMII_PORT4         4
#define NUMBER_OF_CPU_PHY_PORTS	    1
#define NUMBER_OF_CPU_SW_PORTS	    3
#endif 

/* GESW use these macro
 */
#define TGT_DEV_CPU     0 /* Control Plane CPU */
#define TGT_DEV_NGSM    1
#define TGT_DEV_NGWIC   2
#define TGT_DEV_NGVM    3
#define TGT_DEV_DP      4 /* Data Plane cpu */

#define MGMNT_PORT_OVERLORD 0
#define MGMNT_PORT_JUNO     0
#define MGMNT_PORT_UTAH     4
#define MGMNT_PORT_SWORD    4
#define MGMNT_PORT_DAGGER   4
#define MGMNT_PORT_GOLDBEACH   1

/* Fix the 3 cavecreek sg mii port IP address
 */
#define LOCAL_ETH1_IP_ADDR        "192.123.123.1"
#define LOCAL_ETH2_IP_ADDR        "192.123.123.2"
#define LOCAL_ETH3_IP_ADDR        "192.123.123.3"
#define LOCAL_ETH4_IP_ADDR        "192.123.123.4"
#define LOCAL_DUMMY_IP_ADDR       "192.123.123.123"
#define LOCAL_ETH_NETMASK         "255.255.255.0"
#define LOCAL_ETH_IP_BASE         "192.123.123.0"

#define ETH1_IFCONFIG_STR         "ifconfig eth1 192.123.123.1 netmask 255.255.255.0"
#define ETH2_IFCONFIG_STR         "ifconfig eth2 192.123.123.2 netmask 255.255.255.0"
#define ETH3_IFCONFIG_STR         "ifconfig eth3 192.123.123.3 netmask 255.255.255.0"
#define ETH4_IFCONFIG_STR         "ifconfig eth4 192.123.123.4 netmask 255.255.255.0"

#define HOST_ETH_IP_ADDR          "192.123.123.1"

/* Octeon xaui port IP address used in the data plane diag
 *  */
#define OCTEON_XAUI0_IP_ADDR     "192.123.123.234"
#define NC_CMD_PORT_50           50
#define NC_CMD_PORT_51           51

/* Definition of Test options */
#define QUICK_MODE                1
#define FULL_MODE                 2

/* Overlord PCIe Switch Ports Mapping */
#define OVLD_GE_SW_PCIE_P         0
#define OVLD_FPGA_PCIE_P          1
#define OVLD_NGWIC3_PCIE_P        2
#define OVLD_NGWIC1_PCIE_P        8
#define OVLD_NGWIC2_PCIE_P       10
#define OVLD_NGSM1_PCIE_P        12
#define OVLD_NGSM2_PCIE_P        14
#define OVLD_GLADDEN_PCIE_P      16

/* Juno and Utah with PLX PCIe Switch Ports Mapping */
#define JUNO_UTAH_NGWIC3_PLX_PCIE_P       13
#define JUNO_UTAH_NGWIC1_PLX_PCIE_P        3
#define JUNO_UTAH_NGWIC2_PLX_PCIE_P       11

#define UTAH_NGSM1_PLX_PCIE_P        1
#define UTAH_NGSM2_PLX_PCIE_P        2

/* Sword and Dagger with PLX PCIe Switch Ports Mapping */
#define SWORD_NGWIC1_PLX_PCIE_P      2  
#define SWORD_NGWIC2_PLX_PCIE_P      3 
#define SWORD_NGSM1_PLX_PCIE_P        1
#define DAGGER_NGWIC1_PLX_PCIE_P     1
#define DAGGER_NGWIC2_PLX_PCIE_P     5

/* PLX device id and vendor id */
#define PLX_PCIE_SW_VID       0x10b5  /* vendor id */
#define PLX_PCIE_SW_DID_8618  0x8618  /* device id Juno, Utah*/
#define PLX_PCIE_SW_DID_8617  0x8617  /* device id Sword */
#define PLX_PCIE_SW_DID_8604  0x8604  /* device id dagger */

/* IDT device id and vendor id */
#define IDT_PCIE_SW_VID     0x111d  /* vendor id */
#define IDT_PCIE_SW_DID     0x8090  /* device id */

/* Pericom device id and vendor id */
#define PERICOM_PCIE_SW_VID     0x12d8  /* vendor id */
#define PERICOM_PCIE_SW_DID     0x8619  /* device id */

/* Overlord PCIe bus and device assignment
 * This is determined by ROMMON.
 * ROMMON changed the bus assignment in the FCS version (version 12.2)
 */
#define NGIO_PCIE_BUS_NUM_PREFCS     8
#define NGIO_PCIE_BUS_NUM_FCS        3

#define NGIOWIC1_PCIE_DEV_NUM       8
#define NGIOWIC2_PCIE_DEV_NUM      10
#define NGIOWIC3_PCIE_DEV_NUM       2
#define NGIOSM1_PCIE_DEV_NUM       12
#define NGIOSM2_PCIE_DEV_NUM       14

#define POE_30W_MASK                (1)
#define USB0_MASK                   (1 << 1)
#define USB1_MASK                   (1 << 2)
#define AUX_EXT_MASK                (1 << 3)
#define MSATA_MASK                  (1 << 4)
#define EUSB_MASK                   (1 << 5)

/* Cavium PCIe vendor id and device id */
#define CVMX_VENID     0x177D
#define CN6600_DEVID   0x0092
#define CN7300_DEVID   0x9700

int skip_init_seq;

/* Extern */
extern int usb_dump_x(int);
extern int get_i2c_fd(int);
extern int diag_extend_feature(boolean);
extern int nanook_show_cpuinfo(void);
extern void nanook_show_meminfo(void);
extern uint host_ngio_10gkr_capability (uint, uint);
extern int nanook_show_glory_fpga_ver (int);
extern int crocus_show_fpga_ver (int opt);

#endif                          /* _PLAT_DEFS_H_ */
/*-------------------------------------------------
 * $Log: plat_defs.h,v $
 * Revision 1.2  2019/12/11 10:10:33  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

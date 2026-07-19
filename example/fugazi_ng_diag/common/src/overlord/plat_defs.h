/* $Id: plat_defs.h,v 1.48 2020/05/22 02:28:34 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - Overlord platform defines.
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_

/* Common definitions */
#define OVLD_PROCESSOR_NUM      8
#define OVLD_SYS_PRESS_THRE    70
#define OVLD_BUF_SIZE         256
#define OVLD_X86_DIMM_NUM       2
#define GB_X86_DIMM_NUM         1

#define DAGGER_PROCESSOR_NUM      4

/*Curie 1RU CPU critical info
 */
#define RADIUM_CORE_NUM           8
#define RADIUM_PROC_NUM           (RADIUM_CORE_NUM*2)
#define THALLIUM_CORE_NUM         8
#define THALLIUM_PROC_NUM         (THALLIUM_CORE_NUM*2)
#define POLONIUM_CORE_NUM         4
#define POLONIUM_PROC_NUM         (POLONIUM_CORE_NUM*2)
/* Cruie 1RU P1A and P1B proto use the early cpu version with
 * "Genuine" string.
 * P1C uses the production version with "Xeon" string.
 */
#define BROADWELL_GENUINE_2_70GHZ_CPU     "Genuine Intel(R) CPU @ 2.70GHz"
#define BROADWELL_GENUINE_2_00GHZ_CPU     "Genuine Intel(R) CPU @ 2.00GHz"
#define BROADWELL_XEON_2_70GHZ_CPU     "Intel(R) Xeon(R) CPU D-1573N @ 2.70GHz"
#define BROADWELL_XEON_2_00GHZ_CPU     "Intel(R) Xeon(R) CPU D-1563N @ 2.00GHz"

/*Curie 2RU CPU critical info
 */
#define URANIUM_CORE_NUM          12
#define URANIUM_PROC_NUM          (URANIUM_CORE_NUM*2)
#define THORIUM_CORE_NUM          8
#define THORIUM_PROC_NUM          (THORIUM_CORE_NUM*2)
#define SKYLAKE_GENUINE_2_00GHZ_CPU     "Genuine Intel(R) CPU @ 2.00GHz"
#define SKYLAKE_2_60GHZ_CPU       "Intel(R) Xeon(R) D-2168NT CPU @ 2.60GHz"
#define SKYLAKE_2_30GHZ_CPU       "Intel(R) Xeon(R) D-2148NT CPU @ 2.10GHz"

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

#define SKU_C8300_2N2S_4G2X_STR "C8300-2N2S-4G2X" /* uranium */
#define SKU_C8300_2N2S_6G_STR   "C8300-2N2S-6G"   /* thorium */
#define SKU_C8300_2N2S_4T2X_STR "C8300-2N2S-4T2X" /* uranium */
#define SKU_C8300_2N2S_6T_STR   "C8300-2N2S-6T"   /* thorium */
#define SKU_C8300_1N1S_4T2X_STR "C8300-1N1S-4T2X" /* radium */
#define SKU_C8300_1N1S_6T_STR   "C8300-1N1S-6T"   /* thallium */

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
  SKU_C8300_2N2S_4T2X, /* uranium */
  SKU_C8300_2N2S_6T,   /* thorium */
  SKU_C8300_1N1S_4T2X, /* radium */
  SKU_C8300_1N1S_6T,   /* thallium */
  SKU_VG400, 
  SKU_INVALID = 0xff
};

/*
 * Platform eth/sfp configuration information
 */  
#define MAX_NEP_NUM_SFP    4 /* SFP0, SFP1; SFP+ 0, SFP+ 1 */

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


#define CURIE_NIM_MAX_ETH_PORT   1

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

/* Switzer-carrier adapter card is a SM which has two WIC slots: */
/* Sub-WIC1 - 1, Sub-WIC2 - 2 */
#define NGSM_WIC1_SLOT  1
#define NGSM_WIC2_SLOT  2


#define NGVM1_SLOT		1

#define SKIP_VM     (0x1) 
#define SKIP_SM1    (0x10) 
#define SKIP_SM2    (0x100) 
#define SKIP_SM3    (0x1000) 
#define SKIP_SM4    (0x10000) 
#define SKIP_NIM1   (0x100000)
#define SKIP_NIM2   (0x1000000)

/* SATA (HDD) number */
#define SATA_NUM_ONE        1
#define SATA_NUM_TWO        2

/* Host CPU GE ports defines
 * O2 and Juno:* Only have port 0-3. port-0 is mgnt port, 1-3 connected to GESW.
 * USD: port 0-2 are front panel GE ports. port-3 is connected to GESW, port-4 (i211 PHY) is mgnt port.
 * Neptune: port-0 (i211 PHY) is mgnt port, 1-2 connects to GESW (port-1 is 10G-KR).
 */
#define CPU_SGMII_PORT0         0
#define CPU_SGMII_PORT1         1
#define CPU_SGMII_PORT2         2
#define CPU_SGMII_PORT3         3
#define CPU_SGMII_PORT4         4
#define CPU_SGMII_PORT5         5
#define CPU_SGMII_PORT6         6
#define CPU_SGMII_PORT7         7
#define CPU_SGMII_PORT8         8
#define CPU_SGMII_PORT9         9
#define CPU_SGMII_PORT10        10
#define CPU_SGMII_PORT11        11
#define NUMBER_OF_CPU_PHY_PORTS	    1
#define NUMBER_OF_CPU_SW_PORTS	    3

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
 */
#define OCTEON_XAUI0_IP_ADDR     "192.123.123.234"
#define NC_CMD_PORT_50           50
#define NC_CMD_PORT_51           51

/* Definition of Test options */
#define QUICK_MODE                1
#define FULL_MODE                 2

/* Curie 1RU CPU PCIe bus to NGIO */ 
#define CURIE_1RU_NGIO_PCIE_BUS_NUM  (0x78) 

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
#define NGIOSMNIM1_PCIE_DEV_NUM    7
#define NGIOSMNIM2_PCIE_DEV_NUM    9

#define POE_30W_MASK                (1)
#define USB0_MASK                   (1 << 1)
#define USB1_MASK                   (1 << 2)
#define AUX_EXT_MASK                (1 << 3)
#define MSATA_MASK                  (1 << 4)
#define EUSB_MASK                   (1 << 5)
#define DIMM_1_MASK                 (1 << 6)

/* Cavium PCIe vendor id and device id */
#define CVMX_VENID     0x177D
#define CN6600_DEVID   0x0092
#define CN7300_DEVID   0x9700

typedef struct ovld_dev_info {
    char  *name;
    unsigned int mask; 
} ovld_dev_info_t;

extern int get_mgmnt_port(void);
extern int is_plat_10gkr_capable(void);
extern int is_bcm (int);
extern uint host_ngio_10gkr_capability (uint, uint);
extern int cfg_10gkr_port(int port, int en_10gkr);

#endif  /* _PLAT_DEFS_H_ */

/******** History ******** 
$Log: plat_defs.h,v $
Revision 1.48  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.47  2020/01/31 07:17:40  leschen
Support Curie new cookie.

Revision 1.46  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.45  2019/12/31 07:48:06  alpeng
revert cookie to old one for RDT

Revision 1.44  2019/12/21 00:52:37  ptong
Curie PID change to C8300-1N1S-4T2X and C8300-1N1S-6G

Revision 1.43  2019/08/26 03:36:11  alpeng
CSCvq64781 - dimm1 is optional, provide dimm1 option arg for MFG

Revision 1.42  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.41  2019/06/26 08:49:42  alpeng
support side band signal test for neptune; remove local intr check for sfp, since fpga is not support anymore.

Revision 1.40  2018/08/30 06:59:55  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.39  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.38  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.37  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.36.6.15  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.36.6.14  2017/09/28 08:16:25  leschen
Support VG450

Revision 1.36.6.13  2017/08/11 02:41:03  leschen
Support Neptune SM4 slot.

Revision 1.36.6.12  2017/08/02 09:59:41  leschen
Update Neptune PID from ISR4462 to ISR4461.

Revision 1.36.6.11  2017/04/05 06:45:03  leschen
Sync with <ng_diag-tag-032917>

Revision 1.36.6.10  2017/03/24 22:25:23  ptong
Merge new ACT2 code from main trunk that allow chassis SN tobe used in SUDI/WDC. Increase version to V1.1.4

Revision 1.36.6.9  2017/03/13 08:27:30  leschen
Support latest Neptune/Triton/Proteus/Neso PID.

Revision 1.36.6.8  2017/03/07 02:07:36  alpeng
update ISR num to 4462 for neptune

Revision 1.36.6.7  2017/01/23 10:36:52  alpeng
update ngio slot info for triton, proteus and neso

Revision 1.36.6.6  2017/01/04 08:32:38  alpeng
update sku num for neptune

Revision 1.36.6.5  2016/12/13 00:23:40  ptong
Added GESW port list util, host port send pkt to GESW test support for Neptune

Revision 1.36.6.4  2016/11/01 11:36:16  alpeng
support mbox for neptune

Revision 1.36.6.3  2016/10/10 17:02:41  alpeng
support NIM module for neptune

Revision 1.36.6.2  2016/06/10 18:25:38  ptong
Enhance GESW support to Neptune

Revision 1.36.6.1  2016/06/01 23:16:18  jskow
Update plat_defs.h

Revision 1.38  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.37  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.36  2016/03/04 19:19:15  ptong
Clean up obsolete ISR platfrom PID

Revision 1.35  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.34  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.33  2014/04/18 00:18:50  ptong
Prepare to support Greyhound 10G-KR bring-up

Revision 1.32  2014/03/13 20:32:43  ptong
Use table to map BCM GE ports to NGIO ports. Prepare for migration to use the Greyhound GESW

Revision 1.31  2014/02/10 09:08:22  hroni
add MSATA_MASK and EUSB_MASK

Revision 1.30  2014/01/28 02:40:35  ptong
Host SGMII port to GE switch use a fix IP address of 192.123.123.1 to support NGIO module code

Revision 1.29  2013/12/24 05:57:35  hroni
rename AUX_MASK to AUX_EXT_MASK

Revision 1.28  2013/12/23 04:22:18  hroni
add force skip usb and aux mask

Revision 1.27  2013/12/21 01:35:59  ptong
Dagger CPU only has 4 cores

Revision 1.26  2013/11/19 13:31:03  danchung
Add pcie port mapping for Sword NGSM slot 1

Revision 1.25  2013/11/18 07:27:43  alpeng
support get_pci_bus_num()

Revision 1.24  2013/11/01 13:22:16  danchung
Fix testcard failure on Utah.

Revision 1.23  2013/10/22 14:32:34  danchung
Add support for PLX PCIe switch

Revision 1.22  2013/10/16 04:20:54  hroni
do arping and tftp retry for max: 3 times if previous tftp download was failed

Revision 1.21  2013/10/16 03:46:29  hroni
add get_mgmnt_port() to provide the correct port for each platform. 

Revision 1.20  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.19  2013/09/05 01:58:27  alpeng
support mSATA test on Utah

Revision 1.18  2013/08/13 00:07:00  hroni
support Rangeley control plane SGMII to GE same port loopback test

Revision 1.17  2013/08/09 00:33:00  hroni
Utah uses SGMII port#4 for eth and xaui loopback test

Revision 1.16  2013/07/23 04:17:33  alpeng
update SKU num for Juno, Utah and Sword

Revision 1.15  2013/07/09 09:49:10  alpeng
moving function is_platform() related to dash_fpga.c

Revision 1.14  2013/07/03 23:47:55  ptong
Added macro to support checking FPGA board subtype register for Utah platform

Revision 1.13  2013/05/31 12:51:27  danchung
Add checking board type for Juno.

Revision 1.12  2013/03/17 02:04:13  mcharon
support command line for testing 30w poe

Revision 1.11  2013/02/23 07:31:55  ptong
Fixed the problem due to new ROMMON changed the PCIe bus numbering and bumped diag version to 6.3

Revision 1.10  2012/09/19 22:30:40  palin2
Add I2C scan test support those I2C devices that are connected to Cavecreek.

Revision 1.9  2012/09/19 07:27:54  palin2
Add Overlord PCIe switch port mapping definitions.

Revision 1.8  2012/09/18 07:47:19  palin2
Add function to check system pressure in Diag boot-up process.

Revision 1.7  2012/08/18 00:01:13  ptong
Use official SKU for PID checking

Revision 1.6  2012/06/26 22:09:54  ptong
Update the SKU PID string

Revision 1.5  2012/05/27 22:28:24  ptong
Add Overlord SKU string macro

Revision 1.4  2012/04/17 14:13:58  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.3  2012/04/11 21:27:15  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/

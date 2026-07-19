/* $Id: diag_esw_lib.c,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_lib.c - ESW functions library
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proto.h"
#include "common.h"
#include "types.h"
#include "ethernet.h"
#include "diag_esw_lib.h"
#include "diag_esw_test.h"
#include "dnv_eth_lib.h"
#include "dev_98dxc323.h"
#include "dev_88e1680.h"
#include "diag_fpga.h"
#include "diag_eth_pkt_txrx.h"
#include "nvmonvars.h"
#include "linux_pciutils.h"
#include "madApi.h"
#include "madHwCntl.h"
#include "dnv_eth_lib.h"
#include "diag_common.h"
#include "queryflags.h"


//#define DEBUG 0

/* workaround to avoid build error. BIT_0 to BIT_31 are defined in common.h.
   And Marvell code defines them again.
*/
#undef BIT_0
#undef BIT_1
#undef BIT_2
#undef BIT_3
#undef BIT_4
#undef BIT_5
#undef BIT_6
#undef BIT_7
#undef BIT_8
#undef BIT_9
#undef BIT_10
#undef BIT_11
#undef BIT_12
#undef BIT_13
#undef BIT_14
#undef BIT_15
#undef BIT_16
#undef BIT_17
#undef BIT_18
#undef BIT_19
#undef BIT_20
#undef BIT_21
#undef BIT_22
#undef BIT_23
#undef BIT_24
#undef BIT_25
#undef BIT_26
#undef BIT_27
#undef BIT_28
#undef BIT_29
#undef BIT_30
#undef BIT_31

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <generic/cpssTypes.h>
#include <generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#include <cpss/generic/version/cpssGenVersion.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include "nim_dm_cpss_extserv.h"
#include "nim_dm_cpss_extdrv.h"
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include<cpss/generic/log/cpssLog.h>

/*
 * Local Functions
 */

#if 0

static MAD_SEM madOsSemCreate (MAD_SEM_BEGIN_STATE);
static MAD_STATUS madOsSemDelete (MAD_SEM);
static MAD_STATUS madOsSemWait (MAD_SEM, MAD_U32);
static MAD_STATUS madOsSemSignal (MAD_SEM);

#endif 


int smi_read_reg (unsigned int, unsigned int, unsigned short *);
int smi_write_reg (unsigned int, unsigned int, unsigned short);
static unsigned int phy_smi_read (MAD_DEV *, unsigned int, unsigned int, unsigned int *);
static unsigned int phy_smi_write (MAD_DEV *, unsigned int, unsigned int, unsigned int);
static int phy_read_reg (MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U32 *);
static int phy_write_reg (MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U16);
static char * phy_get_dev_name ( MAD_DEVICE_ID);
static int phy_mad_load_driver (MAD_DEV *, int);
static int phy_mad_unload_driver (MAD_DEV *);
static int phy_mad_disable_int (MAD_DEV *);
static int phy_mad_display_reg (MAD_DEV *, MAD_U8, MAD_U16);
static int phy_mad_soft_reset (MAD_DEV *, MAD_LPORT);
static int phy_mad_set_test_mode (MAD_DEV *, uint, uint);
static int phy_mad_set_phy_enable (MAD_DEV *, MAD_LPORT, int);
static int phy_mad_hw_page_reset (MAD_DEV *, MAD_U8, MAD_U16);
static int xcat3_global_enable_pve (uint);
static int xcat3_global_disable_pve (uint);
static int xcat3_set_port_pve (uint, uint, uint);
static int xcat3_clear_port_pve (uint, uint, uint);
static int xcat3_set_port_pve_single_direction (uint, uint, uint);
static int xcat3_clear_port_pve_single_direction (uint, uint, uint);
static int xcat3_port_enable (uint, uint);
static int xcat3_port_disable (uint, uint);
static int xcat3_port_mac_loopback_enable (uint, uint);
static int xcat3_port_mac_loopback_disable (uint, uint);
static int xcat3_soft_reset (uint);
static int xcat3_vlan_add (uint, uint, uint);
static int xcat3_vlan_port_add (uint, uint, uint);
static int xcat3_vlan_port_del (uint, uint, uint);
static int xcat3_vlan_port_show (uint, uint);
static int xcat3_force_link_down_en (uint, uint, boolean);
static int xcat3_force_link_pass_en (uint, uint, boolean);
static int xcat3_port_force_link_set (uint, uint, int, boolean);
static int xcat3_all_reg_test (uint, uint*, uint*, uint*, uint*);
static int xcat3_pcie_config_read (int, uint *);
static int xcat3_pcie_config_write (int, uint);
static int xcat3_led_class_config (uint, uint32_t, boolean, boolean, int, boolean, uint32_t);
static int xcat3_led_init (uint);
static int xcat3_smi_phy_init (uint);
static int xcat3_reg_pci_read (uint, uint, uint*);
static int xcat3_reg_pci_write (uint, uint, uint);
static int xcat3_reg_config_read (uint, uint, uint *);
static int xcat3_reg_config_write (uint, uint, uint);

static int xcat3_port_init (uint, uint);
static int xcat3_specific_port_init (uint);
static int xcat3_specific_port_enable (uint);
static int xcat3_port_phy_1680_init (void);
static int xcat3_cpss_pp_phase1_info_init (uint);
static int xcat3_cpss_pp_phase2_info_init (uint);
static int xcat3_cpss_driver_init (uint);
static int xcat3_cpss_device_init (uint);
static int xcat3_cpss_set_test_mode(uint, uint, uint, uint);
static int xcat3_serdes_tx_config_read(void);
static int xcat3_serdes_tx_config_write (void);
static int xcat3_phy_tx_config_read (void);
static int xcat3_phy_tx_config_write (void);
unsigned long nanook_get_pci_base_addr (void);
int diag_esw_ext_lpbk_test(void); 
int diag_esw_98dxc323_dev_create(dev_98dxc323_object_t *);
int diag_phy_88e1680_dev_create (dev_88e1680_object_t *);
int diag_esw_init(void);
int diag_esw_exit (void);
int diag_reset_esw_to_default(int);
int diag_config_port_speed (uint, uint, uint);
int diag_port_power_control(uint, uint, uint);
void diag_esw_remove_pcie_device (void);
int diag_esw_all_phy_led_on (void);
int diag_esw_all_phy_led_off (void);



/*
 * External Functions
 */
extern GT_STATUS cpssDxChPortPcsResetSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
);

extern GT_STATUS hwPpStartInit
(
    IN  GT_U8                       devNum,
    IN  CPSS_REG_VALUE_INFO_STC     *initDataListPtr,
    IN  GT_U32                      initDataListLen
);

extern GT_STATUS cpssDxChBrgVlanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

extern GT_STATUS internal_cpssDxChHwPpPhase1Init_new
(
    IN      CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *ppPhase1ParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
);

extern void testMADDisplayStatus(MAD_STATUS status);
extern uint32_t pci_config_read(uint32_t, uint16_t, uint32_t, int);
extern uint32_t pci_config_write(uint32_t, uint16_t, uint32_t, int, uint32_t);
extern uint32 err_report(dev_object_t *, char *, uint32);


/*
 * Global variables
 */

/* Dummy for competability, for init process. */
#define GT_DUMMY_REG_VAL_INFO_LIST          \
{                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},\
    {0x00000001, 0x00000000, 0x00000000, 0},\
    {0x00000002, 0x00000000, 0x00000000, 0},\
    {0x00000003, 0x00000000, 0x00000000, 0},\
    {0x00000004, 0x00000000, 0x00000000, 0},\
    {0x00000005, 0x00000000, 0x00000000, 0},\
    {0x00000006, 0x00000000, 0x00000000, 0},\
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter */      \
}


int phy_dev_88e1680_group_start_addr[NANOOK_1680_GROUP_NUM] = {NANOOK_1680_GROUP_0_START_ADDR, NANOOK_1680_GROUP_1_START_ADDR, NANOOK_1680_GROUP_2_START_ADDR};

MAD_DEV phy_dev_88e1680[3];
int marvell_cpssPpInit_xcat3 = FALSE;
int marvell_ac3_cpss_dev_num_nanook = NANOOK_AC3_CPSS_DEV;
static CPSS_PP_DEVICE_TYPE xcat3_pp_dev_type;
static CPSS_DXCH_PP_PHASE1_INIT_INFO_STC xcat3_pp_phase1_info;
static CPSS_DXCH_PP_PHASE2_INIT_INFO_STC xcat3_pp_phase2_info;

static CPSS_REG_VALUE_INFO_STC dummyRegValInfoList[] = GT_DUMMY_REG_VAL_INFO_LIST;

#if 0
/******************************************************************************
 *
 * Function   :	madOsSemCreate
 * Description: madOsSemCreate
 * Inputs     :	state - sem begin state
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static MAD_SEM 
madOsSemCreate (MAD_SEM_BEGIN_STATE state)
{
    return MAD_OK;
}


/******************************************************************************
 *
 * Function   :	madOsSemDelete
 * Description: madOsSemDelete
 * Inputs     :	smid - sem ID
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static MAD_STATUS
madOsSemDelete (MAD_SEM smid)
{
    return MAD_OK;
}


/******************************************************************************
 *
 * Function   :	madOsSemWait
 * Description: madOsSemWait
 * Inputs     :	smid - sem ID
 *                  timeout- wait time 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static MAD_STATUS
madOsSemWait (MAD_SEM smid, MAD_U32 timeout)
{
    return MAD_OK;
}


/******************************************************************************
 *
 * Function   :	madOsSemSignal
 * Description: madOsSemSignal
 * Inputs     :	smid - sem ID
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static MAD_STATUS
madOsSemSignal (MAD_SEM smid)
{
    return MAD_OK;
}


#endif

/******************************************************************************
 *
 * Function   :	smi_read_reg
 * Description:	read from device register through SMIinterface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              rd_data - point to unsigned short which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi_read_reg (unsigned int port_num, unsigned int reg_addr, unsigned short *reg_data)
{
    return (cpssDxChPhyPortSmiRegisterRead(marvell_ac3_cpss_dev_num_nanook, port_num, reg_addr, reg_data));
}


/******************************************************************************
 *
 * Function   :	smi_write_reg
 * Description:	write to device register through SMI interface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              wr_data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi_write_reg (unsigned int port_num, unsigned int reg_addr, unsigned short wr_data)
{
    return (cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, reg_addr, wr_data));
}


/******************************************************************************
 *
 * Function   :	phy_smi_read
 * Description:	read PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              rd_data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int 
phy_smi_read (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int *reg_data)
{
    unsigned int port_num;
    port_num = smi_addr;
	
    if (smi_read_reg(port_num, reg_addr, (unsigned short *)reg_data) == PASSED) {
	return MAD_TRUE;
    } else {
	return MAD_FALSE;
    }
}

/******************************************************************************
 *
 * Function   :	phy_smi_write
 * Description:	write PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              data - data to write to PHY regiter
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int 
phy_smi_write (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int data)
{
    unsigned int port_num;
    port_num = smi_addr;

    if (smi_write_reg(port_num, reg_addr, (unsigned short)data) == PASSED) {
	return MAD_TRUE;
    } else {
	return MAD_FALSE;
    }
}


/******************************************************************************
 *
 * Function   :	phy_read_reg
 * Description:	read a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
phy_read_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U32 *data)
{
    MAD_STATUS status;

    status=mdSysGetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status==MAD_OK) {
      *data &= 0xffff; 
      return PASSED;
    } else {
        testMADDisplayStatus(status);
	return FAILED;
    }
}


/******************************************************************************
 *
 * Function   :	phy_write_reg
 * Description:	write to a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
phy_write_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U16 data)
{
    MAD_STATUS status;

    status=mdSysSetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status==MAD_OK) {
	return PASSED;
    } else {
	testMADDisplayStatus(status);
	return FAILED;
    }
}


/******************************************************************************
 *
 * Function   :	phy_get_dev_name
 * Description:	get PHY device name from device ID.
 * Inputs     :	device_id
 * Outputs    : device name
 *
 ******************************************************************************
 */
static char * phy_get_dev_name ( MAD_DEVICE_ID device_id)
{
    switch (device_id) {
    case MAD_88E10X0: return ("MAD_88E10X0 ");   	
    case MAD_88E10X0S: return ("MAD_88E10X0S ");   
    case MAD_88E1011: return ("MAD_88E1011 ");   
    case MAD_88E104X: return ("MAD_88E104X ");
    case MAD_88E1111: return ("MAD_88E1111/MAD_88E1115 ");
    case MAD_88E1112: return ("MAD_88E1112 ");
    case MAD_88E1116: return ("MAD_88E1116/MAD_88E1116R ");
    case MAD_88E114X: return ("MAD_88E114X ");
    case MAD_88E1149: return ("MAD_88E1149 ");
    case MAD_88E1149R: return ("MAD_88E1149R ");
    case MAD_SWG65G : return ("MAD_SWG65G ");
    case MAD_88E1181: return ("MAD_88E1181 ");
    case MAD_88E3016: return ("MAD_88E3015/MAD_88E3016/MAD_88E3018/MAD_88E3019");	
    case MAD_88E1121: return ("MAD_88E1121/MAD_88E1121R ");
    case MAD_88E3082: return ("MAD_88E3082/MAD_88E3083 ");
    case MAD_88E1240: return ("MAD_88E1240 ");
    case MAD_88E1340S: return ("MAD_88E1340S ");
    case MAD_88E1340: return ("MAD_88E1340 ");
    case MAD_88E1340M: return ("MAD_88E1340M ");
    case MAD_88E1119R: return ("MAD_88E1119R ");
    case MAD_88E1310: return ("MAD_88E1310 ");
    //case MAD_88E1510: return ("MAD_88E1510 ");
    case MAD_88E1540: return ("MAD_88E1540 ");
    case MAD_88E1548: return ("MAD_88E1548 ");
    case MAD_88E1680: return ("MAD_88E1680 ");	
    case MAD_88E1680M: return ("MAD_88E1680M ");
    case MAD_SW1680: return ("MAD_SW1680 ");
    //case MAD_88E3183: return ("MAD_88E3183 ");
    default : return (" No-name ");
    }
} ;


/******************************************************************************
 *
 * Function   :	phy_mad_load_driver
 * Description:	load PHY MAD driver
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 * Outputs    : PASSED(0)/FAILED(1)
 *
 ******************************************************************************
 */
static int
phy_mad_load_driver (MAD_DEV *dev, int smi_addr)
{
    MAD_SYS_CONFIG cfg;
    MAD_STATUS status;

    /* clear structures */
    memset((char*)&cfg,0,sizeof(MAD_SYS_CONFIG));
    memset((char*)dev,0,sizeof(MAD_DEV));

    /*
     *  Register all the required functions to MAD driver.
     */
    cfg.BSPFunctions.readMii   = phy_smi_read;
    cfg.BSPFunctions.writeMii  = phy_smi_write;
#if 0
    cfg.BSPFunctions.semCreate = madOsSemCreate;
    cfg.BSPFunctions.semDelete = madOsSemDelete;
    cfg.BSPFunctions.semTake   = madOsSemWait;
    cfg.BSPFunctions.semGive   = madOsSemSignal;
#endif 
    
    cfg.BSPFunctions.semCreate = NULL;
    cfg.BSPFunctions.semDelete = NULL;
    cfg.BSPFunctions.semTake   = NULL;
    cfg.BSPFunctions.semGive   = NULL;


    cfg.smiBaseAddr = smi_addr;  /* Set SMI Address */

    if((status=mdLoadDriver(&cfg, dev)) != PASSED)
    {
        cterr('f',0,"madLoadDriver return Failed, status = %#x", status);
	return status;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("Device Name   : %s\n", phy_get_dev_name(dev->deviceId));
	printf("Device ID     : 0x%x\n",dev->deviceId);
	printf("Revision      : 0x%x\n",dev->revision);
	printf("Base Reg Addr : 0x%x\n",dev->baseRegAddr);
	printf("No of Ports   : %d\n",dev->numOfPorts);
	printf("MAD has been started.\n");
    }
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	phy_mad_unload_driver
 * Description: unload mad driver
 * Inputs     :	dev - point to MAD_DEV
 * Outputs    : PASSED(0)/FAILED(1)
 *
 ******************************************************************************
 */
static int 
phy_mad_unload_driver (MAD_DEV *dev)
{
    return (mdUnloadDriver(dev));
}


/******************************************************************************
 *
 * Function   :	phy_mad_disable_int
 * Description:	disable all the interrupt
 * Inputs     :	dev - point to MAD_DEV
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
phy_mad_disable_int (MAD_DEV *dev)
{
    MAD_STATUS status;
    MAD_LPORT port;
    MAD_INT_TYPE int_type;

    /* clear out all int causes */
    memset(&int_type, 0, sizeof(MAD_INT_TYPE));

    for(port=0; port<dev->numOfPorts; port++) {
        if((status = mdIntSetEnable(dev,port,&int_type)) != MAD_OK) {
            testMADDisplayStatus(status);
            printf("mdIntSetEnable returned fail.\n");
            return FAILED;
        }
    }

    return PASSED;
}


/******************************************************************************
 *
 * Function   :	phy_display_reg
 * Description:	diaplay a page of registers for a specific port.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int  
phy_mad_display_reg (MAD_DEV *dev, MAD_U8 port_num, MAD_U16 page_num)
{
    MAD_STATUS status;
    int i;
    MAD_U16 data;

    if (dev == 0) {
        printf("MAD driver is not initialized.\n");
        return FAILED;
    }

    printf("Read PHY port %d page %d : \n", (int)port_num, (int)page_num);

    for (i=0; i<32; i++) {
	if((status = madHwReadPagedPhyReg(dev,port_num,page_num, i, &data)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"Reading page %d  port %d register %d failed.", 
		  (int)page_num, (int)port_num, i);
	    return FAILED;
	}

	if ((i+1)%4)
	    printf("reg %02d: 0x%04x    ", i, (int)data);
	else
	    printf("reg %02d: 0x%04x\n", i, (int)data);
    }

    printf("\n");
    
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	phy_mad_soft_reset
 * Description:	soft reset a specific PHY port.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
phy_mad_soft_reset (MAD_DEV *dev, MAD_LPORT port_num) 
{
    MAD_STATUS status;

    status = mdSysSoftReset(dev, port_num);

    if (status==MAD_OK) {
	printf("Soft reset for port %d.\n", (int)port_num);
	return PASSED;
    } else {
        testMADDisplayStatus(status);
	return FAILED;
    }
}


/**********************************************************************
 *
 * Function: phy_mad_set_test_mode
 *
 * Description: This function provides PHY test mode for Marvell GE PHY.
 *
 * Input: dev - point to MAD_DEV
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
phy_mad_set_test_mode (MAD_DEV *dev, uint port, uint test_mode)
{

    MAD_STATUS status;

    assert(dev);

    if (test_mode > 0) {
	if((status = mdDiagSetIEEETest(dev, port, ENABLE, test_mode)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"mdDiagSetIEEETest returned fail.");
	    return FAILED;
	}
    } else {
	/* go back to normal mode */
	if((status = mdDiagSetIEEETest(dev, port, DISABLE, 1)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"mdDiagSetIEEETest returned fail.");
	    return FAILED;
	}
    }

    return PASSED;
}


/**********************************************************************
 *
 * Function: phy_mad_set_phy_enable
 *
 * Description: set phy enable 
 *
 * Input: dev - point to MAD_DEV
 *           port_num - PHY port number 
 *           status - enable/disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
phy_mad_set_phy_enable (MAD_DEV *dev, MAD_LPORT port_num, int status)
{
    return (mdSysSetPhyEnable(dev, port_num, status));
}


/**********************************************************************
 *
 * Function: phy_mad_hw_page_reset
 *
 * Description: reset PHY page
 *
 * Input: dev - point to MAD_DEV
 *           port_num - PHY port number 
 *           page_num - page number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
phy_mad_hw_page_reset (MAD_DEV *dev, MAD_U8 port_num, MAD_U16 page_num)
{
    return (madHwPagedReset(dev, port_num, page_num));
}


/**********************************************************************
 *
 * Function: xcat3_global_enable_pve
 *
 * Description: Enable xCat3 global pve
 *
 * Input: dev_num - cpss dev number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_global_enable_pve (uint dev_num)
{
    uint32_t rc = 0;

    rc  = cpssDxChBrgPrvEdgeVlanEnable(dev_num, GT_TRUE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x\n", rc);
    }
    
    return rc;
} 


/**********************************************************************
 *
 * Function: xcat3_global_disable_pve
 *
 * Description: Disable xCat3 global pve
 *
 * Input: dev_num - cpss dev number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_global_disable_pve (uint dev_num)
{
    uint32_t rc = 0;

    rc  = cpssDxChBrgPrvEdgeVlanEnable(dev_num, GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x\n", rc);
    }
    
    return rc;
} 


/**********************************************************************
 *
 * Function: xcat3_set_port_pve
 *
 * Description: Set xCat3 port pve
 *
 * Input: dev_num - cpss dev number
 *           src_port - source port
 *           dst_port - destination port
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_set_port_pve (uint dev_num, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_TRUE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          dst_port,
                                          GT_TRUE,
                                          src_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    return rc; 
}


/**********************************************************************
 *
 * Function: xcat3_clear_port_pve
 *
 * Description: Clear xCat3 port pve
 *
 * Input: dev_num - cpss dev number
 *           src_port - source port
 *           dst_port - destination port
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_clear_port_pve (uint dev_num, uint src_port, uint dst_port)
{
    uint32_t rc = 0;
     
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_FALSE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          dst_port,
                                          GT_FALSE,
                                          src_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
	cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }

    return rc; 
}


/**********************************************************************
 *
 * Function: xcat3_set_port_pve_single_direction
 *
 * Description: Set xCat3 port pve for single direction
 *
 * Input: dev_num - cpss dev number
 *           src_port - source port
 *           dst_port - destination port
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_set_port_pve_single_direction (uint dev_num, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_TRUE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    return rc; 
}


/**********************************************************************
 *
 * Function: xcat3_clear_port_pve_single_direction
 *
 * Description: Clear xCat3 port pve for single direction
 *
 * Input: dev_num - cpss dev number
 *           src_port - source port
 *           dst_port - destination port
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_clear_port_pve_single_direction (uint dev_num, uint src_port, uint dst_port)
{
    uint32_t rc = 0;
     
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_FALSE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    return rc; 
}


/**********************************************************************
 *
 * Function: xcat3_port_enable
 *
 * Description: Set xCat3 port enable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_port_enable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 1);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet \n");
    }
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_port_disable
 *
 * Description: Set xCat3 port disable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_port_disable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 0);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet");
    }
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_port_mac_loopback_enable
 *
 * Description: Set xCat3 port mac loopback enable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_port_mac_loopback_enable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"Enable port MAC loopback failed, rc 0x%08x\n", rc);
    }
#if 0
#ifdef DEBUG
    uint32_t data;
    rc = xcat2_reg_pci_read(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, &data);
    if (rc != GT_OK) {
        cterr('f',0,"Read port MAC control register failed, rc 0x%08x\n", rc);
	return (FAILED);
    }
    data |= PORT_PCS_LPBK_EN;
    rc = xcat2_reg_pci_write(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, data);
    if (rc != GT_OK) {
        cterr('f',0,"Write port MAC control register failed, rc 0x%08x\n", rc);
    }
#endif
#endif
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_port_mac_loopback_disable
 *
 * Description: Set xCat3 port mac loopback disable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_port_mac_loopback_disable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
        cterr('f',0,"Disable port MAC loopback failed, rc 0x%08x\n", rc);
    }
#if 0
#ifdef DEBUG
    uint32_t data;
    rc = xcat2_reg_pci_read(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, &data);
    if (rc != GT_OK) {
        cterr('f',0,"Read port MAC control register failed, rc 0x%08x\n", rc);
	return (FAILED);
    }
    data &= ~PORT_PCS_LPBK_EN;
    rc = xcat2_reg_pci_write(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, data);
    if (rc != GT_OK) {
        cterr('f',0,"Write port MAC control register failed, rc 0x%08x\n", rc);
    }
#endif
#endif
    return (rc);
}


/******************************************************************** 
* Function:  xcat3_soft_reset
*
* Input:     dev_num  - device number. Number in range 0..31
*
* Output:
*       GT_OK   - on success
*       GT_FAIL - on error
* Description  : Soft reset Marvell switch
*  
* Returns:  PASSED/FAILED
*
*******************************************************************
*/
static int 
xcat3_soft_reset (uint dev_num)
{
    uint32_t rc = GT_OK;

    printf("configure skip parameters related to soft reset\n");
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E,
                                           GT_FALSE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E,
                                           GT_FALSE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E,
                                           GT_TRUE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,
                                           GT_TRUE);

    printf("Calling cpssDxChHwPpSoftResetTrigger\n");
    rc = cpssDxChHwPpSoftResetTrigger(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChHwPpSoftResetTrigger Error code "
               "0x%x\n", rc);
    }
    printf("cpssDxChHwPpSoftResetTrigger done!\n");

    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_vlan_add
 *
 * Description: Add xCat3 vlan
 *
 * Input: dev_num - cpss dev number
 *           vlan_id - vlan ID
 *           port_num - number of port
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_vlan_add (uint dev_num, uint vlan_id, uint num_port)
{
    uint32_t rc = GT_OK;
    uint32_t i;
    CPSS_PORTS_BMP_STC ports_members = {{0}};
    CPSS_PORTS_BMP_STC ports_tagging = {{0}};
    CPSS_DXCH_BRG_VLAN_INFO_STC  vlan_info = {0};
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC ports_tagging_cmd = {{0}};

    for (i = 0; i < num_port; i++) {
	ports_members.ports[i] = 0;
	ports_tagging.ports[i] = 0;
    }

    for (i = 0; i < 256; i++) {
	ports_tagging_cmd.portsCmd[i] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    vlan_info.stgId = 0x0;
    vlan_info.vrfId = 0x0;
    vlan_info.ucastLocalSwitchingEn = GT_FALSE;
    vlan_info.ipv4IpmBrgEn = GT_FALSE;
    vlan_info.unregIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6UcastRouteEn = GT_FALSE;
    vlan_info.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6McastRouteEn = GT_FALSE;
    vlan_info.unregNonIpMcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.mruIdx = 0x0;
    vlan_info.bcastUdpTrapMirrEn = GT_FALSE;
    vlan_info.mirrToRxAnalyzerEn = GT_FALSE;
    vlan_info.ipv6IcmpToCpuEn = GT_FALSE;
    vlan_info.ipv4UcastRouteEn = GT_FALSE;
    vlan_info.ipv4McBcMirrToAnalyzerIndex = 0x0;
    vlan_info.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlan_info.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlan_info.ipv4McastRouteEn = GT_FALSE;
    vlan_info.ipv6McMirrToAnalyzerIndex = 0x0;
    vlan_info.floodVidx = 0xFFF;
    vlan_info.ipv4IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlan_info.ipv6IpmBrgEn = GT_FALSE;
    vlan_info.unkUcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.mirrToTxAnalyzerEn = GT_FALSE;
    vlan_info.ipv6McMirrToAnalyzerEn = GT_FALSE;
    vlan_info.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    vlan_info.unregIpv6McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6SiteIdMode = CPSS_IP_SITE_ID_INTERNAL_E;
    vlan_info.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.unkSrcAddrSecBreach = GT_FALSE;
    vlan_info.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    vlan_info.fidValue = 0x5;
    vlan_info.mirrToTxAnalyzerIndex = 0x0;
    vlan_info.mirrToRxAnalyzerIndex = 0x0;
    vlan_info.ipv6IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlan_info.naMsgToCpuEn = GT_FALSE;
    vlan_info.mcastLocalSwitchingEn = GT_FALSE;
    vlan_info.ipv4IgmpToCpuEn = GT_FALSE;
    vlan_info.unregIpv4McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    /* Disable auto learn on VLAN */
    vlan_info.autoLearnDisable = GT_TRUE;

    rc = cpssDxChBrgVlanEntryWrite(dev_num,
                                   vlan_id,
                                   &ports_members,
                                   &ports_tagging,
                                   &vlan_info,
                                   &ports_tagging_cmd);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanEntryWrite call, rc = %#x\n", rc);
    }
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_vlan_port_add
 *
 * Description: Add xCat3 vlan port
 *
 * Input: dev_num - cpss dev number
 *           vlan_id - vlan ID
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int  
xcat3_vlan_port_add (uint dev_num, uint vlan_id, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChBrgVlanMemberAdd(dev_num, vlan_id, port_num, 
                      GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanMemberAdd call, rc = %#x\n", rc);
	return (FAILED);
    }

    rc = cpssDxChBrgVlanPortVidSet(dev_num, port_num, CPSS_DIRECTION_INGRESS_E, vlan_id);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanPortVidSet call, rc = %#x\n", rc);
    }

    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_vlan_port_del
 *
 * Description: Delete xCat3 vlan port
 *
 * Input: dev_num - cpss dev number
 *           vlan_id - vlan ID
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_vlan_port_del (uint dev_num, uint vlan_id, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChBrgVlanPortDelete(dev_num, vlan_id, port_num);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChBrgVlanPortDelete call, rc = %#x\n", rc);
    }
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_vlan_port_show
 *
 * Description: Short xCat3 vlan port
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_vlan_port_show (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;
    GT_U16 vlan_id; 

    rc = cpssDxChBrgVlanPortVidGet(dev_num, port_num, CPSS_DIRECTION_INGRESS_E, &vlan_id);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChBrgVlanPortVidGet call, rc = %#x\n", rc);
        rc = 0xFF; 
        return (rc);
    }
    rc = vlan_id; 
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_force_link_down_en
 *
 * Description: Force xCat3 link down enable
 *
 * Input: dev_num - cpss dev number
 *           port - port number
 *           set - enable/disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_force_link_down_en (uint dev_num, uint port, boolean set)
{
    return(cpssDxChPortForceLinkDownEnableSet(dev_num, port, set));
}


/**********************************************************************
 *
 * Function: xcat3_force_link_pass_en
 *
 * Description: Force xCat3 link pass enable
 *
 * Input: dev_num - cpss dev number
 *           port - port number
 *           set - enable/disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_force_link_pass_en (uint dev_num, uint port, boolean set)
{
    return(cpssDxChPortForceLinkPassEnableSet(dev_num, port, set));
}


/******************************************************************************
 *
 * Function   :	xcat3_port_force_link_set
 * Description:	set force link status for a specific port
 *              
 * Inputs     :	link: LINK_UP or LINK_DOWN
 *              port:
 *              set: TRUE or FALSE
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_port_force_link_set (uint cpss_dev, uint link, int port, boolean set)
{
    uint32_t dev_num = cpss_dev;
    int rc;

    if (link == LINKDOWN) {
	rc = cpssDxChPortForceLinkDownEnableSet(dev_num, port, set);
	if(rc != GT_OK) {
	    printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	    return (rc);
	}
    } else {
	rc = cpssDxChPortForceLinkPassEnableSet(dev_num, port, set);
	if(rc != GT_OK) {
	    printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	    return (rc);
	}
    }	
    return (rc);
}


/**********************************************************************
 *
 * Function: xcat3_all_reg_test
 *
 * Description: Test xCat3 all register
 *
 * Input: cpss_dev - cpss dev number
 *           testStatus - test status
 *           badReg - bad register
 *           readVal - read value
 *           writeVal - write value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_all_reg_test (uint cpss_dev, uint* testStatus, uint* badReg, uint* readVal, uint* writeVal)
{
    uint32_t rc = 0;
	
    rc  = cpssDxChDiagAllRegTest(cpss_dev,
                                testStatus,
                                badReg,
                                readVal,
                                writeVal);
    if (rc) {
        cterr('f',0,"Error from cpssDxChDiagAllRegTest(), rc = 0x%x\n", rc);
    }
    
    return rc;
} 


/******************************************************************************
 *
 * Function   :	xcat3_pcie_config_read
 * Description: wrapper to read PCIe configuration space register.
 * Inputs     :	offset - register offset
 *                  reg_ptr - pointer to hold register data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
xcat3_pcie_config_read (int offset, uint *reg_ptr)
{
    uint32_t bus;
    int device;

    bus = NANOOK_XCAT_PCIE_BUS;
    device = 0;

    *reg_ptr = pci_config_read(NANOOK_XCAT_PCIE_BUS, device, 0, offset);

    return(PASSED);
}


/******************************************************************************
 *
 * Function   :	xcat3_pcie_config_write
 * Description: wrapper to write PCIe configuration space register.
 * Inputs     :	offset - register offset
 *                  reg_data - register data to write
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat3_pcie_config_write (int offset, uint reg_data)
{
    uint32_t bus;
    int device;

    bus = NANOOK_XCAT_PCIE_BUS;
    device = 0;

    pci_config_write(NANOOK_XCAT_PCIE_BUS, device, 0, offset, reg_data);

    return(PASSED);
}


/**********************************************************************
 *
 * Function: xcat3_led_class_config
 *
 * Description: Configure xCat3 led class
 *
 * Input: led_class - 
 *           inv_ena - 
 *           blk_ena -
 *           blk_sel -
 *           force_ena -
 *           force_data -
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
xcat3_led_class_config (uint cpss_dev, uint32_t led_class, boolean inv_ena, boolean blk_ena, int blk_sel, boolean force_ena, uint32_t force_data)
{
    CPSS_LED_CLASS_MANIPULATION_STC led_class_params;  
    int rc;

    led_class_params.invertEnable = inv_ena;
    led_class_params.blinkEnable  = blk_ena;
    led_class_params.blinkSelect  = blk_sel;
    led_class_params.forceEnable  = force_ena;
    led_class_params.forceData    = force_data;

    rc = cpssDxChLedStreamPortGroupClassManipulationSet(cpss_dev,
				    	CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
					0,
					CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
					led_class, &led_class_params);
    return rc;
}


/**********************************************************************
 *
 * Function: xcat3_led_init
 *
 * Description: Init xCat3 led
 *
 * Input: cpss_dev - cpss dev number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
xcat3_led_init (uint cpss_dev)
{
    int rc;
    int ix;

    CPSS_LED_CONF_STC led_stream_conf;  
    CPSS_LED_GROUP_CONF_STC led_group_params;  
    GT_U32 class0Group = 0; /* led stream group used for class0 */
    GT_U32 class1Group = 1; /* led stream group used for class1 */
    GT_U32 class3Group = 2; /* led stream group used for class3 */
    GT_U32 class4Group = 3; /* led stream group used for class4 */

    //uint32_t value;
    //unsigned long config_base = nanook_get_pci_base_addr();

    GT_U32 dev_num = cpss_dev;

    for (ix = 0; ix < NANOOK_XCAT3_MAX_LED_INTERFACE; ix++) {

        /* for port 0 through 11 and stack port, LED interface 0 is used */
        /*Led stream configuration */
        led_stream_conf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
        /* link status has no effect on other indications */
        led_stream_conf.disableOnLinkDown = GT_FALSE;
        led_stream_conf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        led_stream_conf.blink0Duration  = CPSS_LED_BLINK_DURATION_6_E;
        led_stream_conf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
        led_stream_conf.blink1Duration  = CPSS_LED_BLINK_DURATION_3_E;
        led_stream_conf.pulseStretch  = CPSS_LED_PULSE_STRETCH_3_E;
        /* the first bit in the LED stream to be driven */
        led_stream_conf.ledStart  = 168;
        /* the last bit in the LED stream to be driven */
        //if (get_port_num() == 4)
        //	 led_stream_conf.ledEnd    = 183;
        //else
	        led_stream_conf.ledEnd    = 199;

        /* invert LEDclk pin */
        led_stream_conf.clkInvert = GT_TRUE;
        /* This config is for none dual-media ports */
        led_stream_conf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
        led_stream_conf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;

        rc = cpssDxChLedStreamPortGroupConfigSet(dev_num,
					     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
					     ix, &led_stream_conf);
        if (rc != GT_OK) {
            return rc;
        }

        /* group 0 configuration */
        led_group_params.classA = 0x3;
        led_group_params.classB = 15;
        led_group_params.classC = 15;
        led_group_params.classD = 15;

        rc = cpssDxChLedStreamPortGroupGroupConfigSet(dev_num,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  ix,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class0Group,
                                                  &led_group_params);

        if (rc != GT_OK) {
            return rc;
        }

        /* group 1 configuration */
        led_group_params.classA = 0x4;
        led_group_params.classB = 15;
        led_group_params.classC = 15;
        led_group_params.classD = 15;

        rc = cpssDxChLedStreamPortGroupGroupConfigSet(dev_num,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  ix,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class1Group,
                                                  &led_group_params);

        if (rc != GT_OK) {
            return rc;
        } 
   
        /* group 2 configuration */
        led_group_params.classA = 0x0;
        led_group_params.classB = 15;
        led_group_params.classC = 15;
        led_group_params.classD = 15;

        rc = cpssDxChLedStreamPortGroupGroupConfigSet(dev_num,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  ix,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class3Group,
                                                  &led_group_params);

        if (rc != GT_OK) {
            return rc;
        }

        /* group 3 configuration */
        led_group_params.classA = 0x1;
        led_group_params.classB = 15;
        led_group_params.classC = 15;
        led_group_params.classD = 15;

        rc = cpssDxChLedStreamPortGroupGroupConfigSet(dev_num,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  ix,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class4Group,
                                                  &led_group_params);

        if (rc != GT_OK) {
            return rc;
        }

    }

#if 0

    /* config LED_CTRL(GPIO32) as output first */
    value = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG);
    value &= ~(0x1 << (GPIO_LED_CTRL - 32));
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG) = value;

    /* Drive GPP11(GPIO32) to low to enable the LEDs */
    value = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG);
    value &= ~(0x1 << (GPIO_LED_CTRL - 32));
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG) = value;

#endif

    /* turn off all the LEDs */
    if (xcat3_led_class_config(dev_num ,0, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (xcat3_led_class_config(dev_num, 1, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }

    if (xcat3_led_class_config(dev_num, 3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (xcat3_led_class_config(dev_num, 4, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }

    return rc;
}


/******************************************************************************
 *
 * Function   :	xcat3_smi_phy_init
 * Description:	init SMI master0 and master1 for PHY access.
 *              
 * Inputs     :	cpss_dev - cpss dev number 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_smi_phy_init (uint cpss_dev)
{
    int rc, ix;
    uint32_t sum_err = 0;

    GT_U32 dev_num = cpss_dev;

    /* to fix the PHY access issue after CPSS re-init. */
    xcat3_reg_pci_write(dev_num, LMS0_MISC_CONFIG_REG_OFF, 0x70000);
    /* SMI0, 1680*2, PORT 0-15, PHYSICAL PHY ADDR SMI0 0-15 */
    for (ix = NANOOK_ESW_SMI0_PORT_START_NUM; ix < NANOOK_ESW_SMI1_PORT_START_NUM; ix++) {
	/* set PHY SMI address */
	rc = cpssDxChPhyPortAddrSet(dev_num, ix, (ix - NANOOK_ESW_SMI0_PORT_START_NUM));
	if (rc == PASSED) {
	    /* set SMI interface 0 for PHY ports */
	    rc = cpssDxChPhyPortSmiInterfaceSet(dev_num, ix, CPSS_PHY_SMI_INTERFACE_0_E);
	}
	if (rc != PASSED) {
	    sum_err++;
	}
    }
    /* SMI1, 1680*1, PORT 16-23, PHYSICAL PHY ADDR SMI1 0-7 */
    for (ix = NANOOK_ESW_SMI1_PORT_START_NUM; ix < NANOOK_ESW_PORT_NUM; ix++) {
	/* set PHY SMI address */
	rc = cpssDxChPhyPortAddrSet(dev_num, ix, (ix-NANOOK_ESW_SMI1_PORT_START_NUM));
	if (rc == PASSED) {
	    /* set SMI interface 1 for PHY ports */
	    rc = cpssDxChPhyPortSmiInterfaceSet(dev_num, ix, CPSS_PHY_SMI_INTERFACE_1_E);
	}
	if (rc != PASSED) {
	    sum_err++;
	}
    }

    if (sum_err == 0) {
	rc = cpssDxChPhyPortSmiInit(dev_num);
	if (rc != PASSED) 
	    return (GT_FAIL);
    } else {
	return (GT_FAIL);  
    }

    //printf("DBG: xcat3_smi_phy_init - pass.\n");

    return PASSED;
}


/******************************************************************************
 *
 * Function   :	xcat3_reg_pci_read
 * Description: Read memory mapped xCat3 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *                  offset - register offset
 *                  *data - pointer to hold the read data 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat3_reg_pci_read (uint cpss_dev, uint offset, uint *data)
{
    uint32_t rc;
    int port_group = 0;

    rc = cpssDrvPpHwRegisterRead(cpss_dev, port_group, offset, (GT_U32 *)data);

    return (rc);
}


/******************************************************************************
 *
 * Function   :	xcat3_reg_pci_write
 * Description: write to memory mapped xcat3 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *                  offset - register offset
 *                  data - data to write 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat3_reg_pci_write (uint cpss_dev, uint offset, uint data)
{
    uint32_t rc;
    int port_group = 0;

    rc = cpssDrvPpHwRegisterWrite(cpss_dev, port_group, offset, data);

    return (rc);    
}


/******************************************************************************
 *
 * Function   :	xcat3_reg_config_read
 * Description: Read memory mapped xCat3 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *                  offset - register offset
 *                  *data - pointer to hold the read data 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat3_reg_config_read (uint cpss_dev, uint offset, uint *data)
{

    GT_UINTPTR pci_base, internal_base, ex_base;
    nim_dm_cpss_get_pciemap_ex(&pci_base, &internal_base, &ex_base);

    *data = *(unsigned int *)(internal_base + offset);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	xcat3_reg_config_write
 * Description: write to memory mapped xcat3 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *                  offset - register offset
 *                  data - data to write 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat3_reg_config_write (uint cpss_dev, uint offset, uint data)
{

    GT_UINTPTR pci_base, internal_base, ex_base;
    nim_dm_cpss_get_pciemap_ex(&pci_base, &internal_base, &ex_base);

    *(unsigned int *)(internal_base + offset) = data;

    return (PASSED);   
}

/*
 **********************************************************************
 *
 *  Function: xcat3_port_init
 *
 *  Description: Xcat3 port initial
 *
 *  Input: dev_num - device number
 *         port_num - port number
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_port_init (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;
    //uint32_t mruSize;
    CPSS_PORT_SPEED_ENT speed;
    uint32_t port_mode;

    CPSS_PORTS_BMP_STC      initPortsBmp;       /* bitmap of ports to init */

    if (port_num >= GE0_XCAT3_PORT) {
	
	port_mode = CPSS_PORT_INTERFACE_MODE_KR_E;
	speed = CPSS_PORT_SPEED_10000_E;

    } else {
	port_mode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;
	speed = CPSS_PORT_SPEED_1000_E;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, port_num);

    rc = cpssDxChPortModeSpeedSet(dev_num, initPortsBmp, GT_TRUE, port_mode, speed);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortInterfaceModeSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }

    //mruSize = 1522; /* default */



    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, TRUE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

    if (port_num < GE0_XCAT3_PORT) {


#if 0

        rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_TRUE);
        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }

        rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_TRUE);

        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }
#if 0
        rc = cpssDxChPortFlowCntrlAutoNegEnableSet(dev_num, port_num, GT_TRUE, GT_TRUE);
        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortFlowCntrlAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }

        rc = cpssDxChPortFlowControlEnableSet(dev_num, port_num, CPSS_PORT_FLOW_CONTROL_RX_TX_E);
        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortFlowControlEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }
#endif
#endif 
       
#if 1
        rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_FALSE);
        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }

        rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);

        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	    return rc;
        }

        rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
        if(rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortSpeedSet() for port %d,"
	          " err code %d", port_num, rc);
	    return rc;
        }

	 rc = cpssDxChPortDuplexModeSet(dev_num, port_num, CPSS_PORT_FULL_DUPLEX_E);
        if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortDuplexModeSet() for port %d,"
	          " err code %d", port_num, rc);
	    return rc;
         }
#endif

    }


    if(port_num == GE0_XCAT3_PORT) {

#if 1
        if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(dev_num,port_num,GT_FALSE)) != GT_OK)
        {
            return rc;
        }

        if((rc = cpssDxChPortInbandAutoNegEnableSet(dev_num,port_num,GT_TRUE)) != GT_OK)
        {
            return rc;
        }
#endif

		
    }



    rc = cpssDxChPortSerdesPowerStatusSet(dev_num,port_num,
					  CPSS_PORT_DIRECTION_BOTH_E,0x8,GT_TRUE);
    if( rc != GT_OK ) {
	cterr('f',0,"Failed cpssDxChPortSerdesPowerStatusSet, rc = %#x", rc);
	return rc;
    }

#if 0
    rc = cpssDxChPortMruSet(dev_num, port_num, mruSize);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortMruSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }
#endif

    rc = cpssDxChPortMacCountersEnable(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to enable Port counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChPortMacCountersClearOnReadSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to clear counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChBrgVlanPortIngFltEnable(dev_num, port_num, GT_TRUE);
    if (rc) {
	cterr('f',0,"cpssDxChBrgVlanPortIngFltEnable failed\n");
	return (rc);
    }

    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, FALSE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

#if 0
    if(port_num == 0) {

		cpssDrvPpHwRegisterWrite(0, 0, 0x12000000, 0x8FA1);
		cpssDrvPpHwRegisterWrite(0, 0, 0x12000004, 0x3);
		cpssDrvPpHwRegisterWrite(0, 0, 0x12000008, 0xc009);
		cpssDrvPpHwRegisterWrite(0, 0, 0x12000044, 0xc);
		cpssDrvPpHwRegisterWrite(0, 0, 0x1200000c, 0xb0e8);
		
    }
#endif
    return (rc);
	
#if 0
    /*********************************************************/
    /* Port Configuration                                    */
    /*********************************************************/
    mruSize = 1522; /* default */

    if (port_num >= GE0_XCAT3_PORT)
	port_mode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
    else
	port_mode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;

    speed = CPSS_PORT_SPEED_1000_E;

    rc = cpssDxChPortInterfaceModeSet(dev_num, port_num, port_mode);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortInterfaceModeSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }


    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, TRUE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

    rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);

    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortDuplexModeSet(dev_num, port_num, CPSS_PORT_FULL_DUPLEX_E);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexModeSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSerdesPowerStatusSet(dev_num,port_num,
					  CPSS_PORT_DIRECTION_BOTH_E,0x8,GT_TRUE);
    if( rc != GT_OK ) {
	cterr('f',0,"Failed cpssDxChPortSerdesPowerStatusSet, rc = %#x", rc);
	return rc;
    }

    rc = cpssDxChPortMruSet(dev_num, port_num, mruSize);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortMruSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortMacCountersEnable(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to enable Port counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChPortMacCountersClearOnReadSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to clear counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChBrgVlanPortIngFltEnable(dev_num, port_num, GT_TRUE);
    if (rc) {
	cterr('f',0,"cpssDxChBrgVlanPortIngFltEnable failed\n");
	return (rc);
    }

    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, FALSE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

    return (rc);
#endif
}


/*
 **********************************************************************
 *
 *  Function: xcat3_specific_port_init
 *
 *  Description: Xcat3 specific port initial
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_specific_port_init (uint cpss_dev)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = cpss_dev;

    /* Port initialization */

    for (port = 0; port < NANOOK_XCAT3_USED_PORT; port++) {
	rc = xcat3_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat3_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    port = GE0_XCAT3_PORT;
    rc = xcat3_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat3_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
    }


    /* put used ports in reset */
    for (port = 0; port < NANOOK_XCAT3_USED_PORT; port++) {
	rc = cpssDxChPortEnableSet(dev_num, port, GT_FALSE);
	if (rc != GT_OK) {
	    cterr('f', 0," Error failed port disable:cpssDxChPortEnableSet err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

	rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_TRUE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port reset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    /* unreset the ports */
    for (port = 0; port < NANOOK_XCAT3_USED_PORT; port++) {
	rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_FALSE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

	rc = cpssDxChPortEnableSet(dev_num, port, GT_TRUE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		  " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    port = GE0_XCAT3_PORT;

    rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_FALSE);
    if (rc != GT_OK) {
	cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
	    " dev_num %d, port %d\n", rc, dev_num, port);
	return (rc);
    }

    rc = cpssDxChPortEnableSet(dev_num, port, GT_TRUE);
    if (rc != GT_OK) {
	cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		  " dev_num %d, port %d\n", rc, dev_num, port);
	return (rc);
    }

    return (rc);
}


/*
 **********************************************************************
 *
 *  Function: xcat3_specific_port_enable
 *
 *  Description: Xcat3 specific port enable
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_specific_port_enable (uint cpss_dev)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = cpss_dev;
    GT_BOOL is_link_up;

    /* Port Enable */
    for (port = 0; port < NANOOK_XCAT3_USED_PORT; port++) {
	rc = xcat3_port_enable(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0,"Error failed xcat3_port_enable 0x%0x, "
		   "dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

#if 1
	if (xcat3_port_force_link_set(dev_num, LINKUP, port, TRUE) != OK) {
	    cterr('f',0,"Failed xcat3_port_force_link_set()");
	    return (FAILED);
	}
	if (cpssDxChPortLinkStatusGet(dev_num, port, &is_link_up) != OK) {
	    cterr('f',0,"Failed cpssDxChPortLinkStatusGet()");
	    return (FAILED);
	}
	if (is_link_up == GT_FALSE) {
	    cterr('f',0,"Link is not up for port %d", port);
	    return (FAILED);
	}
#endif

    }

    port = GE0_XCAT3_PORT;

    rc = xcat3_port_enable(dev_num, port);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed xcat3_port_enable 0x%0x, "
            "dev_num %d, port %d\n", rc, dev_num, port);
	 return (rc);
    }

    return (GT_OK);
}


/*
 **********************************************************************
 *
 *  Function: xcat3_port_phy_1680_init
 *
 *  Description: Xcat3 port phy 1680 enable
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_port_phy_1680_init (void)
{

    uint ix, rc;
    uint port_num = 0, reg_addr = 0; 
    unsigned short r_data;
    unsigned short w_data;
    

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
        port_num = ix;

	 //reg_addr = 22;
	 //smi_read_reg(port_num, reg_addr, &r_data);
	 //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);
	 
	 //reg_addr = 2;
	 //smi_read_reg(port_num, reg_addr, &r_data);
	 //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);

        reg_addr = 22;
        w_data = 4;
        smi_write_reg(port_num, reg_addr, w_data);

	 //reg_addr = 22;
	 //smi_read_reg(port_num, reg_addr, &r_data);
	 //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);
	 
	 reg_addr = 27;
	 smi_read_reg(port_num, reg_addr, &r_data);
        //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);

        reg_addr = 27;
	 w_data = (r_data & (~(0x20)));
	 smi_write_reg(port_num, reg_addr, w_data);

        //reg_addr = 27;
	 //smi_read_reg(port_num, reg_addr, &r_data);
        //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);

	 //reg_addr = 22;
	 //smi_read_reg(port_num, reg_addr, &r_data);
	 //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n\n", port_num, reg_addr, r_data);

	 //reg_addr = 22;
        //w_data = 0;
        //smi_write_reg(port_num, reg_addr, w_data);

	 //reg_addr = 0;
	 //smi_read_reg(port_num, reg_addr, &r_data);
	 //printf("DBG: SMI Test - port:%d  reg:0x%x  data:0x%x\n", port_num, reg_addr, r_data);

	 reg_addr = 22;
        w_data = 3;
        smi_write_reg(port_num, reg_addr, w_data);

	 reg_addr = 16;
	 w_data = 0x01;
	 smi_write_reg(port_num, reg_addr, w_data);

        //PHY Errata

        /* MACSec and PTP disable */
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0x12);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 27, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }



        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0xFD);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 8, 0xB53);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 7, 0x200D);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* EEE init */
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0xFF);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 17, 0xB030);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 16, 0x215C);
        if( rc != GT_OK)
        {
            return rc;
        }
		
        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 22, 0x0);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 16, 0x3360);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(marvell_ac3_cpss_dev_num_nanook, port_num, 0, 0x9140);
        if( rc != GT_OK)
        {
            return rc;
        }
	 
    }

    return (GT_OK);
}



/***********************************************************************
 *
 *  Function: xcat3_cpss_pp_phase1_info_init
 *
 *  Description: Xcat3 initial phase 1
 *
 *  Input: cpss_dev - cpss dev number 
 *
 *  Outputs: GT_OK - on success
 *       
 **********************************************************************
 */
static int 
xcat3_cpss_pp_phase1_info_init (uint cpss_dev)
{
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *info;
    nim_dm_cpss_bind_func bind_func;
    GT_VOID *int_vect;
    GT_UINTPTR int_mask;
    GT_UINTPTR pci_base, internal_base, ex_base;


    /* since we use MSI interrupt, PCI interrupt line number is a dummy here */
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
	cterr('f',0,"Failed to get interrupt vector number.");
	return FAILED;
    }

    if (extDrvGetIntMask(0, &int_mask) != GT_OK) {
	cterr('f',0,"Failed to get interrupt mask.");
	return FAILED;
    }
#ifdef DEBUG
    printf("int_vect = %lu, int_mask = %u\n", (unsigned long)int_vect, int_mask);
#endif
    nim_dm_cpss_get_pciemap_ex(&pci_base, &internal_base, &ex_base);
#ifdef DEBUG
    printf("pci_base = %lx, internal_base = %lx, exreg_base = %lx\n", pci_base, internal_base, ex_base);
#endif
    nim_dm_cpss_get_extserv(&bind_func.extDrv, &bind_func.os, &bind_func.trace);
    if (cpssExtServicesBind(&bind_func.extDrv, &bind_func.os, &bind_func.trace) != GT_OK) {
	cterr('f',0,"Failed to do cpssExtServicesBind()");
	return (FAILED);
    }

    info = &xcat3_pp_phase1_info;

    memset(info, 0, sizeof(*info));

    /* AC3 PHASE 1 CONFIG */
    info->hwAddr.busNo= 3;
    info->hwAddr.devSel = 0;
    info->hwAddr.funcNo = 0;
    info->devNum = 0;
    info->busBaseAddr = pci_base;
    info->internalPciBase = internal_base;
    info->resetAndInitControllerBase = ex_base;
    info->intVecNum = CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS;
    info->intMask = CPSS_PARAM_NOT_USED_CNS;
    info->coreClock = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;
    info->mngInterfaceType = CPSS_CHANNEL_PEX_MBUS_E;
    info->isrAddrCompletionRegionsBmp = 0x02;
    info->appAddrCompletionRegionsBmp = 0x3C;
    info->ppHAState = CPSS_SYS_HA_MODE_ACTIVE_E;
    info->powerDownPortsBmp.ports[0] = 0;
    info->serdesRefClock = CPSS_DXCH_PP_SERDES_REF_CLOCK_INTERNAL_125_E;
    info->isExternalCpuConnected = GT_TRUE;
    info->numOfPortGroups = 0;

    return (GT_OK);
}



/*
 **********************************************************************
 *
 *  Function: xcat3_cpss_pp_phase2_info_init
 *
 *  Description: Xcat3 initial phase 2
 *
 *  Input: cpss_dev - cpss dev number
 *
 *  Outputs: GT_OK - on success
 *       
 **********************************************************************
 */
static int 
xcat3_cpss_pp_phase2_info_init (uint cpss_dev)
{
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *info;
    int tc = 0;

    info = &xcat3_pp_phase2_info;

    memset(info, 0, sizeof(*info));

    info->newDevNum = cpss_dev;	

    info->fuqUseSeparate = FALSE;

    info->auqCfg.auDescBlockSize = 0;
    info->auqCfg.auDescBlock = NULL;
    info->fuqCfg.auDescBlockSize = 0;
    info->fuqCfg.auDescBlock = NULL;

    info->netIfCfg.txDescBlockSize = 0;
    info->netIfCfg.txDescBlock = NULL;

    info->netIfCfg.rxDescBlockSize = 0;
    info->netIfCfg.rxDescBlock = NULL;
    info->netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
    info->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = 0;
    info->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = NULL;

    for (tc = 0; tc < (CPSS_MAX_RX_QUEUE_CNS - 1); tc++) {
        info->netIfCfg.rxBufInfo.bufferPercentage[tc] = 0;
    }

    info->netIfCfg.rxBufInfo.bufferPercentage[tc] = 0;

    /* HeaderOffset- The number of bytes at the beginning of the buffer reserved
     * for the application's use before the start of the received packet */
    info->netIfCfg.rxBufInfo.headerOffset = 0;

    /* RxBufSize- The SDMA Rx data buffer size to be used in the CPU. If the 
     * received packet size is greater than the buffer size, the packet is broken
     * and chained to multiple buffers */
    info->netIfCfg.rxBufInfo.rxBufSize = 0;

    return (GT_OK);

}


/******************************************************************** 
* Function:  xcat3_cpss_driver_init
*
* Input: cpss_dev - cpss dev number 
*
* Output:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
* Description  : CPSS driver init
*  
* Returns:  Unix style pass(zero) / fail (non-zero)
*
********************************************************************/
static int 
xcat3_cpss_driver_init (uint cpss_dev)
{
    int rc = 0;
    //GT_U32 dev_num = xcat3_dev_num[get_slot_num()-1];
    GT_U32 dev_num = cpss_dev;
    CPSS_VERSION_INFO_STC cpssVersion;    
    CPSS_REG_VALUE_INFO_STC *regCfgList; 
    GT_U32 regCfgListSize;       
    CPSS_DXCH_IMPLEMENT_WA_ENT waArr[1]={CPSS_DXCH_IMPLEMENT_WA_SERDES_INTERNAL_REG_ACCESS_E} ;

    /* Get CPSS version */
    cpssDxChVersionGet(&cpssVersion);

    cpssLogEnableSet(GT_TRUE);

    rc = xcat3_cpss_pp_phase1_info_init(dev_num);
    if (rc != GT_OK) {
        cterr('f',0," Failed xcat3_cpss_pp_phase1_info_init(), error code 0x%x\n", rc);  
        return (rc);
    } 


    /* Initialize the internal DB of CPSS regarding PPs */

    rc = cpssPpInit();
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssPpInit Error code 0x%x\n",rc);
        return (rc);
    } else {
        marvell_cpssPpInit_xcat3 = TRUE;
    }


    /*********************************************************************/
    /*   HW Phase 1 initialization                                       */
    /*********************************************************************/
    dev_num = xcat3_pp_phase1_info.devNum;
    rc = cpssDxChHwPpPhase1Init(&xcat3_pp_phase1_info, &xcat3_pp_dev_type);
    //rc = internal_cpssDxChHwPpPhase1Init_new(&xcat3_pp_phase1_info, &xcat3_pp_dev_type);
    if ((rc != GT_OK) && (rc != GT_ALREADY_EXIST)) {
        cterr('f',0,"Failed cpssDxChHwPpPhase1Init(), error code 0x%x\n", rc);
	 //cterr('f',0,"Failed internal_cpssDxChHwPpPhase1Init_new(), error code 0x%x\n", rc);
        return (rc);
    }  

    rc = cpssDxChHwPpImplementWaInit(dev_num, 1, waArr, NULL);
    if (rc) {
	cterr('f',0,"Failed cpssDxChHwPpImplementWaInit(), error code 0x%x\n", rc);
        return (rc);
    }


    /* Config functions for this board */          
    regCfgList     = dummyRegValInfoList;
    regCfgListSize = (sizeof(dummyRegValInfoList) / sizeof(CPSS_REG_VALUE_INFO_STC));

    /* Set PP's registers */
    //rc = hwPpStartInit(dev_num, regCfgList, regCfgListSize);
    rc = cpssDxChHwPpStartInit(dev_num, 0, regCfgList, regCfgListSize);
	
    if (rc != GT_OK) {
	cterr('f',0,"Failed hwPpStartInit(), error code 0x%x\n", rc);
	return rc;
    }

    /*********************************************************************/
    /*   HW Phase 2 initialization                                       */
    /*********************************************************************/
    rc= xcat3_cpss_pp_phase2_info_init(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed xcat3_cpss_pp_phase2_info_init(), "
	      "error code 0x%x\n", rc);
        return (rc);
    } 

    rc = cpssDxChHwPpPhase2Init(dev_num, &xcat3_pp_phase2_info);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChHwPpPhase2Init(), error code 0x%x\n", rc);  
        return (rc);
    } 
    
    return rc;
}


/*
 **********************************************************************
 *
 *  Function: xcat3_cpss_device_init
 *
 *  Description: Xcat3 device initial
 *
 *  Input: cpss_dev - cpss dev number 
 *
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_cpss_device_init (uint cpss_dev)
{
    int rc = 0;
    GT_U32 dev_num = cpss_dev;
    //int ix;
    
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppConfig;

    /*********************************************************************/
    /*   Logical phase initialization                                      */
    /*********************************************************************/
    ppConfig.routingMode = CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E;
    rc = cpssDxChCfgPpLogicalInit(dev_num, &ppConfig);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChCfgPpLogicalInit(),  rc %d\n", rc);  
        return (rc);
    } 
   
    /*********************************************************************/
    /*   FDB initialization                                              */
    /*********************************************************************/
    rc = cpssDxChBrgFdbInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgFdbInit() rc %d\n", rc);  
        return (rc);
    }

    /********** Set FDB hash function mode */
    rc = cpssDxChBrgFdbHashModeSet(dev_num, CPSS_MAC_HASH_FUNC_XOR_E);
    if (rc != GT_OK) {
        cterr('f', 0,"Failed cpssDxChBrgFdbHashModeSet(),  rc %d\n", rc);
        return (rc);
    } 

    /********** Set the VLAN lookup mode */
    rc = cpssDxChBrgFdbMacVlanLookupModeSet(dev_num, CPSS_IVL_E);
    if (rc != GT_OK) {
        printf("Failed cpssDxChBrgFdbMacVlanLookupModeSet()\n");
        return (rc);
    } 

    /********** VLAN Initialization-- set VLAN-aware mode */
    rc = cpssDxChBrgVlanBridgingModeSet(dev_num, CPSS_BRG_MODE_802_1Q_E);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgVlanBridgingModeSet()\n");  
        return (rc);
    } 

    /********** Port Initialization */
    rc = cpssDxChPortStatInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChPortStatInit()\n");  
        return (rc);
    }

    /********** Port Configuration */
    rc = xcat3_specific_port_init(dev_num);
    if (rc != GT_OK) {
        cterr('f', 0, "xcat3_specific_port_init() failed, rc 0x%x\n", rc);
        return (rc);
    }
	
    /********** Enable ports */
    rc = xcat3_specific_port_enable(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"xcat3_specific_port_enable() failed, rc 0x%x\n", rc);  
        return (rc);
    } 

    /********** Enable device */
    rc = cpssDxChCfgDevEnable(dev_num, GT_TRUE);
    if(rc != GT_OK) {
        cterr('f',0,"cpssDxChCfgDevEnable() failed, rc 0x%x\n",rc);
        return (rc);
    }

    return (rc);
}


/*
 **********************************************************************
 *
 *  Function: xcat3_cpss_config_test_mode
 *
 *  Description: Xcat3 config test mode for 10G-KR port 
 *
 *  Input: cpss_dev - cpss dev number 
 *
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int 
xcat3_cpss_set_test_mode(uint cpss_dev, uint port_num, uint mode, uint pattern)
{
	
    if (port_num != GE0_XCAT3_PORT) {
        cterr('f',0,"Unsupported port %d for 10G-KR test mode setting.\n", port_num);  
        return (FAILED);
    }

    /* Disable send pattern before setting for serdes 6. */
    xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);

    if (mode == XCAT3_10GKR_TEST_MODE_DISABLE) {
    
        /* Disable send pattern for serdes 6. */
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
      
		
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS09) {

	 /* start register to enable pattern on serdes 6 */
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x81);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x40e0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x872);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x72);
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x81);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x80e0);
	 /* end register to enable pattern on serdes 6 */
		
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS15) {

	 /* start register to enable pattern on serdes 6 */
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x82);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x40e0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x872);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x72);
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x82);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x80e0);
	 /* end register to enable pattern on serdes 6 */
		
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS31) {

	 /* start register to enable pattern on serdes 6 */
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x84);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x40e0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x872);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x72);
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x84);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x80e0);
	 /* end register to enable pattern on serdes 6 */
		
    } else if (mode == XCAT3_10GKR_TEST_MODE_8180) {
    
        /* Setting from Marvell */
        /* start user-define pattern */
        xcat3_reg_pci_write(cpss_dev, 0x13006858, 0xff);
        xcat3_reg_pci_write(cpss_dev, 0x1300685c, 0xff);
        xcat3_reg_pci_write(cpss_dev, 0x13006860, 0xff);
        xcat3_reg_pci_write(cpss_dev, 0x13006864, 0xff);
        xcat3_reg_pci_write(cpss_dev, 0x13006868, 0xff);
        /* end of user-define pattern */

        /* start register to enable pattern on serdes 6 */
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x05);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x40e0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x872);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x72);
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x05);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x80e0);
        /* end register to enable pattern on serdes 6 */
		
    } else if (mode == XCAT3_10GKR_TEST_MODE_CUSTOMIZE) {
    
        /* Setting from Marvell */
        /* start user-define pattern */
        xcat3_reg_pci_write(cpss_dev, 0x13006858, pattern);
        xcat3_reg_pci_write(cpss_dev, 0x1300685c, pattern);
        xcat3_reg_pci_write(cpss_dev, 0x13006860, pattern);
        xcat3_reg_pci_write(cpss_dev, 0x13006864, pattern);
        xcat3_reg_pci_write(cpss_dev, 0x13006868, pattern);
        /* end of user-define pattern */

        /* start register to enable pattern on serdes 6 */
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x05);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x40e0);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0xe0);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x872);
        xcat3_reg_pci_write(cpss_dev, 0x1300688c, 0x72);
        xcat3_reg_pci_write(cpss_dev, 0x1300686c, 0x05);
        xcat3_reg_pci_write(cpss_dev, 0x13006854, 0x80e0);
        /* end register to enable pattern on serdes 6 */
		
    } else {
        cterr('f',0,"Unsupported test mode setting for 10G-KR port %d.\n", mode);  
        return (FAILED);
    }


    msleep(ESW_WAIT_1000MS);
	
    if (mode == XCAT3_10GKR_TEST_MODE_DISABLE) {
        printf("Set 10G-KR test mode normal has done.\n");
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS09) {
        printf("Set 10G-KR test mode PRBS09 has done.\n");
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS15) {
        printf("Set 10G-KR test mode PRBS15 has done.\n");
    } else if (mode == XCAT3_10GKR_TEST_MODE_PRBS31) {
        printf("Set 10G-KR test mode PRBS31 has done.\n");
    } else if (mode == XCAT3_10GKR_TEST_MODE_8180) {
        printf("Set 10G-KR test mode 8180 has done.\n");
    } else if (mode == XCAT3_10GKR_TEST_MODE_CUSTOMIZE) {
        printf("Set 10G-KR test mode customize pattern 0x%x has done.\n", pattern);
    } else {
        cterr('f',0,"Unsupported test mode setting for 10G-KR port %d.\n", mode);  
        return (FAILED);
    }


    return (PASSED);

}


/******************************************************************************
 *
 * Function   :	xcat3_serdes_tx_config_read
 * Description: perform xcat3 serdes tx config read.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_serdes_tx_config_read (void)
{

    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC serdesTxCfgPtr;
    GT_U8    port_num;
    GT_U32    laneNum = 0;
    uint rc;

    /* For HWDV verification */

    printf("Port Map:\n");
    printf("Port 0 - 88E1680-1\n");
    printf("Port 4 - 88E1680-1\n");
    printf("Port 8 - 88E1680-2\n");
    printf("Port 12 - 88E1680-2\n");
    printf("Port 16 - 88E1680-3\n");
    printf("Port 20 - 88E1680-3\n");
    printf("Port 24 - 10G-KR\n");
    port_num = getdec_answer("Enter port number(0/4/8/12/16/20/24): ", 0, 0, NANOOK_ESW_PORT_NUM);
    if((port_num == 0) || (port_num == 4) || (port_num == 8) || (port_num == 12) || 
		(port_num == 16) || (port_num == 20) || (port_num == 24) )  {

	rc = cpssDxChPortSerdesManualTxConfigGet(NANOOK_AC3_CPSS_DEV, port_num, laneNum, &serdesTxCfgPtr);
       if (rc == FAILED) {
           printf("Call cpssDxChPortSerdesManualTxConfigGet() failed with error number 0x%x\n", rc);
	    return(FAILED);
       }

        printf("Current serdes tx config for port %d:\n", port_num);
        printf("txAmp: %d\n", serdesTxCfgPtr.txAmp);
        printf("emph0: %d\n", serdesTxCfgPtr.emph0);
        printf("emph1: %d\n", serdesTxCfgPtr.emph1);
	 
    } else {
        printf("Unsupported port number %d, please enter 0/4/8/12/16/20/24 port number\n");
	 return(FAILED);
    }

    return(PASSED);

}


/******************************************************************************
 *
 * Function   :	xcat3_serdes_tx_config_write
 * Description: perform xcat3 serdes tx config write.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_serdes_tx_config_write (void)
{

    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC serdesTxCfgPtr;
    GT_U8    port_num;
    GT_U32    laneNum = 0;
    uint rc;

    GT_U32      txAmp;
    GT_32       emph0;
    GT_32       emph1;

    /* For HWDV verification */
	
    printf("Port Map:\n");
    printf("Port 0 - 88E1680-1\n");
    printf("Port 4 - 88E1680-1\n");
    printf("Port 8 - 88E1680-2\n");
    printf("Port 12 - 88E1680-2\n");
    printf("Port 16 - 88E1680-3\n");
    printf("Port 20 - 88E1680-3\n");
    printf("Port 24 - 10G-KR\n");
    port_num = getdec_answer("Enter port number(0/4/8/12/16/20/24): ", 0, 0, NANOOK_ESW_PORT_NUM);
    if(!((port_num == 0) || (port_num == 4) || (port_num == 8) || (port_num == 12) || 
		(port_num == 16) || (port_num == 20) || (port_num == 24)))  {
        printf("Unsupported port number %d, please enter 0/4/8/12/16/20/24 port number\n");
	 return(FAILED);
    }

    txAmp = getdec_answer("Enter txAmp (0~32): ", 0, 0, 32);
    emph0 = getdec_answer("Enter emph0 (0~15): ", 0, 0, 15);
    emph1 = getdec_answer("Enter emph1 (0~15): ", 0, 0, 15);

    serdesTxCfgPtr.emph0 = emph0;
    serdesTxCfgPtr.emph1 = emph1;
    serdesTxCfgPtr.txAmp = txAmp;
    serdesTxCfgPtr.txAmpAdjEn = GT_FALSE;
    serdesTxCfgPtr.txAmpShft = GT_FALSE;

    rc = cpssDxChPortSerdesManualTxConfigSet(NANOOK_AC3_CPSS_DEV, port_num, laneNum, &serdesTxCfgPtr);
    if (rc == FAILED) {
        printf("Call cpssDxChPortSerdesManualTxConfigSet() failed with error number 0x%x\n", rc);
	 return(FAILED);
    }

    return(PASSED);

}


/******************************************************************************
 *
 * Function   :	xcat3_phy_tx_config_read
 * Description: perform xcat3 phy tx config read.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_phy_tx_config_read (void)
{

    uint port_num = 0, reg_addr = 0; 
    unsigned short r_data;
    unsigned short w_data;

    /* For HWDV verification */

    printf("Port Map:\n");
    printf("Port 0 - 88E1680-1\n");
    printf("Port 4 - 88E1680-1\n");
    printf("Port 8 - 88E1680-2\n");
    printf("Port 12 - 88E1680-2\n");
    printf("Port 16 - 88E1680-3\n");
    printf("Port 20 - 88E1680-3\n");
	
    port_num = getdec_answer("Enter port number(0/4/8/12/16/20): ", 0, 0, NANOOK_ESW_PORT_NUM);
    if((port_num == 0) || (port_num == 4) || (port_num == 8) || (port_num == 12) || 
		(port_num == 16) || (port_num == 20))  {

        reg_addr = 22;
        w_data = 0xFD;
        smi_write_reg(port_num, reg_addr, w_data);

        reg_addr = 7;
	 w_data = 0x100D;
	 smi_write_reg(port_num, reg_addr, w_data);

	 reg_addr = 9;
	 smi_read_reg(port_num, reg_addr, &r_data);

        printf("Current 88E1680 tx config (Reg 0x9) for port %d: 0x%x\n", port_num, r_data);
        
	 
    } else {
        printf("Unsupported port number %d, please enter 0/4/8/12/16/20 port number\n");
	 return(FAILED);
    }

    return(PASSED);

}


/******************************************************************************
 *
 * Function   :	xcat3_phy_tx_config_write
 * Description: perform xcat3 phy tx config write.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat3_phy_tx_config_write (void)
{

    uint port_num = 0, reg_addr = 0; 
    unsigned short r_data;
    unsigned short w_data;
    uint data;

    /* For HWDV verification */

    printf("Port Map:\n");
    printf("Port 0 - 88E1680-1\n");
    printf("Port 4 - 88E1680-1\n");
    printf("Port 8 - 88E1680-2\n");
    printf("Port 12 - 88E1680-2\n");
    printf("Port 16 - 88E1680-3\n");
    printf("Port 20 - 88E1680-3\n");

    port_num = getdec_answer("Enter port number(0/4/8/12/16/20): ", 0, 0, NANOOK_ESW_PORT_NUM);
    if((port_num == 0) || (port_num == 4) || (port_num == 8) || (port_num == 12) || 
		(port_num == 16) || (port_num == 20))  {


        data = gethex_answer("Enter write data: ", 0, 0, 0xffff);

        reg_addr = 22;
        w_data = 0xFD;
        smi_write_reg(port_num, reg_addr, w_data);

        reg_addr = 8;
	 w_data = (unsigned short)data;
	 smi_write_reg(port_num, reg_addr, w_data);

        reg_addr = 7;
	 w_data = 0x200D;
	 smi_write_reg(port_num, reg_addr, w_data);


        reg_addr = 7;
	 w_data = 0x100D;
	 smi_write_reg(port_num, reg_addr, w_data);

	 reg_addr = 9;
	 smi_read_reg(port_num, reg_addr, &r_data);

        printf("Current 88E1680 tx config for port %d: 0x%x\n", port_num, r_data);
      
	 
    } else {
        printf("Unsupported port number %d, please enter 0/4/8/12/16/20 port number\n");
	 return(FAILED);
    }

    return(PASSED);

}


/*******************************************************************************
 *
 * Function    : nanook_get_pci_base_addr 
 * Description : Function to get ethernet switch device pci base addr
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
unsigned long 
nanook_get_pci_base_addr (void)
{
    GT_UINTPTR ppregs_base, config_base, exregs_base;
    
    nim_dm_cpss_get_pciemap_ex(&ppregs_base, &config_base, &exregs_base);
#ifdef DEBUG
    printf("ppregs_base = %lx, config_base = %lx, exregs_base =%lx\n", ppregs_base, config_base, exregs_base);
    printf("ppregs_base = %#lx,  vendor_id = %#x\n", ppregs_base,
	   *(unsigned int *)(config_base + 0x50));
        printf("ppregs_base = %#lx,  WIN0 = %#x\n", ppregs_base,
	   *(unsigned int *)(config_base + 0x41820));
#endif
    return config_base;
}


/*******************************************************************************
 *
 * Function    : diag_esw_ext_lpbk_test
 * Description : Function to do external loopback
 * Inputs      : regnum - Register Address
 *               regval - Data to be written to ESW register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_ext_lpbk_test (void) 
{
        return (eth_pkt_txrx(inface_lan0p1, LPBKTEST_PKT_CNT, FALSE));
}


/*******************************************************************************
 *
 * Function    : diag_esw_98dxc323_dev_create
 * Description : Function to create 98DXC323 Device Object
 * Inputs      : esw_obj - Pointer of 98DXC323 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_98dxc323_dev_create (dev_98dxc323_object_t *esw_obj)
{
    dev_object_t *dev = (dev_object_t *)esw_obj;

    /* Create common device object */
    mrv98dxc323_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    esw_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    esw_obj->callout_fvt->sgmii_lpbk_test = diag_esw_ext_lpbk_test;
	
    esw_obj->callout_fvt->cpss_driver_init = xcat3_cpss_driver_init;
    esw_obj->callout_fvt->cpss_device_init = xcat3_cpss_device_init;
    esw_obj->callout_fvt->reg_pci_rd = xcat3_reg_pci_read;
    esw_obj->callout_fvt->reg_pci_wr = xcat3_reg_pci_write;
    esw_obj->callout_fvt->reg_config_rd = xcat3_reg_config_read;
    esw_obj->callout_fvt->reg_config_wr = xcat3_reg_config_write;
    esw_obj->callout_fvt->led_init = xcat3_led_init;
    esw_obj->callout_fvt->smi_phy_init = xcat3_smi_phy_init;
    esw_obj->callout_fvt->global_enable_pve = xcat3_global_enable_pve;
    esw_obj->callout_fvt->global_disable_pve = xcat3_global_disable_pve;
    esw_obj->callout_fvt->set_port_pve = xcat3_set_port_pve;
    esw_obj->callout_fvt->clear_port_pve = xcat3_clear_port_pve;
    esw_obj->callout_fvt->set_port_pve_singel_direction = xcat3_set_port_pve_single_direction;
    esw_obj->callout_fvt->clear_port_pve_singel_direction = xcat3_clear_port_pve_single_direction;
    esw_obj->callout_fvt->port_enable = xcat3_port_enable;
    esw_obj->callout_fvt->port_disable = xcat3_port_disable;
    esw_obj->callout_fvt->port_mac_loopback_enable = xcat3_port_mac_loopback_enable;
    esw_obj->callout_fvt->port_mac_loopback_disable = xcat3_port_mac_loopback_disable;
    esw_obj->callout_fvt->soft_reset = xcat3_soft_reset;
    esw_obj->callout_fvt->vlan_add = xcat3_vlan_add;
    esw_obj->callout_fvt->vlan_port_add = xcat3_vlan_port_add;
    esw_obj->callout_fvt->vlan_port_del = xcat3_vlan_port_del;
    esw_obj->callout_fvt->vlan_port_show = xcat3_vlan_port_show;
    esw_obj->callout_fvt->force_link_down_en = xcat3_force_link_down_en;
    esw_obj->callout_fvt->force_link_pass_en = xcat3_force_link_pass_en;
    esw_obj->callout_fvt->xcat3_all_reg_test = xcat3_all_reg_test;
    esw_obj->callout_fvt->pcie_config_read = xcat3_pcie_config_read;
    esw_obj->callout_fvt->pcie_config_write = xcat3_pcie_config_write;
    esw_obj->callout_fvt->set_10g_kr_test_mode = xcat3_cpss_set_test_mode;
    esw_obj->callout_fvt->serdes_tx_config_read = xcat3_serdes_tx_config_read;
    esw_obj->callout_fvt->serdes_tx_config_write = xcat3_serdes_tx_config_write;
    esw_obj->callout_fvt->phy_tx_config_read = xcat3_phy_tx_config_read;
    esw_obj->callout_fvt->phy_tx_config_write = xcat3_phy_tx_config_write;
	
    
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_phy_88e1680_dev_create
 * Description : Function to create 88E1680 Device Object
 * Inputs      : esw_obj - Pointer of 88E1680 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_phy_88e1680_dev_create (dev_88e1680_object_t *phy_1680_obj)
{
    dev_object_t *dev = (dev_object_t *)phy_1680_obj;

    /* Create common device object */
    mrv88e1680_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    phy_1680_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */

    phy_1680_obj->callout_fvt->phy_read_reg = phy_read_reg;
    phy_1680_obj->callout_fvt->phy_write_reg = phy_write_reg;
    phy_1680_obj->callout_fvt->phy_mad_load_driver = phy_mad_load_driver;
    phy_1680_obj->callout_fvt->phy_mad_unload_driver = phy_mad_unload_driver;
    phy_1680_obj->callout_fvt->phy_mad_disable_int = phy_mad_disable_int;
    phy_1680_obj->callout_fvt->phy_mad_display_reg = phy_mad_display_reg;
    phy_1680_obj->callout_fvt->phy_mad_soft_reset = phy_mad_soft_reset;
    phy_1680_obj->callout_fvt->phy_mad_set_test_mode = phy_mad_set_test_mode;
    phy_1680_obj->callout_fvt->phy_mad_set_phy_enable = phy_mad_set_phy_enable;
    phy_1680_obj->callout_fvt->phy_mad_hw_page_reset = phy_mad_hw_page_reset;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : diag_esw_init
 * Description: Function to init Nanook switch(Marvell 98DXC323 & PHY Marvell 88E1680).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_esw_init (void)
{
		
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;
	
    int    ctr = 0, ix =0, rc;
    int    cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    MAD_DEV * mad_dev;
    int    phy_addr;
    uint   reset_data;

    if (marvell_cpssPpInit_xcat3 == TRUE) {

	 cpssDxChCfgDevRemove(NANOOK_AC3_CPSS_DEV);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("0. cpssDxChCfgDevRemove Done\n");
        }

        /* Call 88e6180 MAD driver start function */
        for (ix = 0; ix < NANOOK_1680_GROUP_NUM; ix++) {
	     phy_addr = phy_dev_88e1680_group_start_addr[ix];
            mad_dev = &phy_dev_88e1680[ix];
            mdUnloadDriver(mad_dev);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("1. mdUnloadDriver Done\n");
        }

        nim_dm_cpss_extserv_cleanup_ex();

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("2. extserve_cleanup_ex Done\n");
        }

        system(ETH_RM_AC3_NIM_DM_MODULE);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("3. rmmod nim_dm Done\n");
        }

        diag_esw_remove_pcie_device();

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("4. remove pcie device Done\n");
        }
	
        /* Put switch/phy in Reset. */

        reset_data = (FPGA_DEV_RST_AC3_RST | FPGA_DEV_RST_88E1680_2_RST |
                         FPGA_DEV_RST_88E1680_1_RST | FPGA_DEV_RST_88E1680_0_RST);

        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, reset_data, TRUE,
                          ESW_WAIT_200MS) != PASSED) {
            printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
            return (FAILED);
        }
        //msleep(ESW_WAIT_500MS);

        /* Release switch/phy from Reset. */
        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, reset_data, FALSE,
            ESW_WAIT_200MS) != PASSED) {
            printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(ESW_WAIT_500MS);

        system(PCI_RESCAN);
        msleep(ESW_WAIT_500MS);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("5. Reset AC3/88E1680 Done\n");
        }

        system(ETH_INSMOD_AC3_NIM_DM_MODULE);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("6. insmod nim_dm Done\n");
        }

    }

    if (nim_dm_cpss_extserv_init_ex(0, 1) != PASSED) {
        return (FAILED);
    }
	
    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 init function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_init((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
            printf("%s:%d Failed to ESW xcat3 init.",
                   __FUNCTION__, __LINE__, ctr);
            goto _exit_98dxc323;
    }

    /* Call 98dxc323 xcat3 init function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_init_post((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
            printf("%s:%d Failed to ESW xcat3 init post.",
                   __FUNCTION__, __LINE__, ctr);
            goto _exit_98dxc323;
    }

    /* Need to add phy out of reset here. */
    /* For Nanook platform, 3 * 1680 phy reset are controlled by FPGA. */
    // To-do

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    /* Call 88e6180 MAD driver start function */
    for (ix = 0; ix < NANOOK_1680_GROUP_NUM; ix++) {
	 phy_addr = phy_dev_88e1680_group_start_addr[ix];
        mad_dev = &phy_dev_88e1680[ix];
        if (phy_88e1680_obj_p->callin_fvt->phy_start_mad_driver((dev_object_t *)phy_88e1680_obj_p, mad_dev, phy_addr)!= PASSED) {
                printf("%s:%d Failed to start phy mad driver for 88e1680.",
                       __FUNCTION__, __LINE__, ctr);
                goto _exit;
        }
    }

    /* Config 1680 for disable matrix setting and LED setting */
    rc = xcat3_port_phy_1680_init();
    if (rc == FAILED) {
        cterr('f', 0, "%s: xcat3_port_phy_1680_init failed",__func__);
        goto _exit_98dxc323;
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);


}


/*******************************************************************************
 *
 * Function   : diag_esw_exit
 * Description: Function to exit Nanook switch(Marvell 98DXC323 & PHY Marvell 88E1680).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_esw_exit (void)
{

    int ix;
    MAD_DEV * mad_dev;
    int    phy_addr;
    uint   reset_data;

    cpssDxChCfgDevRemove(NANOOK_AC3_CPSS_DEV);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("0. cpssDxChCfgDevRemove Done\n");
        }

        /* Call 88e6180 MAD driver start function */
        for (ix = 0; ix < NANOOK_1680_GROUP_NUM; ix++) {
	     phy_addr = phy_dev_88e1680_group_start_addr[ix];
            mad_dev = &phy_dev_88e1680[ix];
            mdUnloadDriver(mad_dev);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("1. mdUnloadDriver Done\n");
        }

        nim_dm_cpss_extserv_cleanup_ex();

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("2. extserve_cleanup_ex Done\n");
        }

        system(ETH_RM_AC3_NIM_DM_MODULE);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("3. rmmod nim_dm Done\n");
        }

        diag_esw_remove_pcie_device();

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("4. remove pcie device Done\n");
        }
	
        /* Put switch/phy in Reset. */

        reset_data = ( FPGA_DEV_RST_AC3_RST | FPGA_DEV_RST_88E1680_2_RST |
                         FPGA_DEV_RST_88E1680_1_RST | FPGA_DEV_RST_88E1680_0_RST);

        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, reset_data, TRUE,
                          ESW_WAIT_200MS) != PASSED) {
            printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(ESW_WAIT_500MS);

        /* Release switch/phy from Reset. */
        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, reset_data, FALSE,
            ESW_WAIT_200MS) != PASSED) {
            printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(ESW_WAIT_500MS);

        system(PCI_RESCAN);
        msleep(ESW_WAIT_500MS);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("5. Reset AC3/88E1680 Done\n");
        }

    marvell_cpssPpInit_xcat3 = FALSE;

    return (PASSED);

}



/*******************************************************************************
 *
 * Function   : diag_reset_esw_to_default
 * Description: Function to reset Nanook switch and re-init it.
 * Inputs     : quiet_opt - To print message(opt = FALSE) or not(opt = TRUE)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_reset_esw_to_default (int quiet_opt) {

    uint rc;

    rc = diag_esw_init();
    return rc;

}


int 
diag_config_port_speed (uint dev_num, uint port_num, uint target_speed) {

    uint32_t rc = GT_OK;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORTS_BMP_STC      initPortsBmp;       /* bitmap of ports to init */
    GT_BOOL is_link_up;

    rc = xcat3_port_disable(dev_num, port_num);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed xcat3_port_disable 0x%0x, "
		   "dev_num %d, port %d\n", rc, dev_num, port_num);
        return (rc);
    }

    if (xcat3_port_force_link_set(dev_num, LINKUP, port_num, FALSE) != OK) {
	  cterr('f',0,"Failed xcat3_port_force_link_set()");
	  return (FAILED);
    }

    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, TRUE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

    if (target_speed == SPD_1000MBPS) {
        speed = CPSS_PORT_SPEED_1000_E;
    } else if (target_speed == SPD_100MBPS) {
        speed = CPSS_PORT_SPEED_100_E;
    } else if (target_speed == SPD_10MBPS) {
        speed = CPSS_PORT_SPEED_10_E;
    } else {
        cterr('f',0,"Unsupported speed.\n");
        return (FAILED);
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, port_num);


    rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	 return rc;
    }

    rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedSet() for port %d,"
	          " err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortDuplexModeSet(dev_num, port_num, CPSS_PORT_FULL_DUPLEX_E);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexModeSet() for port %d,"
	          " err code %d", port_num, rc);
       return rc;
    }

    if (xcat3_port_force_link_set(dev_num, LINKDOWN, port_num, FALSE) != OK) {
        cterr('f',0,"Failed xcat3_port_force_link_set()");
        return (FAILED);
    }

    rc = cpssDxChPortEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	 cterr('f', 0," Error failed port disable:cpssDxChPortEnableSet err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port_num);
	 return (rc);
    }

    rc = cpssDxChPortMacResetStateSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
	 cterr('f',0," Error failed port reset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port_num);
	 return (rc);
    }

    rc = cpssDxChPortMacResetStateSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	 cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port_num);
	 return (rc);
    }

    rc = cpssDxChPortEnableSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
	 cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		  " dev_num %d, port %d\n", rc, dev_num, port_num);
	 return (rc);
    }

    rc = xcat3_port_enable(dev_num, port_num);
    if (rc != GT_OK) {
	 cterr('f',0,"Error failed xcat3_port_enable 0x%0x, "
		   "dev_num %d, port %d\n", rc, dev_num, port_num);
	 return (rc);
    }

    if (xcat3_port_force_link_set(dev_num, LINKUP, port_num, TRUE) != OK) {
	 cterr('f',0,"Failed xcat3_port_force_link_set()");
	 return (FAILED);
    }

    if (cpssDxChPortLinkStatusGet(dev_num, port_num, &is_link_up) != OK) {
	 cterr('f',0,"Failed cpssDxChPortLinkStatusGet()");
	 return (FAILED);
    }
    if (is_link_up == GT_FALSE) {
	 cterr('f',0,"Link is not up for port %d", port_num);
	 return (FAILED);
    }

    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: diag_port_power_control
 *
 *  Description: Xcat3 port power control
 *
 *  Input: dev_num - device number
 *         port_num - port number
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
int 
diag_port_power_control(uint dev_num, uint port_num, uint pwr_ctrl)
{
    uint32_t rc = GT_OK;
    CPSS_PORT_SPEED_ENT speed;
    uint32_t port_mode;

    CPSS_PORTS_BMP_STC      initPortsBmp;       /* bitmap of ports to init */

    if (port_num >= GE0_XCAT3_PORT) {
	
	port_mode = CPSS_PORT_INTERFACE_MODE_KR_E;
	speed = CPSS_PORT_SPEED_10000_E;

    } else {
	port_mode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;
	speed = CPSS_PORT_SPEED_1000_E;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, port_num);

    if (pwr_ctrl == ESW_PORT_PWR_UP) {
	 //printf("ESW_PORT_PWR_UP for port %d\n", port_num);
        rc = cpssDxChPortModeSpeedSet(dev_num, initPortsBmp, GT_TRUE, port_mode, speed);
        if (rc != GT_OK) {
	     cterr('f',0,"Failed to call cpssDxChPortModeSpeedSet() for port %d,"
	         " err code %d", port_num, rc);
            return rc;
        }

    } else {
        //printf("ESW_PORT_PWR_DOWN for port %d\n", port_num);
        rc = cpssDxChPortModeSpeedSet(dev_num, initPortsBmp, GT_FALSE, port_mode, speed);
        if (rc != GT_OK) {
	     cterr('f',0,"Failed to call cpssDxChPortModeSpeedSet() for port %d,"
	         " err code %d", port_num, rc);
            return rc;
        }
    }

    return (GT_OK);

}



/*******************************************************************************
 *
 * Function   : diag_esw_remove_pcie_device
 * Description: remove the pcie device
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_esw_remove_pcie_device (void)
{
/*
# lspci -nn | grep -i 11AB:F41B
0e:00.0 Ethernet controller: Marvell Technology Group Ltd. Device f41b (rev 04)
# find /sys -name *0e:00.0
/sys/bus/pci/devices/0000:0e:00.0
/sys/bus/pci/drivers/pcieport/0000:0e:00.0
/sys/devices/pci0000:00/0000:00:0e.0/0000:0e:00.0
# echo 1 >  /sys/devices/pci0000:00/0000:00:0e.0/remove	
*/

    system("echo 1 >  /sys/bus/pci/devices/0000:1c:00.0/remove");
	system("echo 1 > /sys/bus/pci/devices/0000\\:00\\:0f.0/remove");
    msleep(ESW_WAIT_1000MS);

}


/******************************************************************************
 *
 * Function   :	diag_esw_all_phy_led_on
 * Description: perform esw all phy led on.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_all_phy_led_on (void)
{
    int ix, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;


    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    /* Force 88E1680 phy led on */
    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	 port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
        /* Call 88e1680 PHY led on function */
        if (phy_88e1680_obj_p->callin_fvt->led_on((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to force led on for phy port %d", ix);
            goto _exit;
        }
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);

}


/******************************************************************************
 *
 * Function   :	diag_esw_all_phy_led_off
 * Description: perform esw all phy led off.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_all_phy_led_off (void)
{
    int ix, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;


    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    /* Force 88E1680 phy led off */
    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	 port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
        /* Call 88e1680 PHY led off function */
        if (phy_88e1680_obj_p->callin_fvt->led_off((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to force led on for phy port %d", ix);
            goto _exit;
        }
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);

}


/*-------------------------------------------------
 * $Log: diag_esw_lib.c,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

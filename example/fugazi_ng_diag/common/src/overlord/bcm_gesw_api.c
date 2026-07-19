/* $Id: bcm_gesw_api.c,v 1.11 2018/05/18 09:24:50 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw_api.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_api.c - User API for broadcom switch setup. This API is
 *                  based on the Xformer's GE switch API.
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include "types.h"
#include "common.h"
#include "queryflags.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "bcm_gesw_defs.h"
#include "dash_fpga.h"
#include "plat_defs.h"
#include "linux_pciutils.h"


#undef DEBUG
#define DEBUG  0

/* BCM GESW BCM56321L (Helix) port mapping on all Victory platforms
 */
static gesw_port_asgn_t victory_bcm56321_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        10,            BCM_PTYPE_GE,        "ge8" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        9,             BCM_PTYPE_GE,        "ge7" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        12,            BCM_PTYPE_GE,        "ge10" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        11,            BCM_PTYPE_GE,        "ge9" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE0,        14,            BCM_PTYPE_GE,        "ge12" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE1,        13,            BCM_PTYPE_GE,        "ge11" },
    /* NG VM (ISC) */
    { TGT_DEV_NGVM,   -1,           NGIO_GE0,        16,            BCM_PTYPE_GE,        "ge14" },
    { TGT_DEV_NGVM,   -1,           NGIO_GE1,        15,            BCM_PTYPE_GE,        "ge13" },
    /* NG SM 1-2 */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE0,        6,             BCM_PTYPE_GE,        "ge4" },
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE1,        5,             BCM_PTYPE_GE,        "ge3" },

#ifdef UTAH /* Utah */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_XAUI,       27,            BCM_PTYPE_XE,        "xe0" },
#else /* Overlord/Juno */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_XAUI,       27,            BCM_PTYPE_XE,        "xe1" },
#endif

    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE0,        8,             BCM_PTYPE_GE,        "ge6" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE1,        7,             BCM_PTYPE_GE,        "ge5" },

#ifdef UTAH /* Utah */
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_XAUI,       28,            BCM_PTYPE_XE,        "xe1" },
#else /* Overlord/Juno */
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_XAUI,       28,            BCM_PTYPE_XE,        "xe2" },
#endif

#ifdef UTAH /* Utah */
    /* Control plane, CPU 3 */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT3, 2,             BCM_PTYPE_GE,        "ge0" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              26,            BCM_PTYPE_XE,        "hg0" },
#else /* Overlord/Juno */
    /* Control plane, CPU 1-3 */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT1, 2,             BCM_PTYPE_GE,        "ge0" },
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT2, 3,             BCM_PTYPE_GE,        "ge1" },
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT3, 4,             BCM_PTYPE_GE,        "ge2" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              26,            BCM_PTYPE_XE,        "xe0" },
#endif
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/* BCM GESW BCM53403 (Greyhound) port mapping on Utah platform
 */
static gesw_port_asgn_t utah_bcm53403_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        16,            BCM_PTYPE_10GKR,     "xe0" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        3,             BCM_PTYPE_GE,        "ge0" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        17,            BCM_PTYPE_10GKR,     "xe1" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        4,             BCM_PTYPE_GE,        "ge1" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE0,        18,            BCM_PTYPE_10GKR,     "xe2" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE1,        7,             BCM_PTYPE_GE,        "ge4" },
    /* NG VM (ISC) */
    { TGT_DEV_NGVM,   -1,           NGIO_GE0,        5,             BCM_PTYPE_GE,        "ge2" },
    { TGT_DEV_NGVM,   -1,           NGIO_GE1,        6,             BCM_PTYPE_GE,        "ge3" },
    /* NG SM 1-2 */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE0,        20,            BCM_PTYPE_10GKR,     "xe4" },
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE1,        9,             BCM_PTYPE_GE,        "ge6" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE0,        19,            BCM_PTYPE_10GKR,     "xe3" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE1,        10,            BCM_PTYPE_GE,        "ge7" },
    /* Control plane, CPU 3 */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT3, 8,             BCM_PTYPE_GE,        "ge5" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              2,             BCM_PTYPE_XE ,       "hg0" },
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/* BCM GESW BCM53404 (Greyhound) port mapping on Sword and Dagger platform
 */
static gesw_port_asgn_t sword_dagger_bcm53404_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        11,            BCM_PTYPE_10GKR,     "xe0" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        3,             BCM_PTYPE_GE,        "ge0" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        12,            BCM_PTYPE_10GKR,     "xe1" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        4,             BCM_PTYPE_GE,        "ge1" },
    /* NG VM (ISC) */
    { TGT_DEV_NGVM,   -1,           NGIO_GE0,        5,             BCM_PTYPE_GE,        "ge2" },
    { TGT_DEV_NGVM,   -1,           NGIO_GE1,        6,             BCM_PTYPE_GE,        "ge3" },
    /* NG SM 1-2 */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE0,        13,            BCM_PTYPE_10GKR,     "xe2" },
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE1,        9,             BCM_PTYPE_GE,        "ge6" },
    /* Control plane, CPU 3 */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT3, 8,             BCM_PTYPE_GE,        "ge5" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              2,             BCM_PTYPE_XE ,       "hg0" },
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/* BCM GESW BCM53403 (Greyhound) port mapping on Neptune platform
 */
static gesw_port_asgn_t neptune_bcm53403_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        19,            BCM_PTYPE_10GKR,     "xe4" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        4,             BCM_PTYPE_GE,        "ge1" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        5,             BCM_PTYPE_GE,        "ge2" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        6,             BCM_PTYPE_GE,        "ge3" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE0,        7,             BCM_PTYPE_GE,        "ge4" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE1,        8,             BCM_PTYPE_GE,        "ge5" },
    /* NG SM 1-3 */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE0,        17,            BCM_PTYPE_10GKR,     "xe2" },
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE1,        9,             BCM_PTYPE_GE,        "ge6" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE0,        18,            BCM_PTYPE_10GKR,     "xe3" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE1,        10,            BCM_PTYPE_GE,        "ge7" },
    { TGT_DEV_NGSM,   NGSM3_SLOT,   NGIO_GE0,        16,            BCM_PTYPE_10GKR,     "xe1" },
    { TGT_DEV_NGSM,   NGSM3_SLOT,   NGIO_GE1,        15,            BCM_PTYPE_GE,        "ge8" },
    /* Control plane, CPU */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT1, 20,            BCM_PTYPE_10GKR,     "xe5" },
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT2, 3,             BCM_PTYPE_GE,        "ge0" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              2,             BCM_PTYPE_XE ,       "xe0" },
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/* BCM GESW BCM53403 (Greyhound) port mapping on Triton and Proteus platform
 */
static gesw_port_asgn_t triton_proteus_bcm53403_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        19,            BCM_PTYPE_10GKR,     "xe3" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        4,             BCM_PTYPE_GE,        "ge1" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        5,             BCM_PTYPE_GE,        "ge2" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        6,             BCM_PTYPE_GE,        "ge3" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE0,        7,             BCM_PTYPE_GE,        "ge4" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE1,        8,             BCM_PTYPE_GE,        "ge5" },
    /* NG VM (ISC) */
    { TGT_DEV_NGVM,   -1,           NGIO_GE0,        11,            BCM_PTYPE_GE,        "ge8" },
    { TGT_DEV_NGVM,   -1,           NGIO_GE1,        12,            BCM_PTYPE_GE,        "ge9" },
    /* NG SM 1-2 */
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE0,        17,            BCM_PTYPE_10GKR,     "xe1" },
    { TGT_DEV_NGSM,   NGSM1_SLOT,   NGIO_GE1,        9,             BCM_PTYPE_GE,        "ge6" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE0,        18,            BCM_PTYPE_10GKR,     "xe2" },
    { TGT_DEV_NGSM,   NGSM2_SLOT,   NGIO_GE1,        10,            BCM_PTYPE_GE,        "ge7" },
    /* Control plane, CPU */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT1, 20,            BCM_PTYPE_10GKR,     "xe4" },
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT2, 3,             BCM_PTYPE_GE,        "ge0" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              2,             BCM_PTYPE_XE ,       "xe0" },
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/* BCM GESW BCM53403 (Greyhound) port mapping on Neso platform
 */
static gesw_port_asgn_t neso_bcm53403_gesw_port_map[] = {
    /* slot_type,     slot_num,     ngio_port_num, bcm_port_num, bcm_port_type, bcm_port_name */
    /* NG WIC 1-3 */
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE0,        19,            BCM_PTYPE_10GKR,     "xe3" },
    { TGT_DEV_NGWIC,  NGWIC1_SLOT,  NGIO_GE1,        4,             BCM_PTYPE_GE,        "ge1" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE0,        5,             BCM_PTYPE_GE,        "ge2" },
    { TGT_DEV_NGWIC,  NGWIC2_SLOT,  NGIO_GE1,        6,             BCM_PTYPE_GE,        "ge3" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE0,        7,             BCM_PTYPE_GE,        "ge4" },
    { TGT_DEV_NGWIC,  NGWIC3_SLOT,  NGIO_GE1,        8,             BCM_PTYPE_GE,        "ge5" },
    /* NG VM (ISC) */
    { TGT_DEV_NGVM,   -1,           NGIO_GE0,        11,            BCM_PTYPE_GE,        "ge8" },
    { TGT_DEV_NGVM,   -1,           NGIO_GE1,        12,            BCM_PTYPE_GE,        "ge9" },
    /* Control plane, CPU */
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT1, 20,            BCM_PTYPE_10GKR,     "xe4" },
    { TGT_DEV_CPU,    -1,           CPU_SGMII_PORT2, 3,             BCM_PTYPE_GE,        "ge0" },
    /* Data plane (DP) */
    { TGT_DEV_DP,     -1,           -1,              2,             BCM_PTYPE_XE ,       "xe0" },
    { -1,             -1,           -1,              -1,            -1,                  "" },
};

/*******************************************************************/
/*****     Code for GESW API     ***********************************/
/*******************************************************************/

/*
 * Function: is_bcm
 * Check the BCM GESW dev_id exist on the platform
 */
int is_bcm (int dev_id)
{
    struct pci_dev *dev;  
    ushort vendor_id, device_id; 

    vendor_id = BCM_GESW_VID;
    device_id = dev_id;

    dev = diag_pci_get_device(vendor_id, device_id, NULL);

    if (dev == NULL) {
        return FALSE;
    } else {
        dev = NULL;
        return TRUE;
    }
}

/*
 * Function: is_bcm_greyhound_wrapper
 * Check if the BCM GESW is Greyhound
 */
int is_bcm_greyhound(void)
{
    return(is_bcm(BCM53403_DID) || is_bcm(BCM53404_DID));
}

/*
 * Function: get_bcm_port_assignment
 * Returns the pointer of the table entry which matches the provided
 * slot and port info of target device connected to the BCM GESW.
 */
static gesw_port_asgn_t *get_bcm_port_assignment(gesw_port_asgn_t *map_p,
						 int slot, 
						 int tgt_device, 
						 int local_port)
{
    gesw_port_asgn_t *mptr = (void *)(-1);
    char buffer[8];

    switch(tgt_device) {
    case TGT_DEV_CPU:
        sprintf(buffer, "CPU");
	break;
    case TGT_DEV_NGSM:
        sprintf(buffer, "NGSM");
	break;
    case TGT_DEV_NGWIC:
        sprintf(buffer, "NGWIC");
	break;
    case TGT_DEV_NGVM:
        sprintf(buffer, "NGVM");
	break;
    case TGT_DEV_DP:
        sprintf(buffer, "DP");
	break;
    default:
        sprintf(buffer, "Unknown");
	break;
    }

    mptr = map_p;

    while (mptr->slot_type >= 0) {
        if (mptr->slot_type == tgt_device) {
	    if ((mptr->slot_num == slot) || (mptr->slot_num == -1)) {
	        if ((mptr->ngio_port_num == local_port) || (mptr->ngio_port_num == -1)) {
		    return(mptr);
		}
	    }
	}
	mptr++;
    }

    printf("%s: GESW port assignment map has no match for target device %s slot %d port %d\n",
	   __FUNCTION__, buffer, slot, local_port);
    mptr = (void *)(-1);
    return(mptr);
}

/*------------------------------------------------------------------
 *  
 * Function: ovld_get_ge_sw_port_num
 *
 * This function returns the GE Switch port number (2 - 16, 26 -28)
 * given the slot number and the target device (CP, NGWIC, NGSM, NGVM, DP).
 *
 * In the HW spec, slot numbering for NGSM and NGWICs starts with 1.
 * Each NGIO has 2 ethernet ports (E0 and E1) which is passed
 * in the function as local_port.
 *
 * Each NGSM also has 1 xaui port.
 *
 * A slot number check is made for the following target devices
 * and an error is flagged if the slot number exceeds the value
 * given for the respective target device:
 * MAX_NUM_NGSM_SLOTS, MAX_NUM_VM_SLOTS, MAX_NUM_NGWIC_SLOTS
 *  
 * The following is the GE switch port mapping for the connections
 * on Overlord:
 * Broadcom GE switch port to connection mapping:
 *
 *   BCM name | BCM port # | platform assignment
 *   cmic       0            Host CPU PCIe port
 *   reservedd  1            not used
 * if (platform is O2 or Juno) {
 *   ge0        2            CPU x86 sgmii 1
 *   ge1        3            CPU x86 sgmii 2
 *   ge2        4            CPU x86 sgmii 3
 * } else (platform is USD) {
 *   ge0        2            CPU x86 sgmii 3
 *   ge1        3            not used
 *   ge2        4            not used
 * }
 *   ge3        5            NGSM1 E1 port
 *   ge4        6            NGSM1 E0 port
 *   ge5        7            NGSM2 E1 port
 *   ge6        8            NGSM2 E0 port
 *   ge7        9            NGWIC1 E1 port
 *   ge8        10           NGWIC1 E0 port
 *   ge9        11           NGWIC2 E1 port
 *   ge10       12           NGWIC2 E0 port
 *   ge11       13           NGWIC3 E1 port
 *   ge12       14           NGWIC3 E0 port
 *   ge13       15           NGVM E1 port
 *   ge14       16           NGVM E0 port
 *   ge15       17           reserved
 *   ge16       18           reserved
 *   xe0        26           DP (cavium cpu)
 *   xe1        27           NMSM1
 *   xe2        28           NGSM2
 *
 * Input: 
 *	slot - target device slot number (SM and WIC starts with 1,
 *             others starts with 0)
 *      tgt_device - target devices are:
 *	    TGT_DEV_CPU, TGT_DEV_NGWIC, TGT_DEV_NGVM, TGT_DEV_NGSM
 *      local_port - CPU 2 sgmii ports, SM and WIC has 2 ports (E0, E1)
 *
 * Output: ge switch port number or -1 if error
 *
 *------------------------------------------------------------------
 */
int
ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port)
{
    gesw_port_asgn_t *pmap_p;

    pmap_p = get_bcm_port_assignment(plat_gesw_port_map_get(), slot, tgt_device, local_port);

    if (pmap_p == (void*)(-1)) {
        return(-1);
    }
    else {
        return(pmap_p->bcm_port_num);
    }
}

/*
 * Function: is_plat_10gkr_capable
 * Check if the Victory platform has the greyhound GESW which is
 * 10G-KR capable.
 */
int is_plat_10gkr_capable(void)
{
    return(is_bcm_greyhound());
}

/*
 * Function: plat_gesw_port_map_get
 * Returns the pointer to the correct BCM GESW port map for
 * the platform.
 */
gesw_port_asgn_t *plat_gesw_port_map_get(void)
{
    if (is_usd_machines()) {
        if (is_bcm(BCM56321_DID)) {
	    return(victory_bcm56321_gesw_port_map);
	}
	else if (is_bcm(BCM53403_DID)) {
	    return(utah_bcm53403_gesw_port_map);
	}
	else {
	    return(sword_dagger_bcm53404_gesw_port_map);
	}
    }
    else if (is_neptune() || is_vg450()) {
        return(neptune_bcm53403_gesw_port_map);
    }
    else if (is_triton() || is_proteus()) {
        return(triton_proteus_bcm53403_gesw_port_map);
    }
    else if (is_neso()) {
        return(neso_bcm53403_gesw_port_map);
    }
    else {
        return(victory_bcm56321_gesw_port_map);
    }
}
	
/*
 * Function: get_gesw_pbmp
 * Returns the BCM port bit map (PBMP) of the BCM GESW
 * chip on the platform. The port_type determine if ge,
 * xe, or all port bit map is returned.
 */
void get_gesw_pbmp(uint64 *pbmp_buf, int port_type)
{
    uint64 pbmp = 0;
    gesw_port_asgn_t *mptr;

    mptr = plat_gesw_port_map_get();

    while (mptr->slot_type >= 0) {
        if (mptr->bcm_port_type & port_type) {
	    pbmp |= (1 << mptr->bcm_port_num);
	}
	mptr++;
    }

    *pbmp_buf = pbmp;
}

/*
 * Function: get_gesw_ptype
 * Returns the BCM port type (ge, xe, or gexe) of the specific port.
 */
int get_gesw_ptype(int port_num)
{
    gesw_port_asgn_t *mptr;

    mptr = plat_gesw_port_map_get();

    while (mptr->slot_type >= 0) {
        if (mptr->bcm_port_num == port_num) {
	  return(mptr->bcm_port_type);
	}
	mptr++;
    }
    return(0);
}

/*
 * Function: get_gesw_pname
 * Returns the BCM port name (ge0, ge1, etc) of the specific port.
 */
char *get_gesw_pname(int port_num)
{
    gesw_port_asgn_t *mptr;

    mptr = plat_gesw_port_map_get();

    while (mptr->slot_type >= 0) {
        if (mptr->bcm_port_num == port_num) {
	  return(mptr->bcm_port_name);
	}
	mptr++;
    }
    return(0);
}

void show_greyhound_port_assign(char plat)
{
    switch(plat) {
    case 's':
    case 'd':
        if (plat == 's') {
	    printf("Sword BCM53404:\n");
	}
	else {
	    printf("Dagger BCM53404:\n");
	}

	printf("NGIO   GE0   GE1   (bcm port#)\n" 
	       "----   ---   ---   -----------\n" 
	       "NIM 1  xe0   ge0   (11, 3)\n" 
	       "NIM 2  xe1   ge1   (12, 4)\n");
        if (plat == 's') {
	    printf("SM 1   xe2   ge6   (13, 9)\n");
	}
	printf("ISC    ge2   ge3   (5, 6)\n" 
	       "CPU    ge5    -    (8)\n");
        break;
    case 'u':
	printf("Utah BCM53403:\n"
	       "NGIO   GE0   GE1   (bcm port#)\n" 
	       "----   ---   ---   -----------\n" 
	       "NIM 1  xe0   ge0   (16, 3)\n" 
	       "NIM 2  xe1   ge1   (17, 4)\n" 
	       "NIM 3  xe2   ge4   (18, 7)\n" 
	       "SM 1   xe4   ge6   (20, 9)\n"
	       "SM 2   xe3   ge7   (19, 10)\n" 
	       "ISC    ge2   ge3   (5, 6)\n" 
	       "CPU    ge5    -    (8)\n");
        break;
    case 'n':
    case 't':
    case 'p':
    case 'o':
        if (plat == 'n') { printf("Neptune BCM53403:\n"); } else
	if (plat == 't') { printf("Triton BCM53403:\n"); } else
	if (plat == 'p') { printf("Proteus BCM53403:\n"); }
	else { printf("Neso BCM53403:\n"); }
      
	printf("NGIO   GE0   GE1   (bcm port#)\n" 
	       "----   ---   ---   -----------\n" 
	       "X86    xe5   ge0   (20, 3)\n"
	       "CVM    xe0    -    (2)\n"
	       "NIM 1  xe4   ge1   (19, 4)\n" 
	       "NIM 2  ge2   ge3   (5, 6)\n" 
	       "NIM 3  ge4   ge5   (7, 8)\n");
        if ((plat == 'n') || (plat == 't') || (plat == 'p')) {
	    printf("SM 1   xe2   ge6   (17, 9)\n"
		   "SM 2   xe3   ge7   (18, 10)\n");
	}
        if (plat == 'n') {
	    printf("SM 3   xe1   ge8   (16, 15)\n");
	}
        if ((plat == 't') || (plat == 'p') || (plat == 'o')) {
	    printf("ISC    ge8   ge9   (11, 12)\n");
	}
        break;
    default:
        printf("%s: Unknown input plat= %c\n", __FUNCTION__, plat);
	break;
    }
}

void show_gesw_port_assign(void)
{
    char plat;

    if (is_bcm_greyhound()) {
        if (is_usd_machines()) {
	    plat = getc_answer("Select 'u': Utah, 's': Sword, 'd': Dagger", "usd", 'u');
	    show_greyhound_port_assign(plat);
	}
	else {
	    /* Neptune and the Victory MLK machines
	     */
	    plat = getc_answer("Select 'n': Neptune, 't': Triton, 'p': Proteus, 'o': Neso", "ntpo", 'n');
	    show_greyhound_port_assign(plat);
	}
    }
    else {
        printf("Victory platform BCM56321:\n"
	       "NGIO   GE0   GE1   XAUI   (bcm port#)\n" 
	       "----   ---   ---   ----   -----------\n" 
	       "NIM 1  ge8   ge7          (10, 9)\n" 
	       "NIM 2  ge10  ge9          (12, 11)\n" 
	       "NIM 3  ge12  ge11         (14, 13)\n" 
	       "SM 1   ge4   ge3   xe1    (6, 5, 27)\n"
	       "SM 2   ge6   ge5   xe2    (8, 7, 28)\n" 
	       "ISC    ge14  ge13         (16, 15)\n" 
	       "CVMX   xe0                (26)\n"
	       "CPU1   ge0                (2)\n"
	       "CPU2   ge1                (3)\n"
	       "CPU3   ge2                (4)\n");
    }
}

/*
 * Function: is_gesw_1g_intf
 *     Returns TRUE if GESW port is 1G
 *     Return FALSE if GESW port is 10G-KR or XAUI intf. 
 *
 * slot_type - NGIO slot type, TGT_DEV_NGWIC or TGT_DEV_NGSM.
 * slot_num  - NGIO slot num, for example, NGWIC1_SLOT and NGSM2_SLOT. 
 * ngio_port_num - NGIO port num connect to GESW, NGIO_GE0 or NGIO_GE1.
 *
 */
int is_gesw_1g_intf (int slot_type, int slot_num, int ngio_port_num)
{
    gesw_port_asgn_t *mptr;

    mptr = plat_gesw_port_map_get();

    while (mptr->slot_type >= 0) {
        if ((mptr->slot_type == slot_type) && 
            (mptr->slot_num == slot_num) &&
            (mptr->ngio_port_num == ngio_port_num) ) {
           
            if (mptr->bcm_port_type == BCM_PTYPE_GE) {
                return (TRUE);  /* 1G */
            }  else {  
                return (FALSE); /* 10G-KR or XAUI */
            } 
        }
        mptr++;
    }

    printf("Not match for slot_type=%d, slot_num=%d, ngio_port_num=%d\n", 
           slot_type, slot_num, ngio_port_num);

    return (BCM_PTYPE_NONE);
}

/******** History ******** 
$Log: bcm_gesw_api.c,v $
Revision 1.11  2018/05/18 09:24:50  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.10.32.7  2017/11/27 06:08:40  leschen
Initial check in to support VG450.

Revision 1.10.32.6  2017/04/17 10:13:24  alpeng
add gesw ptype check

Revision 1.10.32.5  2017/03/14 21:30:47  ptong
Fix set_bcm_port_mode() to set xe5 for Neptune but xe4 for MLKs to connect to x86 10G-KR port

Revision 1.10.32.4  2017/03/14 06:32:32  alpeng
revert the port num for triton, it is HW issue

Revision 1.10.32.3  2017/03/13 07:40:18  alpeng
fixed trition NGVM port number

Revision 1.10.32.2  2016/12/13 00:23:40  ptong
Added GESW port list util, host port send pkt to GESW test support for Neptune

Revision 1.10.32.1  2016/06/10 18:25:38  ptong
Enhance GESW support to Neptune

Revision 1.10  2014/08/22 00:27:16  ptong
Fixed line loopback set up for Greyhound 10gKR ports

Revision 1.9  2014/08/06 21:29:29  ptong
Use BRCM SDK-LGA20140718 for Greyhound. Support autoneg on KR ports

Revision 1.8  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.7  2014/04/18 00:18:49  ptong
Prepare to support Greyhound 10G-KR bring-up

Revision 1.6  2014/04/02 21:47:51  ptong
Use port mapping between BCM and platform to support the new BCM chip

Revision 1.5  2014/03/13 20:32:42  ptong
Use table to map BCM GE ports to NGIO ports. Prepare for migration to use the Greyhound GESW

Revision 1.4  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.3  2013/09/06 20:36:41  ptong
Bug fix: return correct GESW port num for Utah

Revision 1.2  2013/08/13 00:07:00  hroni
support Rangeley control plane SGMII to GE same port loopback test

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.4  2012/09/07 22:50:00  ptong
Code clean-up

Revision 1.3  2012/05/31 14:24:40  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/

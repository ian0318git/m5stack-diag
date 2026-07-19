/* $Id: sm_slot.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/sm_slot.h,v $
 *-----------------------------------------------------------------------------
 * sm_slot.h - Contains defines for SM slots. 
 *
 * Mar. 2008 Alan O'Sullivan
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __SM_SLOT_H__
#define __SM_SLOT_H__

#include <inttypes.h>

#ifdef LINUX_KLM
typedef long PFI;
typedef long PFV;
typedef long PFT;
#endif

/*
 * This structure is used to map the SM Module id to the name and
 * diagnostic routine.
 */
struct sm_slot {
    char    *name;            /* Name of SM Module                */
    PFI     diag;             /* The diagnostic for this SM       */
    ushort  id;               /* The SM cookie id                 */
    PFI     diag_iface;       /* The interface test for this SM   */
};

#define NUM_SM_INTRS    8 

/*
 * Data Structure for the SM Module interface.
 */
typedef struct sm_interface_param_t_ sm_iface_t;
struct sm_interface_param_t_ {
    int    slot;                 /* SM Slot Number on Platform              */
    ulong  phy_base_addr;        /* Physical Base Address of PCIe Endpoint  */
    ulong  vir_base_addr;        /* Virtual Base Address of PCIe Endpoint   */
    uint32_t pci_base_addr;      /* PCI Base Address of PCIe Endpoint       */
    ushort msi_data;             /* MSI Data Value to enable intr's at EP   */
    ulong  msi_addr_hi;          /* MSI High Address, where msi data goes   */
    ulong  msi_addr_lo;          /* MSI Low Address, where msi data goes    */
    ushort cookie_id;            /* SM Cookie id                            */
    ulong  serial_num;           /* SM Card Serial Number                   */
    ushort board_rev;            /* SM card Board Revision                  */ 
    int    menu_display;         /* Flag set if submenu is invoked          */
    PFV    isr[NUM_SM_INTRS];    /* SM Interrupt handlers                   */
    int    test_type;            /* Test type, Full or interface test       */
    int msi_enable;
    int timeout;

} sm_interface_param_t;

#ifndef LINUX_KLM
/* Define for number of SM Slots in system. */
#define SM_MAX_IDS (sizeof(sm_slot_tbl) / sizeof(struct sm_slot))

/* Data Structure for SM information. */
extern struct sm_slot sm_info[];

/* SM interface data structure. */
extern sm_iface_t sm_params[], *sm_iface[];
extern int setup_sm_interrupt(int, uchar, int);
extern PFV put_sm_isr_vect (int sm_slot, uchar intr_level, PFV isr_vect);
#endif /* LINUX_KLM */

/* external prototypes */
extern int sm_vacant(void);
extern int pse2_sm_test(sm_iface_t *pse2_sm_iface);
extern int eagleeye_sm_test(sm_iface_t *eagleeye_sm_iface);
extern int apex_vega_carrier_test(sm_iface_t *vega_sm_iface);
extern int apex_zeta_test(sm_iface_t *sm);
#endif  /* __SM_SLOT_H__ */
/* ------ End of Module ------ */

/* ------ History ------------ 
$Log: sm_slot.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

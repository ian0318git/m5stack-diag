/* $Id: bcm_gesw_defs.h,v 1.2 2021/04/15 00:52:23 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/bcm_gesw_defs.h,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_defs.c -  For Support NIM Kalamata
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __BCM_GESW_DEFS_H__
#define __BCM_GESW_DEFS_H__

/* Cisco Overlord platform uses BCM56321L, which only implement
 * 17 SGMII and 3 XAUI ports from the BCM56321 (24 SGMII, 4 XAUI).
 * BCM SDK port bitmap (bcm_pbmp_t) defines bit-0 as the CPU port
 * (CMIC), and bit-1 as reserved. Bit-2 is the start of the 
 * SGMII ports so SGMII port 0 (physical) is mapped as port-2 in
 * the SDK. the XAUI ports are numbered right after
 * the SGMII ports, which is port 26-29.
 *
 * The SDK convention:
 * ge0 to ge23 are SGMII port 0 to SGMII port 23.
 * xe0 to xe3 are XAUI port 0 to XAUI port 3.
 * ge0 uses the port number value 2, and pbmp value 0x00000004.
 * xe0 uses the port number value 26, and pbmp value 0x04000000.
 */

/* PCIe Vendor ID and device ID
 */
#define BCM_GESW_VID               0x14e4
#define BCM56321_DID               0xb321
#define BCM53403_DID               0x8403
#define BCM53404_DID               0x8404

/* Define the marco for the numbers of ports
 */
#define BCM56321_SGMII_PORT_MAX    24
#define BCM56321_XAUI_PORT_MAX     4
#define BCM56321_PHYSICAL_PORT_MAX (BCM56321_SGMII_PORT_MAX + \
                                    BCM56321_XAUI_PORT_MAX)
#define BCM56321_LOGICAL_PORT_MAX  2  /* cpu and reserved */
#define BCM56321_PORT_ALL          (BCM56321_PHYSICAL_PORT_MAX + \
                                    BCM56321_LOGICAL_PORT_MAX)

/* The BCM56321L only implement the following ports in the
 * 56321 chip.
 */
#define BCM56321_SGMII_AVAIL       17
#define BCM56321_XAUI_AVAIL        3 

/* Bit mask according to the above info.
 * These bit masks are just mask use by diag software to
 * mark the ports, not the bcm_pbmp_t value used by the SDK,
 * so 0x1 is ge0 (port 0), and 0x1000000 is xe0 (port 24).
 */
#define BCM56321_SGMII_MASK_ALL    0x0ffffff /* 24 ge ports */
#define BCM56321_XAUI_MASK_ALL     0xf000000
#define BCM56321_PORT_MASK_ALL     (BCM56321_XAUI_MASK_ALL | BCM56321_SGMII_MASK_ALL)

#define BCM56321_SGMII_MASK_AVAIL  0x001ffff /* 17 ge ports */
#define BCM56321_XAUI_MASK_AVAIL   0x7000000
#define BCM56321_PORT_MASK_AVAIL   (BCM56321_XAUI_MASK_AVAIL | BCM56321_SGMII_MASK_AVAIL)

/* BCM SDK numbers the first physical port as port 2
 * in the bcm_pbmp_t system.
 */
#define BCM56321_CMIC_PORT_START         0
#define BCM56321_SGMII_PBMP_PORT_START   2
#define BCM56321_XAUI_PBMP_PORT_START    (BCM56321_SGMII_PBMP_PORT_START + \
                                          BCM56321_SGMII_PORT_MAX)

/* For the sake of manipulate the bit masks and get the
 * correlated port number in a for loop, we still treat 
 * the chip has all the ports in BCM56321.
 */
#define GESW_SGMII_PORT_MAX     BCM56321_SGMII_PORT_MAX
#define GESW_XAUI_PORT_MAX      BCM56321_XAUI_PORT_MAX
#define GESW_PHYSICAL_PORT_MAX  BCM56321_PHYSICAL_PORT_MAX
#define GESW_PORT_ALL           BCM56321_PORT_ALL

/* The Overlord design only use the following number of
 * ports on the BCM56321L
 */
#define GESW_SGMII_PORT_CNT     14 /* sgmii 0-13 used on the platform */
#define GESW_XAUI_PORT_CNT      3  /* xaui 0-2 */
#define GESW_SGMII_MASK      0x0003fff
#define GESW_XAUI_MASK       0x7000000
#define GESW_PORT_MASK      (GESW_SGMII_MASK | GESW_XAUI_MASK)

/* Cisco diaglinux GE port numbering scheme:
 * GESW physical port number starts with 0 and up, which
 * are mapped to 2 and up in the BCM SDK PBMP scheme.
 */
#define GESW_UNPBMP_SGMII_START     0
#define GESW_UNPBMP_XAUI_START      BCM56321_SGMII_PORT_MAX
#define GESW_UNPBMP_PORT_NUM(_p)    (_p)

/* Broadcom SGMII port 0 (ge0) is assigned as port 2 in the SDK
 * CMIS port (cpu port) start as port 0.
 */
#define GESW_PBMP_CMIC_START      BCM56321_CMIC_PORT_START
#define GESW_PBMP_SGMII_START     BCM56321_SGMII_PBMP_PORT_START
#define GESW_PBMP_XAUI_START      BCM56321_XAUI_PBMP_PORT_START
#define GESW_PBMP_PORT_NUM(_p)    ((_p) + BCM56321_SGMII_PBMP_PORT_START)

/* These PBMP masks represent all the used port
 * on the Overlord platform.
 */
#define GESW_CMIC_PBMP_MASK   (0x0000001)
#define GESW_SGMII_PBMP_MASK  (GESW_SGMII_MASK << BCM56321_SGMII_PBMP_PORT_START)
#define GESW_XAUI_PBMP_MASK   (GESW_XAUI_MASK << BCM56321_SGMII_PBMP_PORT_START)
#define GESW_PORT_PBMP_MASK   (GESW_PORT_MASK << BCM56321_SGMII_PBMP_PORT_START)

/* Supported loopback setting for the BCM switch ports
 */
#define GESW_LOOPBACK_NONE    0
#define GESW_MAC_LOOPBACK     1
#define GESW_LINE_LOOPBACK    2
#define GESW_2PORT_LOOPBACK   4 /* define to support xformer api */

/* BCM port types based on the BCM progamming guide.
 * Used in the port mapping table in bcm_gesw_api.c
 */
#define BCM_PTYPE_GE        0x1
#define BCM_PTYPE_10GKR     0x2
#define BCM_PTYPE_XE        0x4
#define BCM_PTYPE_ALL       (BCM_PTYPE_GE | BCM_PTYPE_10GKR | BCM_PTYPE_XE)
#define BCM_PTYPE_NONE      0xFF

/* Structure to map the NGIO GE port to the BCM GESW ports in
 * order to use the BCM API functions.
 */
typedef struct gesw_port_asgn_s {
    char slot_type;
    char slot_num;
    char ngio_port_num;
    char bcm_port_num;
    char bcm_port_type;
    char bcm_port_name[16];
} gesw_port_asgn_t;

/*
 * Externs
 */
extern int bcm_uid;

extern int bcm_gesw_config(void);
extern int set_gesw_line_loopback(int, int);
extern int ovld_get_ge_sw_port_num(int, int, int);
extern int bcm_gesw_ge_lpbk_get(int, int, int, int *);
extern int bcm_gesw_xaui_lpbk_set(int, int, int);
extern int bcm_gesw_ge_lpbk_set(int, int, int);
extern int bcm_gesw_ge_link_status_get(int, int, int *);
extern int bcm_gesw_xaui_lpbk_get(int, int, int, int *);
extern int get_bcm_shell_test_result(int, int *, int *);
extern void get_gesw_pbmp(uint64 *, int);
extern int get_gesw_ptype(int port_num);
extern char *get_gesw_pname(int port_num);
extern gesw_port_asgn_t *plat_gesw_port_map_get(void);
extern int is_plat_10gkr_capable(void);
extern int is_bcm (int dev_id);
extern int is_bcm_greyhound(void);
extern int cfg_10gkr_port(int port, int en_10gkr);
extern int is_gesw_1g_intf(int, int, int); 

#endif /* __BCM_GESW_DEFS_H__ */


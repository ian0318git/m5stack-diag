/* $Id: diag_sprom.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_sprom.h,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef _DIAG_SPROM_H__
#define _DIAG_SPROM_H__

#include <errno.h>
#include "types.h"

typedef uint16_t diag_smb_path_t;

typedef struct _diag_smb {
    char bus[64];
    diag_smb_path_t path;
    uint32_t addr;
    uint32_t offset;
    uint32_t asize;
    uint32_t rsize;
    uint32_t wdelay;
} diag_smb_dev_t;

typedef struct _diag_smb_acc {
    diag_smb_dev_t *smb_dev;
    int (*smb_rd) ( diag_smb_dev_t *, uint32_t, uint32_t, uint8_t * );
    int (*smb_wr) ( diag_smb_dev_t *, uint32_t, uint32_t, uint8_t * );
} diag_smb_acc_t;


typedef struct _diag_sprom {
    diag_smb_acc_t *smb_acc;
    void (* we)(void);
    void (* wd)(void);

    char *name;
    uint32_t type;

    int cache_valid;
    uint8_t *cache;
} diag_sprom_t;

typedef struct _diag_sprom_acc {
    diag_sprom_t *sprom_dev;

    int (* rd)(diag_sprom_t *, uint32_t, uint32_t, uint8_t *);
    int (* wr)(diag_sprom_t *, uint32_t, uint32_t, uint8_t *);
} diag_sprom_acc_t;

extern int diag_sprom_rd (diag_sprom_t *, uint32_t, uint32_t, uint8_t *);
extern int diag_sprom_wr (diag_sprom_t *, uint32_t, uint32_t, uint8_t *);

extern int sprom_rd (diag_smb_dev_t *sprom, uint32_t offset, uint32_t len, uint8_t *buf);
extern int sprom_wr (diag_smb_dev_t *sprom, uint32_t offset, uint32_t len, uint8_t *buf);
extern int sprom_psu_wr (diag_smb_dev_t *sprom, uint32_t offset, uint32_t len, uint8_t *buf);
extern int host_ipmi_sprom_rd (diag_smb_dev_t *sprom, uint32_t offset, uint32_t len, uint8_t *buf);
extern int host_ipmi_sprom_wr (diag_smb_dev_t *sprom, uint32_t offset, uint32_t len, uint8_t *buf);


#define sprom_printf printf
#define sprom_err( fmt, args... ) fprintf ( stderr, fmt, ## args )

#define SPROM_ERR_INVALID EINVAL
#define SPROM_ERR_NOMEM   ENOMEM
#define SPROM_ERR_NODEV   ENODEV
#define SPROM_ERR_IO      EIO

typedef enum {
    SPROM_PLATFORM_IOM = 0,
    SPROM_PLATFORM_IBMC= 1,
    SPROM_PLATFORM_FEX = 2,
    SPROM_PLATFORM_HOST= 3,
    SPROM_PLATFORM_FEXT = 4,
} sprom_platform_t;

typedef enum {
    SPROM_UTIL_TYPE_MIN = 0,
    SPROM_UTIL_TYPE_SUP = 1,
    SPROM_UTIL_TYPE_BP  = 2,
    SPROM_UTIL_TYPE_LC  = 3,
    SPROM_UTIL_TYPE_PS  = 4,
    SPROM_UTIL_TYPE_DIMM= 5,
    SPROM_UTIL_TYPE_FAN = 6,
    SPROM_UTIL_TYPE_PALO = 7,
    SPROM_UTIL_TYPE_MENLO= 8,
    SPROM_UTIL_TYPE_OPLIN= 9,
    SPROM_UTIL_TYPE_IBMC =10,
    SPROM_UTIL_TYPE_MENLO_E    = 11,
    SPROM_UTIL_TYPE_MOD_HOST   = 12,
    SPROM_UTIL_TYPE_MEZZ       = 13,
    SPROM_UTIL_TYPE_NIANTIC    = 14,
    SPROM_UTIL_TYPE_NETEFFECT  = 15,
    SPROM_UTIL_TYPE_SCHULTZ    = 16,
    SPROM_UTIL_TYPE_TIGERSHARK = 17,
    SPROM_UTIL_TYPE_EVEREST    = 18,
    SPROM_UTIL_TYPE_BBU        = 19,
    SPROM_UTIL_TYPE_MONTEREYPARK = 20,
    SPROM_UTIL_TYPE_BROCKWAY_UPLINK_SUP = 21,
    SPROM_UTIL_TYPE_BROCKWAY_UPLINK_BP = 22,
    SPROM_UTIL_TYPE_MEMFRONT = 23,
    SPROM_UTIL_TYPE_MEMBACK = 24,
    SPROM_UTIL_TYPE_DUBLIN = 25,
    SPROM_UTIL_TYPE_FREMONT = 26,
    SPROM_UTIL_TYPE_LIVERMORE = 27,
    SPROM_UTIL_TYPE_ALCATRAZ = 28,
    SPROM_UTIL_TYPE_TURLOCK = 29,
    SPROM_UTIL_TYPE_SDEEPROM = 30,
    SPROM_UTIL_TYPE_MAX,
} sprom_util_type_t;


typedef enum {
    SPROM_CMD_SET,
    SPROM_CMD_SHOW,
    SPROM_CMD_SET_SERIAL,
    SPROM_CMD_SET_PART_NUM,
    SPROM_CMD_SET_PART_REV,
    SPROM_CMD_SET_PART_VER,
    SPROM_CMD_SET_LICENSE,
    SPROM_CMD_SET_MAC,
    SPROM_CMD_DUMP,
    SPROM_CMD_INVALIDATE,
    SPROM_CMD_ZERO,
    SPROM_CMD_SET_FFID,
} sprom_cmd_t;

/*
 * shared back end routines
 */
extern uint16_t
diag_sprom_compute_xsum (void *gen_block, uint32_t len);

extern int sprom_hton ( const diag_sprom_t *, uint8_t * );

extern int sprom_ntoh ( const diag_sprom_t *, uint8_t * );

extern int
sprom_set_common_blk_field ( diag_sprom_t * const sprom, sprom_cmd_t field );

extern int
sprom_set_fru_file_id ( diag_sprom_t * );

extern int
sprom_set_mac ( diag_sprom_t * );

extern int
sprom_set_lic ( diag_sprom_t * const sprom );

extern diag_sprom_t * const sprom_init_pfm ( uint32_t, uint32_t);

extern sprom_platform_t sprom_platform_probe_pfm ( void );

// More prototypes.

extern sprom_platform_t
sprom_get_platform ( void );

extern int
sprom_get_slot_sup ( sprom_platform_t, uint32_t *);

extern int
sprom_get_slot_bp ( sprom_platform_t, uint32_t *);

extern int
sprom_get_slot_chassis ( sprom_platform_t, uint32_t *);

extern diag_sprom_t * const
sprom_init ( sprom_platform_t, uint32_t, uint32_t);

extern int 
sprom_get_size ( const diag_sprom_t * );

extern char * const
sprom_get_name ( const diag_sprom_t * );

extern int 
sprom_verify ( diag_sprom_t *, const uint8_t * );

extern int 
sprom_show ( diag_sprom_t *, uint8_t * const );

extern int 
sprom_set  ( diag_sprom_t * );

extern int 
sprom_read ( diag_sprom_t * const sprom, uint8_t *rbuf );

extern int
sprom_write ( diag_sprom_t * const, uint8_t * );

extern int 
sprom_dump ( diag_sprom_t *const sprom, uint32_t offset, uint32_t len );

extern int
sprom_invalidate( diag_sprom_t *const sprom );

extern int
sprom_zero( diag_sprom_t *const sprom );

extern int sprom_get_hw_revision(diag_sprom_t *sprom, uint16_t *, uint16_t *);
extern int sprom_get_card_index (diag_sprom_t *sprom, uint16_t *);

extern sprom_platform_t ipmi_sprom_platform_probe ( void );
extern sprom_platform_t ipmi_sprom_get_platform ( void );
extern diag_sprom_t * ipmi_sprom_init_pfm ( uint32_t, uint32_t);
extern diag_sprom_t * ipmi_sprom_init ( sprom_platform_t, uint32_t, uint32_t);

extern int ipmi_sprom_get_slot_sup_iom ( uint32_t *);
extern int ipmi_sprom_get_slot_bp_iom ( uint32_t *);
extern int ipmi_sprom_get_slot_chassis_iom ( uint32_t *);

extern int ipmi_sprom_get_slot_sup ( sprom_platform_t, uint32_t *);
extern int ipmi_sprom_get_slot_bp ( sprom_platform_t, uint32_t *);
extern int ipmi_sprom_get_slot_chassis ( sprom_platform_t, uint32_t *);
extern int ipmi_sprom_get_size ( const diag_sprom_t * );
extern char *ipmi_sprom_get_name ( const diag_sprom_t * );
extern int ipmi_sprom_verify ( diag_sprom_t *, const uint8_t * );
extern int ipmi_sprom_show ( diag_sprom_t *, uint8_t * const );
extern int ipmi_sprom_set  ( diag_sprom_t * );
extern int ipmi_sprom_read ( diag_sprom_t * const sprom, uint8_t *rbuf );
extern int ipmi_sprom_write ( diag_sprom_t * const, uint8_t * );
extern int ipmi_sprom_dump ( diag_sprom_t *const sprom, uint32_t offset, uint32_t len );
extern int ipmi_sprom_invalidate( diag_sprom_t *const sprom );
extern int ipmi_sprom_zero( diag_sprom_t *const sprom );
extern int ipmi_sprom_iom_show(diag_sprom_t* const sprom);
extern int ipmi_sprom_iom_bp_show(diag_sprom_t* const sprom);
extern int ipmi_sprom_set_mac ( diag_sprom_t * const sprom );
extern int ipmi_sprom_ui (int argc, char *argv[]);
extern int ipmi_sprom_set_serial ( diag_sprom_t * const sprom );
extern int ipmi_sprom_brd_serial_no_get  (uint8_t type, uint8_t *pserial, uint8_t psize);
extern int ipmi_sprom_set_fru_file_id ( diag_sprom_t * const sprom );
extern int ipmi_sprom_product_name_get (uint8_t type, uint8_t slot, uint8_t *pname, uint8_t psize);
#endif // _DIAG_SPROM_H__

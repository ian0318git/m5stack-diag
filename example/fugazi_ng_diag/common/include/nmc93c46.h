/* $Id: nmc93c46.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/nmc93c46.h,v $
 *------------------------------------------------------------------
 * nmc93c46.h -- NMC93c46 EEPROM Support
 *
 * November 1995, Steve Zhang
 * Ported to DiagMon September 1996, Rob Clevenger
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __NMC93C46_H__
#define __NMC93C46_H__

/*
 * PA management typedef
 */
typedef struct pas_management_t_ {

    uint pa_bay;			/* Port Adaptor bay */

    /*
     * PA ID eeprom clock register
     */
    ulong clk_reg_width;
    ulong clk_mask;
    volatile void *clk_reg;

    /*
     * PA ID eeprom select register
     */
    ulong select_reg_width;
    ulong select_mask;
    ulong enable_select_bits;
    ulong disable_select_bits;
    volatile void *select_reg;
    
    /*
     * PA ID eeprom datain register
     */
    ulong datain_reg_width;
    ulong datain_mask;
    volatile void *datain_reg;

    /*
     * PA ID eeprom dataout register
     */
    ulong dataout_reg_width;
    ulong dataout_mask;
    volatile void *dataout_reg;

} pas_management_t;

typedef int pas_eeprom_type;		/* EEPROM device type */
typedef int pas_eeprom_cmd_t;		/* EEPROM command type */

/*
 * Define the big band signal bits
 */
#define TST_DO_IN       0x80
#define TST_DI_OUT      0x40
#define TST_MS_OUT      0x20
#define TST_MODE1_OUT   0x10
#define TST_MODE0_OUT   0x08
#define TST_CLK_OUT     0x04
#define TST_DIO_DIR     0x02
#ifdef DUPLICATE
#define TST_DIO_BI      0x01
#endif
/*
 * Define the two supported ID eeproms
 */
#define PAS_EEPROM_X2444		1
#define PAS_EEPROM_NMC93C46		2
#define PAS_EEPROM_AT93C66              3

/*
 * Define the command set supported
 */
#define PAS_EEPROM_CMD_READ		1
#define PAS_EEPROM_CMD_WRITE		2
#define PAS_EEPROM_CMD_ERASE		3
#define PAS_EEPROM_CMD_ZERO		4

/*
 * Define timings constants for the EEPROM's
 */
#define PAS_EEPROM_DELAY		10	/* 10 us */
#define PAS_X2444_DELAY			5000	/* 5000 us */
#define PAS_NMC93C46_WRITE_DELAY        10000   /* 10 ms */
/* 
 * Define NMC93C46 command codes      	SB    OP    Addr[5,0]     
 */
#define NMC93C46_CMD_CONTROL   		(0x1 | 0x0)
#define NMC93C46_CMD_WRDS      		(0x1 | 0x0 | 0x00)
#define NMC93C46_CMD_ERASE_ALL 		(0x1 | 0x0 | 0x08)
#define NMC93C46_CMD_WRITE_ALL 		(0x1 | 0x0 | 0x10)
#define NMC93C46_CMD_WREN      		(0x1 | 0x0 | 0x18)
#define NMC93C46_CMD_READ      		(0x1 | 0x2)
#define NMC93C46_CMD_WRITE     		(0x1 | 0x4)
#define NMC93C46_CMD_ERASE     		(0x1 | 0x6)

extern void init_for_eeprom_access(int, pas_management_t *);
extern void show_pa_info(pas_management_t *);
extern int  pas_eeprom_dataout(pas_management_t *);
extern void pas_eeprom_select (pas_management_t *, boolean);
extern void pas_eeprom_clock (pas_management_t *, boolean);
extern void pas_eeprom_datain (pas_management_t *, boolean);

extern pas_management_t *get_pas_cookie_4_ptr(int, int);
extern pas_management_t *pas_select_testport(uint);
extern boolean pas_eeprom_io (pas_management_t *, pas_eeprom_type ,
                              int , int , ushort *);
extern boolean pas_access_sys_eeprom (pas_management_t *,
                               void *, pas_eeprom_type ,
                               pas_eeprom_cmd_t , ushort );


#endif /* __NMC93C46_H__ */

/* end of file */


/******** History ******** 
$Log: nmc93c46.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

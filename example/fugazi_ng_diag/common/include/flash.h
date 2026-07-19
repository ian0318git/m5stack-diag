/* $Id: flash.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/flash.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

/*
** Flash id structure.
** Used in the flash device table and the flash driver structure.
*/

struct flash_id {
    unsigned short dev_code;    /* manufacturer and device code as in manual */
    unsigned short r_dev_code;  /* man and dev code as we see it */
    unsigned int dev_size;      /* the size of a single device */
    unsigned int sector_size;   /* the size of an erase sector */
    char *dev_string;           /* the device name string */
    unsigned int flags;         /* flag field (see below) */
};

/* Flags for above flag field */
#define FI_STATREG 1            /* this flash part has a status register */
#define AMD_STATREG 0x80        /* this flash part is an AMD with a status register */

/*
** Flash driver structure.
** Used by all of the flash driver routines.
*/

struct flash_dvr_info {
    struct flash_id *fiptr;      /* pointer to flash_id structure */
    long flash_base;             /* base address of flash on the platform */
    unsigned int flash_width;    /* width of the flash bus */
    unsigned int flash_size;     /* total flash size in bank */
    unsigned int flash_banks;    /* number of flash banks on the platform */
    unsigned int flash_sector;   /* size of flash sectors in this group */
    unsigned long flash_mask;    /* flash width mask */
    unsigned int flash_flags;    /* flag field */
    struct flash_dvr_info *next; /* for HW designs with noncontiguous banks */
};

#define INTEL_SERIES100   1
#define INTEL_SERIES200   2
#define INTEL_MINIFLASH   INTEL_SERIES100 | INTEL_SERIES200
#define INTEL_STRATA      4

/*
** Flash command codes.
** These commands are bit reversed for cisco 68k products. On R4k based
** products, the flash memory is wired up correctly.
*/
#define R_READ_MEMORY	  0
#define R_READ_ID_CODES	  0x9  /* actually 0x90 */
#define R_READ_STAT_REG	  0xE  /* actually 0x70 */
#define R_CLEAR_STAT_REG  0xA  /* actually 0x50 */
#define R_ERASE		  0x4  /* actually 0x20 */
#define R_ERASE_CONFIRM	  0xB  /* actually 0xd0 */
#define R_ERASE_VERIFY	  0x5  /* actually 0xa0 */
#define R_ERASE_SUSPEND   0xD  /* actually 0xb0 */
#define R_ERASE_RESUME    0xB  /* actually 0xd0 */
#define R_PROGRAM	  0x2  /* actually 0x40 */
#define R_BYTE_WRITE	  0x2  /* actually 0x40 */
#define R_ALT_BYTE_WRITE  0x8  /* actually 0x10 */
#define R_PROGRAM_VERIFY  0x3  /* actually 0xc0 */
#define R_RESET		  0xFF
#define R_QUERY           0x19
/*
** Status bit defines for the new Intel 28F008SA chips
** These are reverse bitswapped.
*/
#define R_WR_STAT_RDY     0x1  /* actually 0x80 */
#define R_ERA_SUSP_STAT   0x2  /* actually 0x40 */
#define R_ERASE_STAT      0x4  /* actually 0x20 */
#define R_BYTE_WR_STAT    0x8  /* actually 0x10 */
#define R_VPP_STAT        0x10 /* actually 0x08 */

/*
** Flash command codes (correct bit ordering for R4k based products
**/
#define F_READ_MEMORY	  0x00
#define F_READ_ID_CODES	  0x90
#define F_READ_STAT_REG	  0x70
#define F_CLEAR_STAT_REG  0x50
#define F_ERASE		  0x20
#define F_ERASE_CONFIRM	  0xD0
#define F_ERASE_VERIFY	  0xA0
#define F_ERASE_SUSPEND   0xB0
#define F_ERASE_RESUME    0xD0
#define F_PROGRAM	  0x40
#define F_BYTE_WRITE	  0x40
#define F_ALT_BYTE_WRITE  0x10
#define F_PROGRAM_VERIFY  0xC0
#define F_RESET		  0xFF
#define F_QUERY           0x98
/*
** Status bit defines for the new Intel 28F008SA chips
** These are ordered correctly for R4k systems
*/
#define F_WR_STAT_RDY     0x80
#define F_ERA_SUSP_STAT   0x40
#define F_ERASE_STAT      0x20
#define F_BYTE_WR_STAT    0x10
#define F_VPP_STAT        0x08

#if defined(__mips) || defined(QUAKE) || defined(FERRARI) || \
    defined(MANTIS) || defined(CUISI)

/*
 * These platforms do not need reversed wiring.
 * The flash is wired up correctly.
 */
#define FORWARD_WIRING
#endif

#ifndef FORWARD_WIRING

/*
 * Ordered backwards
 */
#define C_READ_MEMORY	  R_READ_MEMORY	  
#define C_READ_ID_CODES	  R_READ_ID_CODES	  
#define C_READ_STAT_REG	  R_READ_STAT_REG	  
#define C_CLEAR_STAT_REG  R_CLEAR_STAT_REG  
#define C_ERASE		  R_ERASE		  
#define C_ERASE_CONFIRM	  R_ERASE_CONFIRM	  
#define C_ERASE_VERIFY	  R_ERASE_VERIFY	  
#define C_ERASE_SUSPEND   R_ERASE_SUSPEND   
#define C_ERASE_RESUME    R_ERASE_RESUME    
#define C_PROGRAM	  R_PROGRAM	  
#define C_BYTE_WRITE	  R_BYTE_WRITE	  
#define C_ALT_BYTE_WRITE  R_ALT_BYTE_WRITE  
#define C_PROGRAM_VERIFY  R_PROGRAM_VERIFY  
#define C_RESET		  R_RESET		  
#define C_WR_STAT_RDY     R_WR_STAT_RDY     
#define C_ERA_SUSP_STAT   R_ERA_SUSP_STAT   
#define C_ERASE_STAT      R_ERASE_STAT      
#define C_BYTE_WR_STAT    R_BYTE_WR_STAT    
#define C_VPP_STAT        R_VPP_STAT        
#define C_QUERY           R_QUERY

#else /* FORWARD_WIRING */

/*
 * Ordered the correct way
 */
#define C_READ_MEMORY	  F_READ_MEMORY	  
#define C_READ_ID_CODES	  F_READ_ID_CODES	  
#define C_READ_STAT_REG	  F_READ_STAT_REG	  
#define C_CLEAR_STAT_REG  F_CLEAR_STAT_REG  
#define C_ERASE		  F_ERASE		  
#define C_ERASE_CONFIRM	  F_ERASE_CONFIRM	  
#define C_ERASE_VERIFY	  F_ERASE_VERIFY	  
#define C_ERASE_SUSPEND   F_ERASE_SUSPEND   
#define C_ERASE_RESUME    F_ERASE_RESUME    
#define C_PROGRAM	  F_PROGRAM	  
#define C_BYTE_WRITE	  F_BYTE_WRITE	  
#define C_ALT_BYTE_WRITE  F_ALT_BYTE_WRITE  
#define C_PROGRAM_VERIFY  F_PROGRAM_VERIFY  
#define C_RESET		  F_RESET		  
#define C_WR_STAT_RDY     F_WR_STAT_RDY     
#define C_ERA_SUSP_STAT   F_ERA_SUSP_STAT   
#define C_ERASE_STAT      F_ERASE_STAT      
#define C_BYTE_WR_STAT    F_BYTE_WR_STAT    
#define C_VPP_STAT        F_VPP_STAT        
#define C_QUERY           F_QUERY        

#endif /* FORWARD_WIRING */

/*
** Commands for the new AMD 29F016 flash chips.
** Both reversed and normal commands are represented here for
** platforms with flash chips wired backwards.
** THESE ONLY WORK ON 32 BIT WIDE FLASH
*/
#define AMD_FLASH_CMD_ADDR1  (0x15554 / 4)
#define AMD_FLASH_CMD_ADDR2  (0xAAA8 / 4)
#define AMD_FLASH_CMD_ADDR3  (0x2AA)
#ifdef FORWARD_WIRING
#define AMD_FLASH_CMD_CODE1  0xaaaaaaaa
#define AMD_FLASH_CMD_CODE2  0x55555555
#define AMD_FLASH_RESET      0xf0f0f0f0
#define AMD_FLASH_AUTOSELECT 0x90909090
#define AMD_FLASH_PROGRAM    0xa0a0a0a0
#define AMD_FLASH_ERASE1     0x80808080
#define AMD_FLASH_ERASE_CHIP 0x10101010
#define AMD_FLASH_ERASE_SCTR 0x30303030
#define AMD_DQ7_MASK         0x80808080
#else  /* reverse order */
#define AMD_FLASH_CMD_CODE1  0x55555555
#define AMD_FLASH_CMD_CODE2  0xaaaaaaaa
#define AMD_FLASH_RESET      0x0f0f0f0f
#define AMD_FLASH_AUTOSELECT 0x09090909
#define AMD_FLASH_PROGRAM    0x05050505
#define AMD_FLASH_ERASE1     0x01010101
#define AMD_FLASH_ERASE_CHIP 0x08080808
#define AMD_FLASH_ERASE_SCTR 0x0c0c0c0c
#define AMD_DQ7_MASK         0x01010101
#endif

#define MAXFLASHSETS    8
#define MAXFLASHBANKS   16
#define MAXFLASHWRITE   25
#define MAXFLASHERASE	3000

/******  Add in code support MARS  ********/

/*
 * Base addresses of flash devices
 */

#define FLASH_BASE1          ADRSPC_FLASH1_BASE
#define FLASH_BASE2          ADRSPC_FLASH2_BASE
#define FLASH_BASE_PCCARD1   ADRSPC_PCCARD1_MEM
#define FLASH_BASE_PCCARD2   ADRSPC_PCCARD2_MEM

/*
 * Data cache controls
 */

#define CLEAR_DCACHE      0x0800

#define FLASH_NO_PRINT    0x80    /* menu item to determine whether flahs
				    should dipslay warning and ask for user
				    input to stop or continue */

/*
 * PCMCIA Socket & Window definitions
 */

#define C3600_PCMCIA0_SOCK PCMCIA_SOCKET_0 /* flash card socket 0           */
#define C3600_PCMCIA1_SOCK PCMCIA_SOCKET_1 /* flash card socket 1           */

#define C3600_PCMCIA0_DWIN0 PCMCIA_WINDOW_0 /* 15 mb win for common data    */
#define C3600_PCMCIA0_DWIN1 PCMCIA_WINDOW_1 /* 16 mb win for common data    */
#define C3600_PCMCIA0_AWIN  PCMCIA_WINDOW_2 /* 1  mb win for attribute mem  */
#define C3600_PCMCIA0_IOWIN PCMCIA_WINDOW_4 /* 64 Kbyte window for IO space */

#define C3600_PCMCIA1_DWIN0 PCMCIA_WINDOW_0 /* 15 mb win for common data  */
#define C3600_PCMCIA1_DWIN1 PCMCIA_WINDOW_1 /* 16 mb win for common data  */
#define C3600_PCMCIA1_AWIN  PCMCIA_WINDOW_2 /* 1  mb win for attribute mem */
#define C3600_PCMCIA1_IOWIN PCMCIA_WINDOW_4 /* 64 Kbyte window for IO space */

/*
 * PCMCIA attribute space and window sizes 
 */

#define ADRSPC_PCMCIA_ATTR_SIZ  ONE_MEG
#define ADRSPC_PCMCIA_WIN0_SIZ  SIXTEEN_MEG
#define ADRSPC_PCMCIA_WIN1_SIZ  (SIXTEEN_MEG - ADRSPC_PCMCIA_ATTR_SIZ)

#define PCMCIA_CNTLR "Cirrus 6729"

/* Bring these define in from src/flash.c */
#define ERASECMD \
	(C_ERASE | (C_ERASE << 8) | (C_ERASE << 16) | (C_ERASE << 24))
#define ERASECONFIRM \
	(C_ERASE_CONFIRM | (C_ERASE_CONFIRM << 8) | \
	(C_ERASE_CONFIRM << 16) | (C_ERASE_CONFIRM << 24))
#define ERASESTAT \
	(C_ERASE_STAT | (C_ERASE_STAT << 8) | (C_ERASE_STAT << 16) | \
	(C_ERASE_STAT << 24))
#define ERASEVER \
	(C_ERASE_VERIFY | (C_ERASE_VERIFY << 8) | (C_ERASE_VERIFY << 16) | \
	(C_ERASE_VERIFY << 24))
#define WRSTATRDY \
	(C_WR_STAT_RDY | (C_WR_STAT_RDY << 8) | (C_WR_STAT_RDY << 16) | \
	(C_WR_STAT_RDY << 24))
#define BYTEWRITE \
	(C_BYTE_WRITE | (C_BYTE_WRITE << 8) | (C_BYTE_WRITE << 16) | \
	(C_BYTE_WRITE << 24))
#define BYTEWRSTAT \
	(C_BYTE_WR_STAT | (C_BYTE_WR_STAT << 8) | (C_BYTE_WR_STAT << 16) | \
	(C_BYTE_WR_STAT << 24))
#define CLRSTATREG \
	(C_CLEAR_STAT_REG | (C_CLEAR_STAT_REG << 8) | \
	(C_CLEAR_STAT_REG << 16) | (C_CLEAR_STAT_REG << 24))
#define READMEM \
	(C_READ_MEMORY | (C_READ_MEMORY << 8) | (C_READ_MEMORY << 16) | \
	(C_READ_MEMORY << 24))
#define RDSTATCMD \
	(C_READ_STAT_REG | (C_READ_STAT_REG << 8) | (C_READ_STAT_REG << 16) | \
	(C_READ_STAT_REG << 24))
#define PROGCMD \
	(C_PROGRAM | (C_PROGRAM << 8) | (C_PROGRAM << 16) | (C_PROGRAM << 24))
#define PROGVER \
	(C_PROGRAM_VERIFY | (C_PROGRAM_VERIFY << 8) | \
	(C_PROGRAM_VERIFY << 16) | (C_PROGRAM_VERIFY << 24))
#define RESETCMD \
	(C_RESET | (C_RESET << 8) | (C_RESET << 16) | (C_RESET << 24))
#define VPPSTAT \
	(C_VPP_STAT | (C_VPP_STAT << 8) | (C_VPP_STAT << 16) | \
	(C_VPP_STAT << 24))
#define READID \
        (C_READ_ID_CODES | (C_READ_ID_CODES << 8) | \
	(C_READ_ID_CODES << 16) | (C_READ_ID_CODES << 24))
#define QUERY \
	(C_QUERY | (C_QUERY << 8) | (C_QUERY << 16) | (C_QUERY << 24))

/*
 * Global function prototypes
 */
extern void c3600_flash_parser_init(void);
extern void c3600_parser_init(void);

extern void print_flash_struct (struct flash_dvr_info *fdiptr);

/*
 * function prototypes from file src/am29dl_flash.c
 */
extern int am29dl_is_start_of_a_sector(ulong addr);
extern int am29dl_nvflash_erase(ulong flash_addr, ulong mem_addr, uchar type,
                                        uchar chip_erase, uchar verify);
extern int am29dl_nvflash_rd(ulong mem_addr, ulong size, ulong freq,
                                                      ulong ret_addr);
extern int am29dl_nvflash_wr(ulong flash_addr, ulong mem_addr, ushort data,
                                                   uchar type, uchar verify);
extern int am29dl_nvflash_verify(ulong mem_addr, ulong size,
                                 ulong freq, ulong data_addr);
extern void am29dl_erase_cmd(ulong flash_addr, uchar type);
extern void am29dl_program_cmd(ulong flash_addr, uchar type);

#ifndef UNIX
/* flash.c */
extern int get_flash_info(struct flash_dvr_info *);
extern ulong poll_flash_stat(volatile uchar *, ulong, int, ulong);
extern int program_flash(volatile uchar *, long, struct flash_dvr_info *);
extern struct flash_dvr_info *get_fdi_struct(struct flash_dvr_info *, unsigned);
extern int flash_cleanup(void);
extern int erase_flash(struct flash_dvr_info *, unsigned, int, int);
extern int flash_test_perform(struct flash_dvr_info *fdi, int testlength);
extern int flash_test(int);
extern int ftest(struct flash_dvr_info *, ulong *, int, char *);
extern uint get_flash_width(volatile ulong *);
extern int flash(int argc, char *argv[]);
extern int fabbr_test(struct flash_dvr_info *, ulong *, int numvals, char *);
extern int bootflash_addr(ulong *, int *);
extern unsigned get_flash_base(void);
extern unsigned get_flash_end(void);
extern uint get_flash_card_base(void);
extern void flash_prog_en(void);
extern void flash_prog_dis(void);

#endif

/******** History ******** 
$Log: flash.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: common_utils.h,v 1.1 2014/03/25 02:12:32 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 * Copyright (c) 2008-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

/* Print error message based on level */
#define LVL_0           0x00
#define LVL_1           0x01
#define LVL_2           0x02
#define LVL_3           0x03
#define LVL_X           0x10

#define PATTERN   0x5ADBA56C

#define I2C_BUS                 0x01
#define SPI_BUS                 0x02

#define MASK_128B        0x0000007f
#define ALLIGN_128B(x)  ((void *)((uchar *)((ulong)(x + MASK_128B) & ~MASK_128B)))
#define MEM_MGR_MAX_KMALLOC_SIZE  0x20000
#define MAX_LIST_SIZE 512

#define C2600_OSC_40MHZ         38976000
#define C2600_OSC_400MHZ         38976000 *10

typedef long long off64_t;

typedef struct mem_info_t_
{
    uint32_t size;   /* size of of memory that user */
    unsigned long  phy_addr; /* phys addr of location requested by user. used by i/o */
    unsigned long virt_addr; /* virt addr of location requested by user. used by i/o */
    unsigned long kernel_virt_addr; /* kernel virt addr of location requested by user.
                                       klm uses this */
    unsigned long start_phy_addr; /* phy start address of mapped region. */
    unsigned long start_virt_addr; /* virt start address of mapped region. */
    unsigned long mmaped_size;     /* actual mmaped size. minimum is a page size (4k)*/
    unsigned int in_use;
} mem_info_t;

typedef struct reg_info_t_ext_ {
    uint   size;
    int	  (*rd_ptr) (ulong addr, int size, ulong *buf, void *param);
    int	  (*wr_ptr) (ulong addr, int size, ulong data, void *param);
    void   *param;		/* Parameter passed to rd_ptr and wr_ptr */
} reg_info_t_ext;

typedef struct reg_info {
    char           *name;
    uint           offset;
    unsigned char          type;
    union {
	ulong	   size;	/* if type is not REG_ACCESS */
	reg_info_t_ext *ext;	/* if type is REG_ACCESS */
    } size;
    uint          mask;
    uint          reset_val;
} reg_info_t;

/* display registers for multiple channels */
typedef struct reg_ch_info {
    reg_info_t	register_info;
    uint	channel_info;
} reg_ch_info_t;


extern int  register_tests(reg_info_t *, int);
extern int  register_display(reg_info_t *, int);
extern int  register_alter(reg_info_t *, int);
extern uint32_t  new_register_read(reg_info_t *, ulong,
                                    volatile unsigned char *, int);
extern int getc_answer(char * msg, char *cmpstr, char curval );
extern char *take_0x_addr (char *addr_p);
extern unsigned long gethex_answer(char *msgstr, unsigned long currentval,
				   unsigned long min, unsigned long max);
extern unsigned int getdec_answer(char *msgstr, unsigned int currentval,
				  unsigned int min, unsigned int max);
extern void *malloc_nm (unsigned long size);
extern void free_nm (void *s);
extern int display_etsec_regs (void);
extern void msleep (int n);
extern void wastetime (long usec);
extern void print_err (boolean, char *, char);

#endif /* __COMMON_UTILS_H__ */

/* End of File */
/*------------------------------------------------------------------------------
 * $Log: common_utils.h,v $
 * Revision 1.1  2014/03/25 02:12:32  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.4  2012/01/09 23:06:17  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.3  2011/10/07 01:11:44  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:22  huanngo
 * Update code to patriot2-branch
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

/* $Id: common_utils.h,v 1.3 2018/08/31 03:59:29 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/common_utils.h,v $
 *------------------------------------------------------------------
 * Copyright (c) 2011 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

#define READ_WRITE 0
#define READ_ONLY  1
#define WRITE_ONLY 2

typedef enum {
    WALKING_0 = 0,
    WALKING_1,
    PSEUDO_RANDOM,
    ADDR_EQ_DATA,
    CHECKER_BOARD,
    INVERSE_CHECKER,  /* 0x5 */
    USER_PATTERN,
    MEMORY_WRITE,
    MEMORY_READ,
    MEM_ACCESS_TEST,
    LG_CACHE_MEM,     /* 0xa */
    LG_UNCACHE_MEM,
    SH_CACHE_MEM,
    SH_UNCACHE_MEM,
    PLAT_SPEC_MEM,
    GRP_TESTS,        /* 0xf */
    INCREMENT_PAT,
    ALL_0_THEN_1,
    MARCH_TEST,
} MEM_TEST_TYPE;

typedef enum {
    SHORT_UNCACHE = 0,
    SHORT_CACHE,
    LONG_UNCACHE,
    LONG_CACHE,
} MEM_CACH_TYPE;

#define DATA_CACHE        1
#define L2_CACHE          2

typedef enum {
    ONE_CHANNEL,	
    TWO_CHANNELS,
} CHANNEL_TYPE;

#define CHANNEL_0  	0x0	
#define CHANNEL_1  	0x1
#define NON_CHANNEL	0xf

#define ROTATE_LEFT(x)    (((x) << 1) | (((x) & 0x80000000) >> 31))

typedef struct reg_info_t_ext_ {
    uint   size;
    int	  (*rd_ptr) (unsigned long addr, int size, unsigned long *buf, void *param);
    int	  (*wr_ptr) (unsigned long addr, int size, unsigned long data, void *param);
    void   *param;		/* Parameter passed to rd_ptr and wr_ptr */
} reg_info_t_ext;

typedef struct reg_info {
    char           *name;
    unsigned int           offset;
    unsigned char          type;
    union {
	unsigned long	   size;	/* if type is not REG_ACCESS */
	reg_info_t_ext *ext;	/* if type is REG_ACCESS */
    } size;
    unsigned int          mask;
    unsigned int          reset_val;
} reg_info_t;

/* display registers for multiple channels */
typedef struct reg_ch_info {
    reg_info_t	register_info;
    unsigned int	channel_info;
} reg_ch_info_t;

typedef struct mem_test_info {
    unsigned long          start_addr;
    unsigned long          end_addr;
    MEM_TEST_TYPE  test_type;
    MEM_CACH_TYPE  cache_type;
    unsigned long          test_pattern;
    unsigned long          test_freq;
    unsigned long          real_start_addr;   /* use for  addr = data test */
    unsigned long	   retention_time;	/* data retention time in milliseconds*/
} mem_test_info_t;

struct testdat {
    long rd_pat;            /* read verify pattern */
    long wr_pat;            /* write pattern */
    long flag;              /* 1 = increment, 0 = decrement */
};


extern struct testdat march_patterns[];
extern int  register_tests(unsigned long, reg_info_t *);
extern int  register_def_tests(unsigned long, reg_info_t *);
extern unsigned long  register_read(unsigned long, int);
extern void  register_write(unsigned long, unsigned long, int);
extern int  register_display(unsigned long, reg_info_t *);
extern int  register_alter(unsigned long, reg_info_t *);
extern int  test_mainmem(int);
extern int  mem_util(void);
extern int  size_mem(void);
extern long sizemainmem(void);
extern int  platform_memory_test(unsigned long, unsigned long);
extern int  platform_memory_64_test(unsigned long, unsigned long);
extern int  enable_mem_cache(unsigned long *);
extern void restore_mem_cache(unsigned long);
extern int  mem_addr_eq_data_test(unsigned long, unsigned long, unsigned long);
extern int  mem_checkerboard_test(unsigned long, unsigned long, unsigned long);
extern int  mem_walking_test(unsigned long, unsigned long, unsigned long); 
extern int  verify_walking_data(unsigned long, unsigned long, unsigned long);
extern int  mem_random_num_test(unsigned long, unsigned long, unsigned long);
extern void memory_write(unsigned long, unsigned long, unsigned long);
extern void memory_read(unsigned long, unsigned long, unsigned long);
extern int  mem_access_test(unsigned long, unsigned long, unsigned long);
extern int  mem_grp_test(unsigned long, unsigned long, unsigned long);
extern int  check_pci_dev_id_rev(int, int, unsigned long, unsigned long);
extern int  mem_march_test(MEM_CACH_TYPE, unsigned long, unsigned long);
extern int  mem_specific_test(mem_test_info_t *);
extern int  mem_all_zeros_ones_test(unsigned long, unsigned long, unsigned long);
extern void init_mem_test_info_ds(mem_test_info_t *);
extern void  dis_mem_sub(void *);
extern void  fil_mem_sub(void *);
//extern u64  read64(u64 *);
//extern void write64(u64 *, u64);
//extern void pci_conf_wr_util (unsigned long bus_num, unsigned long pci_dev_num);
//extern void pci_conf_rd_util (unsigned long bus_num, unsigned long pci_dev_num);
//extern void pcie_conf_wr_util (uint32_t, uint32_t bus_num, uint32_t);
//extern void pcie_conf_rd_util (uint32_t, uint32_t, uint32_t);
extern void mdelay(unsigned int msecs);
extern void dismem(unsigned char *addr, int length, unsigned long disaddr, 
		   int fldsize);
extern int chkberr(volatile unsigned int *address, int size, unsigned readonly);
extern void get_memory_location(unsigned long *val1, unsigned long *val2);
extern long getfreememstart(void );
extern unsigned long gethex_answer(char *msgstr, unsigned long currentval, 
				   unsigned long min, unsigned long max);
extern int getc_answer(char *msg, char *cmpstr, char curval);
//extern uchar get_pci_bus_no(uint);
//extern int get_pci_dev_num(int , int );

#endif /* __COMMON_UTILS_H__ */

/******** History ******** 
$Log: common_utils.h,v $
Revision 1.3  2018/08/31 03:59:29  chieyang
Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2

Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/

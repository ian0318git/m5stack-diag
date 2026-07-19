/* $Id: free.h,v 1.3 2016/04/20 07:03:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/free.h,v $
 *------------------------------------------------------------------
 * Definitions & prototypes for memory pool functions (malloc, etc.)
 *
 * September 1998, David Turner
 *
 * Copyright (c) 2007-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#define MALLOC_SHOW_BUF_BYTES  32   /* # bytes to display from malloc buf */

#define MALLOC_BYTE_ALIGNMENT  64   /* Power of two only */

typedef char Align[MALLOC_BYTE_ALIGNMENT];

typedef union free_header_t_ {
    struct {
	union free_header_t_  *freeptr; /* next free block */
	unsigned long         size;     /* # header-sized units of this block */
	unsigned short        padding;  /* fill struct to long word */
	unsigned short        pool_id;
    } s;
    Align a;
} free_header_t;

typedef enum mempool_class_ {
    MEMPOOL_CLASS_UNKNOWN  = 0,
    MEMPOOL_CLASS_DRAM,
    MEMPOOL_CLASS_DRAM_DEV,
    MEMPOOL_CLASS_PCIMEM,
    MEMPOOL_CLASS_IOMEM,
    MEMPOOL_CLASS_SHMEM,
    MEMPOOL_CLASS_SPACE_1,
    MEMPOOL_CLASS_SPACE_2,
    MEMPOOL_CLASS_SPACE_3,
    MEMPOOL_CLASS_SPACE_4,
    MEMPOOL_CLASS_SPACE_5,
    MEMPOOL_CLASS_SPACE_6,
    MEMPOOL_CLASS_SPACE_7,
    MEMPOOL_CLASS_SPACE_8,
    MEMPOOL_CLASS_SPACE_9,
    MEMPOOL_CLASS_SPACE_10,
    MEMPOOL_CLASS_MAPPED_1,
    MEMPOOL_CLASS_MAPPED_2,
    MEMPOOL_CLASS_MAPPED_3,
    MEMPOOL_CLASS_MAPPED_4,
    MEMPOOL_CLASS_MAPPED_5,
    MEMPOOL_CLASS_MAPPED_6,
    MEMPOOL_CLASS_MAPPED_7,
    MEMPOOL_CLASS_MAPPED_8,
    MEMPOOL_CLASS_KERNEL,   /* Used exclusively by the kernel */
    MEMPOOL_CLASS_MAX
} mempool_class;

typedef struct mempool_t_ {
    char              *name;              /* name of memory pool */
    mempool_class     class;              /* memory category */
    void              *membase;           /* first location of this memory */ 
    unsigned long     memlen;             /* length of this pool in bytes */
    free_header_t     *freep;             /* next available block */
    void              (*moremem)(void *); /* if !NULL, for more if needed */
    void              *moremem_param;     /* param for moremem call */
} mempool_t;

/*
 * Entry for table that specifies mempools to be initialized on
 * the platform.
 */
typedef struct mempools_table_t_ {
    char              *name;              /* name of memory pool */
    mempool_class     class;              /* pool class */
    unsigned long     phy_base;           /* physical address of pool */
    unsigned long     vir_base;           /* virtual address of pool */
    unsigned long     max_size;           /* max length in bytes */
    unsigned long     act_size;           /* actual length (<= max_size) */
} mempools_table_t;

/*
 * Descriptor that specifies the mempools table initialized on the
 * platform.
 */
typedef struct mempools_tbl_desc_t_ {
    mempools_table_t   *table;             /* initialized table */
    unsigned long      num_pools;          /* # that span actual memory on mb */
    unsigned long      rnd_rbn;            /* next index of pool for access */
} mempools_tbl_desc_t;

/*
 * Globals
 */
extern mempool_t mempool[];
extern mempools_tbl_desc_t *mempools_tbl_desc_p;

/*
 * Prototypes
 */
#if defined(OVERLORD) || defined(TACHI)
#else
extern void *malloc(unsigned long nbytes);
extern void free(void *buf);
extern void *realloc (void *ptr, unsigned long size);

#endif

extern void *malloc_pool(mempool_class class, unsigned long nbytes);
extern void free_nm(void *buf);
extern void reset_mempools(void);
extern void init_mempool(char *memname,
			 mempool_class pool,
			 void *membase,
			 unsigned long nbytes);
extern void add_region_to_mempool(mempool_class pool,
                                  void *membase,
                                  unsigned long nbytes);
extern void restore_mempools(void);
extern void display_mempool(mempool_class pool, char *header);
extern void init_mempools_from_table (mempools_table_t *, unsigned long
                                      tbl_size, unsigned long unavail_bytes);
extern void show_mempools_table(void);
extern void *malloc_all(unsigned long nbytes);
extern unsigned long map_pool_to_phy(unsigned long vaddr);
extern unsigned long map_phy_to_pool(unsigned long paddr);
extern void *malloc_nm (unsigned long nbytes);
extern void free_nm (void *);

/*
 * Macros to map between the virtual and physical addresses 
 * corresponding to one of entries in the platform's mempool table.
 * These macros result in calls to either map_pool_to_phy or
 * map_phy_to_pool.  If the platform has no mempool table, the
 * functions default to executions of PHY_ADDR(vaddr) and
 * PHY_TO_KSEG1(paddr).
 */
#define  POOL_TO_PHY(vaddr)  (map_pool_to_phy(vaddr))
#define  PHY_TO_POOL(paddr)  (map_phy_to_pool(paddr))

/******** History ******** 
$Log: free.h,v $
Revision 1.3  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.2.60.2  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

Revision 1.2.60.1  2015/08/04 04:21:13  meho
Added flag for TACHI.

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: htdp.h,v 1.1 2013/07/22 19:50:39 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/htdp.h,v $
 *------------------------------------------------------------------
 *
 * htdp.h --header for espresso xaui
 *
 * 2013
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _HTDP_H_
#define _HTDP_H_

#define NNOTRINGS 2
#define NCPUS_NS  2

#include "scby_htdp.h"

#ifndef CIRC_CNT
/* how many left to be processed..space between tail to head */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))
#endif

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#ifndef CIRC_SPACE
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))
#endif

#define DMA_OFFSET(b, a)  ((unsigned long)b - (unsigned long)a)
// max fifo is 32, we set that in all cases

/* Transform (cpu, slot, priority) into a read queue index */
//#define HTS_FCPU_OFFSET(dest, prio)    ((dest*64) + (prio*4))
#define HTS_FCPU_INDEX(dest, prio)     ((dest*4) + prio)
#define HTS_TCPU_PTR_OFFSET            (FCPU_NDEST*4)
#define HTS_FB_FILL_OFFSET             (HTS_TCPU_PTR_OFFSET + 4)

#define HTS_

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

// For rx, there are 2 pri (hi/low) * 2 "cpu's" =  4.
// For tx, there are 4 queue * 2 pri * 2 cpus = 16

#define NHTDP_PAGES     8192   // be a power of two..easier on masking (was 128)
#define NHTDP_PAGES_MSK (NHTDP_PAGES - 1)
//#define NHTDP_PG        (NHTDP_PAGES/(PAGE_SIZE/ sizeof (struct htdp_pg)))  /* 8K (4K/64) = 128 ?? */
#define NHTDP_PG        128

/*  ver 14 added DUAL_FP drain */

#define PACKED __attribute__((packed))


/*
 *   Ethernet packet buffers.
 */

#define TRUE  1
#define FALSE 0


#define HTSDBG_ALLOC	0x00000001	// hts supplies the memory 
#define HTSDBG_ALLOC1	0x00000002	// see print of kmalloc() rollup
#define HTSDBG_IOCTL1	0x00000004	// see print of kmalloc() rollup
#define HTSDBG_INIT	0x00000008	// see print of init issues     
#define HTSDBG_XON	0x00000010	// see print of xon  issues     
#define HTSDBG_IGNLINK	0x00000020	// ignore netlink
#define HTSDBG_PGBUF	0x00000040	// see print of alloc pg (particle) buf
#define HTSDBG_INTR	0x00000080	// see print of interrupt related 
#define HTSDBG_INTR_MERR	0x00000100	// get misc_error printed
#define HTSDBG_XMIT	0x00000200	// see xmit
#define HTSDBG_USER	0x00000400	// see user related calls
#define HTSDBG_READ	0x00000800	// for hts_read() prints  0x00100000
#define HTSDBG_WRITE	0x00001000	// for hts_write() prints 0x00200000
//#define HTSDBG_CFG	0x00001000	// see cfg (dbg)  
#define HTSDBG_ERP	0x00002000	// see erp (dbg)  
#define HTSDBG_HTDP0	0x00004000	// see htdp level 0 (dbg)
#define HTSDBG_HTDP1	0x00008000	// see htdp level 0 (dbg)
#define HTSDBG_CILINK	0x00010000	// see cilink       (dbg)
#define HTSDBG_NOXFER	0x00020000	// setup but no xfer(dbg)
#define HTSDBG_ZEROMEM	0x00040000	// zero newly kmalloc mem
#define HTSDBG_FREE	0x00080000	// free memory
#define HTSDBG_CFG	0x00100000	// see cfg (dbg)  formally 0x1000
//#define HTSDBG_READ	0x00100000	// for hts_read() prints  0x0800
//#define HTSDBG_WRITE	0x00200000	// for hts_write() prints 0x1000
#define HTSDBG_PACCESS	0x00400000	// for physical asic prints
#define HTSDBG_PBCHECK	0x00800000	// pariticle buffer checking

#define HTSDBG_SMDIS	0x04000000	// TEST - do not disable sm ints txfifo)
#define HTSDBG_NSMDIS2	0x08000000	// TEST - disable sm ints txfifo)
#define HTSDBG_NIDCACHE	0x10000000	// TEST - do not invalidate dcache (htc)
#define HTSDBG_TXRXTST	0x20000000	// tx-rx loopback test, encode tx bufadr
#define HTSDBG_ITRACE	0x40000000	// ihts_hts() tracing on
#define HTSDBG_WRERR	0x80000000	// create write error (no SOP)

// this matches HTD_TCPUCFG_HDREXP_X
#define HTSDBG_EXPHDR_M 0x03000000	// hdr exp mask  HTD_TCPUCFG_HDREXP

#define HTSDBG_EXPHDR_0		HTD_TCPUCFG_HDREXP_0 
#define HTSDBG_EXPHDR_64  	HTD_TCPUCFG_HDREXP_64
#define HTSDBG_EXPHDR_128	HTD_TCPUCFG_HDREXP_128
#define HTSDBG_EXPHDR_256	HTD_TCPUCFG_HDREXP_256



/*
 * this struct is control information of each 4K page we kmalloc'ed.
 * it sits in an array within driver
 * within each page, sits the virtaddr back to the struct
 *  might be interested int the _va of the page_struct
 */
struct htmq {
	unsigned char  htdpg_q;		/* HTMEM_Q_x */
	unsigned char  htdpg_resv;	/* unused */
	unsigned short htdpg_flag2;	/* used only for hdrs */
};

struct htdp_pg {
    unsigned char *htdpg_kva;	// kernel virtual address from kmalloc
    unsigned char *htdpg_paddr;	// physical addr ?
    unsigned int htdpg_dma;	// dma addr ?
    //struct page   *htdpg_page;	// page struct pointer for remap needed ?? ZZZ
    unsigned short	htdpg_pgno;	// which page number (4k only)
    unsigned short	htdpg_flags;
    unsigned int    htmq_no;
    unsigned int	htdpg_read;	// which page number (4k only)
    struct htdp_pg	*htdpg_next;	// only used when not allocated.
    unsigned int	htdpg_len;	// sentinel or lenflags returned

};

#if defined(CONFIG_X86_64)
#define SWDATA_TO_HTDP_PG(_swdata)	\
    ((struct htdp_pg *)__va((unsigned long)_swdata << 3))
#define HTDP_PG_TO_SWDATA(_htpg_p)	({				\
    __typeof__(_htpg_p) ptr = (_htpg_p);				\
    unsigned int swdata = (__pa(ptr) >> 3) & 0xffffffff;		\
    __typeof__(_htpg_p) ptr2 =						\
	(__typeof__(_htpg_p))SWDATA_TO_HTDP_PG(swdata);			\
    if (ptr != ptr2) {							\
	panic("HTDP_PG_TO_SWDATA(): %lx != %lx\n",			\
	       (unsigned long)ptr, (unsigned long)ptr2);		\
	swdata = 0;							\
    }									\
    swdata;								\
})
#else
#define HTDP_PG_TO_SWDATA(_htpg_p)	((unsigned int)(_htpg_p))
#define SWDATA_TO_HTDP_PG(_swdata)	((struct htdp_pg *)(_swdata))
#endif

// htdpg_flags
// updated from htsa/htdp.h 5/19/06
#define HTDPG_HT	0x00000001	// with HT
#define HTDPG_PROC	0x00000002	// with a proc
#define HTDPG_TXFIFO	0x00000004	// allocated to a tx
#define HTDPG_FBUF	0x00000008	// allocated as a freebug
#define HTDPG_ALLOC	0x00000010	// alloc = 1
#define HTDPG_IRXPKT	0x00000020	// was in irxpkt
#define HTDPG_READ	0x00000040	// was in read
#define HTDPG_NOMEM	0x00000080	/* has no pages allocated, hdr only */
#define HTDPG_RXFPM	0x00000100	/* came from a tcpu0/1 int mask */
#define HTDPG_RXFPM_S	8		/* shift to normalize */
#define HTDPG_RXFP0	0x00000000	/* came from a tcpu0 int */
#define HTDPG_RXFP1	0x00000100	/* came from a tcpu1 int */

/* struct htmem_q contains head/tail info for a number of hdr queues 
 * introduced for 0copy lsmpi
 */
struct htmem_q {
    unsigned int	htm_flags;
    unsigned short	htm_osize;	/* original size */
    unsigned short	htm_size;	/* incr hts_put(), decr hts_get() */
    unsigned short	htm_hiw;	/* high water mark */
    unsigned short	htm_low;	/* low water mark */
    struct htdp_pg	*htm_head;	/* head ptr for hts_put() hts_get() */
    struct htdp_pg	*htm_tail;	/* tail ptr for hts_put() hts_get() */
    void   	        *htm_htdp;	/* static array of kmalloc() htdp_pg */
    //	spinlock_t	htm_headlock;	/* protects concurrent mods */
};

#define HTMEM_Q_DEF	0		/* 0 is our normal queue -- non lsmpi*/
#define HTMEM_Q_RXHTS	1		/* 1 for rx, hts owns -- rx in prog*/
#define HTMEM_Q_RXRET	2	/* reserved  2 for rx, hts returned to lsmpi*/
#define HTMEM_Q_TX	3		/* 3 for tx, hts holding q for tx*/
#define HTMEM_Q_MAX	1		/* must be +1 after last */

#define HTMF_ALLOC	0x00000001	/* kmalloc real memory for this */

// hts_ldev = HT Scooby Logical DEVice
// one per open of the device 	
// s.b htldev (ht logical device)  (minor device)
struct ht_ldev {
	unsigned short		htl_init;	/* 0=not 1=yes */
	unsigned short		htl_flags;
	struct ht_pdev		*htl_ht_pdev;	/* ptr to hts_pdev  (user)  */
	struct vm_area_struct	*htl_vma;

	struct task_struct	*htl_task;
	struct file		*htl_fp;	/* user file struct */
	struct scby_htdp	*htl_htvp;	/* (dup) vaddr of scooby ht addr */
	unsigned short		htl_cpu;	/* 0 or 1 cpu f0/f1 */
	unsigned short		htl_tcpu_indx;	/* HTS_TCPU_xxx for indx to wp/rp*/
	unsigned int		htl_tcpu_pri;	//0 or 1 which priority	(open)

						// is a wierd byte swap index
						// to match fpcu_rp
	unsigned int		htl_fcpu_indx;	//index calced by Cpu/Queue/pri	
	struct _fcpu_dfifo	*htl_fcpu_dp;	// tx(fcpu) descriptor fifo
						// fcpu_desc (ht_scby_type.h)

	unsigned int		htl_pxflen;	// org pxflen on the rx
	unsigned int		htl_pxf_rd;	//consumed so far offst into pkt

	unsigned int		htl_pxf_slot;
	unsigned int		htl_flags2;
// pxf hdr tx/rx
	unsigned int		htl_txhdrx;	// tx header indx
	unsigned int		htl_pxf_dstflg;	// default, ioctl, nltx change
	unsigned int		htl_txhdrlen;	// (done at init), 
	void (*htl_txpxf)(void *pxftp, struct ht_ldev *ht_ldevp, 
			int olen, int chnl);	
	void (*htl_txpxfdst)(struct ht_ldev *ht_ldevp, int slot, 
			int port, int type);

	unsigned int		htl_rxhdrx;	// rx header indx
	unsigned int		htl_rxhdrlen;	// (done at init), 
	void (*htl_rxpxf)(void *pxfrp, unsigned int *pxflen,
		 unsigned int *pxfflen, unsigned int *pxfslot,
		 unsigned int *pxfdst);

// ToCpu == rx
// last read pointer-> HHHHLLLL index. which last values are
// vaddr is htl_vrdto == HHHHLLLL index
// rdnx_hp(lp) == where sw read to last and wrote this HHHHLLLL indexes to 
//  0x20 or 0x24
// 
//	unsigned int	*htl_tcpuwp;	/* where to read up to pointer 0x40 */

//	unsigned int	*htl_vrdnx_st;	/*based htl_tcpuwp, v(read) cpustart */
//	unsigned int	*htl_vrdnx_en;	/* 0x20/24 v(write)tell hw what cpu rd*/

	struct tcpu_desc *htl_vtcpu_dbase; /* v(tocpu first desc) 0x54/6c*/
//5/23	unsigned short	htl_rdnx_hp;	/* sw rx (to cpu) last read index HP */
//5/23	unsigned short	htl_rdnx_lp;	/* sw rx (to cpu) last read index LP */
// s.b. only this stream's specific rx count, as we should OR real value

//	unsigned int	wake_code;	/* from int/timer to _read()'s sleep*/
//	volatile unsigned short	tcc;    /* first rcv msg belonging to CC */
//	volatile unsigned short	tnc;    /* first rcv msg belonging to NC */
	unsigned int	tx_packets;
	unsigned int	tx_bytes;
	unsigned int	tx_errors;
	unsigned int	tx_dropped;	/* size/error  or not ready  */
	unsigned int	tx_pdropped;	/* cpu dropped no buf rdy*/
	unsigned int	tx_srcaddr;	/* bad source packet addr   */

	volatile unsigned short	rcc;    /* first rcv msg belonging to CC */
	volatile unsigned short	rnc;    /* first rcv msg belonging to NC */
	unsigned int	rx_packets;
	unsigned int	rx_errors;
	unsigned int	rx_read;	 /* called to do read */

    //	struct	 timer_list htl_timer;
    //wait_queue_head_t   htl_waitq;	
};

struct ht_pdev {

    struct fpga_ht   *htp_htvp;	// vaddr HT offset in Scooby  6_0000

    volatile unsigned char	*htp_fcpu_rp;	// FCPU read pointers (HW)

    // sw maintained fifo ptr indx

    unsigned int	htp_hdrsize;	//used with HDR_EXP
    //5/30	struct ht_tcpu	htp_tcpu[NCPUS]; // using 2 (internal save of last indx)
    // next 2 are for 1 cpu only.... FIXME

    unsigned char  cpu_wp[FCPU_DESC_FIFO];	// pointer maintained by software
    unsigned char  cpu_rp[TCPU_NPRIORITY];	// poitner maintained by software
};


/***************************************************
 *  (ToCPU) Tcpu (FromCPU) Fcpu (Free Buffer Pool) fb Descriptors
 ***************************************************
 */

#define  TCPU_FLAG_ERR		0x00000001	/* err */
#define  TCPU_FLAG_EOP		0x00000002	/* End of Packet */
#define  TCPU_FLAG_SOP		0x00000004	/* Start of Packet */
#define  TCPU_FLAG_M		0x00000007	/* mask for ERR/EOP/SOP */
#define  TCPU_BUFLEN_M		0xfff00000
#define  TCPU_BUFLEN_M_NORM	0x00000fff	/* mask after normalized */
#define  TCPU_BUFLEN_S		20

// located at 0xb00 (Fcpu_fb_dfifo_reg0)

// ToCpu HL descr first address
#define TCPU_1ST_DESC_SZ	0x4000	/* 16k; 1K entry. each entry has 16 bytes */
#define TCPU_FB_1ST_DESC_SZ	0x4000	/* 16K;  1K entry. each entry has 16 bytes */
#define TCPU_FB_RING_SZ		0x400	/* 1024 * 16 byte desc == 0x4000 */
#define TCPU_FB_RING_M		(TCPU_FB_RING_SZ - 1)
#define TCPU_WPADDR_SZ		0x100	/* was 0x2000 sb 16 -- 8k --? for 040 */

//#define TCPU_DRING_SZ		0x0010	/*  0x80 (11/22) = 128 */
//#define TCPU_DRING_SZ		1024	/*  0x80 (11/22) = 128 */
//#define TCPU_DRING_SZ_M		(TCPU_DRING_SZ -1)	/*  to circ buf this */

//#define FCPU_DRING_SZ		NTRB	/* 32? wow -- really small */

/* min time between Tcpu ints to assist in not being inundated */
/* Mike to define timer metric.   */
/* FYI: diag has 0x2 */
#define BOUNDARY_1M                 0x100000
#define MASK_1M                     (BOUNDARY_1M-1)

#define HTS_PARTICLE_SIZE       1024     /* AKL-----    remember 4K */
#define HTS_PARTICLE_SIZE_S     9       /* particle size is 2**9  */
#define HTS_PARTICLE_SHIFT      6
#define HTS_PARTICLE_ALIGN      1 << HTS_PARTICLE_SHIFT
#define HTS_PARTICLE_MASK       HTS_PARTICLE_ALIGN-1
#define HTDP_BUFFER_ALIGN       HTS_PARTICLE_ALIGN
/* fix me should be 1024 for from cpu */
#define HTS_NPARTICLES          0x2000    /* the first uio
                                           mapping is for descriptors and read write poitners.
                                           the other 4 uio mappings are for free buffers. */

#define HTS_RSHIFT(a) ((((unsigned long)(a))>>HTS_PARTICLE_SHIFT) & 0xFFFFFFFF)
#define HTS_LSHIFT(a) (((unsigned long)(a))<<HTS_PARTICLE_SHIFT)

/* convert virt to phy and shift used by cpu */
#define HTS_VIRT_ADDR(a) (__va((void *)HTS_LSHIFT(a)))

/* convert virt to phy and shift used by dma */
#define HTS_PHY_ADDR(a)  (((unsigned long)__pa((void *)a)) >> HTS_PARTICLE_SHIFT)



/* if we add too much at each int we may take too much time */
#define FB_NOM_GOAL		HTS_NPARTICLES	/* # fb's in reserve for tcpu use */
//#define FB_NOM_GOAL		128	/* # fb's in reserve for tcpu use */


//	volatile unsigned int Tcpu_fb_cfg;	/* 0xb7c Tcpu fb config*/
#define HTD_TCPU_FBCFG_EN	0x80000000	/* enable */
#define HTD_TCPU_FBCFG_DSZ_MSB	0x0000fff8	/* len of desc ring msb */
#define HTD_TCPU_FBCFG_DSZ_LSB	0x00000007	/* len of desc ring lsb */

//	volatile unsigned int Fcpu_fb_cfg;	/* 0xb84 Fcpu fb config*/
#define HTD_FCPU_FBCFG_EN	0x80000000	/* enable */
#define HTD_FCPU_FBCFG_DSZ_MSB	0x0000fff8	/* len of desc ring msb */
#define HTD_FCPU_FBCFG_DSZ_LSB	0x00000007	/* len of desc ring lsb */o

#define FB_PARTICLE_SZ		0x0400	/* 1024 bytes */

#define NMAXHT_PG    0xC000


extern void *get_ht(unsigned long);
extern int hts_reset(void *);
extern int hts_mmap(void);
extern int hts_alloc(int);
extern int hts_setup_tcpu_rb(void *);
extern int hts_setup_tcpu_fb(void *);
extern int hts_add_fbuf(void *ht, int);
extern int hts_setup_rw_ptr(void *ht);
extern void hts_set_lpbk(void *ht, int);
extern int hts_tx(void *ht, unsigned int, unsigned int, unsigned int,
                  void *, unsigned int);
extern int hts_rx(void *ht, void *);
extern int hts_return_fbuf(void *ht, unsigned int);
extern int hts_set_intr_lvl(void *ht, unsigned int, unsigned int);
extern void hts_dump_regs(void *);
extern void hts_debug(void *);
extern void put_higig2_header(int, int, unsigned char*);
extern void put_eth_header_crc(char *, char *,
                               unsigned int *, unsigned char *);
#endif /* _HTDP_H_*/



/*---------------------------------------------------------------
$Log: htdp.h,v $
Revision 1.1  2013/07/22 19:50:39  mcharon
move hts headers to utah dir


$Endlog$
*/

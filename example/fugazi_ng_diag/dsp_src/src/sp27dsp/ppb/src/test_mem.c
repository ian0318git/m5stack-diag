/* $Id: test_mem.c,v 1.4 2012/08/15 14:52:23 srane Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/test_mem.c,v $
 *------------------------------------------------------------------
 * test_mem.c
 *      Graffham project: test LSI SP2704 DSP DDR3 SDRAM 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c)2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdio.h"
#include "diag_ppb.h"
#include "uart.h"
#include "debug_console.h"

/* NEW_ADDED_CODE */
#define LSI_MG_DDR3MEM_BASE 0xE0000000
#define LSI_MG_DDR3MEM_SIZE 0x1ff00000 /* 511 M will be tested */

#define PASSED 0
#define FAILED 1

/* defines for the above flag field */
#define QUITONERR  0x01    /* abort the test on first error */
#define ABBR_TEST  0x02    /* do only the abbreviated test */
#define INDIAGS    0x04    /* use the diagnostic environment provided */
#define RDADDR     0x08    /* separate read and write addresses */
#define MUTE       0x10    /* keep quiet about errors, etc. */
#define DRAM       0x20    /* DRAM present on the platform */
#define PARITY_EN  0x40    /* Parity enabled during a main or */
                           /* shared memory parity test (cancun only)*/

#define ROTATE_LEFT(x)  (((x) << 1) | (((x) & 0x80000000) >> 31))

struct testmem {
  unsigned *start, *rdaddr;
  signed int length;
  signed int passcount;
  unsigned char flag;
};

struct testdat {
    signed int rd_pat;            /* read verify pattern */
    signed int wr_pat;            /* write pattern */
    signed int flag;              /* 1 = increment, 0 = decrement */
};

static struct testdat dpatterns[] = {
    {0x5a5aa5a5, 0xa5a55a5a, 1},
    {0xa5a55a5a, 0x5a5aa5a5, 1},
    {0x5a5aa5a5, 0xa5a55a5a, 0},
    {0xa5a55a5a, 0x5a5aa5a5, 0},
    {0x5a5aa5a5, 0x3c3cc3c3, 1},
    {0x3c3cc3c3, 0xc3c33c3c, 0},
    {0xc3c33c3c, 0x3c3cc3c3, 1},
    {0x3c3cc3c3, 0xf0f0f0f0, 0},
    {0xf0f0f0f0, 0x0f0f0f0f, 1},
    {0x0f0f0f0f, 0xf0f0f0f0, 0},
    {0xf0f0f0f0, 0x00ff00ff, 1},
    {0x00ff00ff, 0xff00ff00, 0},
    {0xff00ff00, 0x00ff00ff, 1},
    {0x00ff00ff, 0x0000ffff, 0},
    {0x0000ffff, 0xffff0000, 1},
    {0xffff0000, 0x0000ffff, 0},
    {0x0000ffff, 0xffffffff, 1},
    {0xffffffff, 0x00000000, 0},
    {0x00000000, 0xffffffff, 1},
    {0xffffffff, 0x00000000, 0},
};
unsigned num_patrns = (sizeof(dpatterns)/sizeof(struct testdat));

static int rvw_mem(struct testmem *tmemp, struct testdat *dpatterns);
static uint32_t test_mem_addr(uint32_t low_mem, uint32_t high_mem);
static uint32_t test_mem_checkerboard(uint32_t low_mem, uint32_t high_mem);
static uint32_t test_mem_walk0(uint32_t low_mem, uint32_t high_mem);
static uint32_t test_mem_walk1(uint32_t low_mem, uint32_t high_mem);
static uint32_t test_mem_flipping(uint32_t low_mem, uint32_t high_mem);

/* Forwards declarations */
extern dspif_info_t *hd_if;
extern int sprintf(char *, const char *, ...);

/***********************************************************************
 *
 * Function: test_mem
 *
 * Description:  Test DSP DDR2 external SDRAM 
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint32_t test_mem (void)
{   
    uint32_t low_mem, high_mem;
    uint32_t ret = PASSED;

    low_mem = LSI_MG_DDR3MEM_BASE;
    high_mem = LSI_MG_DDR3MEM_BASE + LSI_MG_DDR3MEM_SIZE - 1;
    /* address line */
    if (ret == PASSED) {
        uart_puts("\r\naddress lines test");
        ret = test_mem_addr(low_mem, high_mem);
    } 
    /* data line */
    if (ret == PASSED) {
        uart_puts("\r\nwalking-ones test");
        ret = test_mem_walk1(low_mem, low_mem+1024);
    }
    if (ret == PASSED) {
        uart_puts("\r\nwalking-zeros test");
        ret = test_mem_walk0(low_mem, low_mem+1024);
    }
    /* all bits flipping */
    if (ret == PASSED) {
        uart_puts("\r\nbit flipping test");
        ret = test_mem_flipping(low_mem, low_mem+1024);
    }
    /* SR ?? */
    /* checkerboard */
    if (ret == PASSED) {
        uart_puts("\r\ncheckerboard test");
        ret = test_mem_checkerboard(low_mem, high_mem);
    }
    return (ret);
}

/**********************************************************************
 *
 * Function name:   rvw_mem()
 *
 * Description:
 *     This function increment or decrement through memory,
 * read and compare with rd_pat then write with wr_pat
 *
 * Input: test memory structure and test pattern
 *
 * Output: PASSED if successful, FAILED otherwise 
 *
 **********************************************************************
 */
static int
rvw_mem(struct testmem *tmemp, struct testdat *dpatterns)
{
    register uint32_t count; 
    register uint32_t rdata, rd_pat, wr_pat;
    register signed int adrinc;
    register uint32_t *addr_ptr;
    unsigned char flag;

    flag = dpatterns->flag;
    rd_pat = dpatterns->rd_pat;
    wr_pat = dpatterns->wr_pat;

    if (flag) {                 /* increment through memory */
        adrinc = 1;
        addr_ptr = (uint32_t *)(tmemp->start);
    } else {
        adrinc = -1;            /* decrement through memory */
        addr_ptr = (uint32_t *)(tmemp->start + tmemp->length/4 - 1);  /* length in bytes */
    }

    for (count = tmemp->length/4; count > 0; count--) {
        rdata = *addr_ptr;
        if (rdata != rd_pat) {
               sprintf((char *)&(hd_if->errmsg),"Memory read error when %s\n"
                "adr 0x%ld, expect 0x%ld, read 0x%ld\n",
                flag ? "ascending" : "descending",
                (unsigned long)addr_ptr, rd_pat, rdata);
            return(FAILED);
        }
        *addr_ptr = wr_pat;
        addr_ptr += adrinc;
    }
    return(PASSED);
}

/***********************************************************************
 *
 * Function: test_mem_addr
 *
 * Description: check DSP external memory.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
static uint32_t test_mem_addr(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr; 

    ptr = (uint32_t *) low_mem;        /* beginning of DARAM */ 
    
    sprintf((char *)&(hd_if->errmsg[0]), "address lines test\n"); 
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
            ptr = (uint32_t *) indx; 
			*ptr = (uint32_t)indx; 
    } 
    uart_puts("\r\nLast address written to = ");
    uart_put_long(indx, 16);
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *) indx; 
        if (*ptr != (uint32_t)indx) { 
            sprintf((char *)&(hd_if->errmsg[0]), 
                    "Failed address lines test at 0x%x. " 
                    "expected: 0x%x, received: 0x%x. ", 
                    (unsigned int)ptr, (unsigned int)indx, (unsigned int)*ptr);
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, (char *)hd_if->errmsg);
            return (FAILED); 
        }
    }
 
    return (PASSED);
}

/***********************************************************************
 *
 * Function: test_mem_walk1
 *
 * Description: check DSP external memory.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
static uint32_t test_mem_walk1(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr; 
    uint32_t indx2, tmp1; 

    ptr = (uint32_t *) low_mem;        /* beginning of DARAM */ 
    
    /* perform a 32-bit walking-ones test */ 
	sprintf((char *)&(hd_if->errmsg), "walking-ones test\n"); 
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
		ptr = (uint32_t *) indx;
		
        tmp1 = 0x1; 
        for (indx2 = 0; indx2 < 32; indx2++) { 
            *ptr = tmp1; 
            if (*ptr != tmp1) { 
                uart_puts("\r\nFailed walking one's test ");
                sprintf((char *)&(hd_if->errmsg), 
			"Failed 32-bit walking-ones test at 0x%x.\n" 
			"expected: 0x%x, received: 0x%x.\n", 
			(unsigned int)ptr, (unsigned int)tmp1, (unsigned int)*ptr); 
                uart_puts((char *)hd_if->errmsg);
                cterr('f', 0, "%s", hd_if->errmsg);
		return (FAILED); 
            } 
            tmp1 <<= 1; 
        } 
    } 
    uart_puts("\r\nLast address written to = ");
    uart_put_long(indx, 16);
 
    return (PASSED);
}


/***********************************************************************
 *
 * Function: test_mem_walk0
 *
 * Description: check DSP external memory.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
static uint32_t test_mem_walk0(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr; 
    uint32_t indx2, tmp1, tmp2; 

    ptr = (uint32_t *) low_mem;        /* beginning of DARAM */ 
    
    /* perform a 32-bit walking-zeroes test */ 
	sprintf((char *)&(hd_if->errmsg), "walking-zeros test\n"); 
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *) indx; 
        tmp2 = 0xfffffffe; 
        for (indx2 = 0; indx2 < 32; indx2++) { 
            *ptr = tmp2; 
            if ((tmp1 = *ptr) != tmp2) { 
                sprintf((char *)&(hd_if->errmsg), 
                        "Failed 32-bit walking-zeroes test at 0x%x.\n" 
                        "expected: 0x%x, received: 0x%x.\n", 
                        (unsigned int)ptr, (unsigned int)tmp2, (unsigned int)tmp1); 
                uart_puts((char *)hd_if->errmsg);
                cterr('f', 0, "%s", hd_if->errmsg);
                return (FAILED); 
            } 
            tmp2 = ROTATE_LEFT(tmp2); 
        } 
    } 
    uart_puts("\r\n Last address written to = ");
    uart_put_long(indx, 16);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: test_mem_cherckerboard
 *
 * Description: check DSP external memory.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
static uint32_t test_mem_checkerboard(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr; 

    ptr = (uint32_t *) low_mem;        /* beginning of DARAM */ 
    
    /* patterns test through DARAM */ 
    /* perform a 32-bit alternating 0x5a5a5a5a and 0xa5a5a5a5 test */ 
    sprintf((char *)&(hd_if->errmsg), "checkerboard test\n"); 
    /* pass 1  - write test data */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        *ptr = 0x5a5a5a5a; 
    }
    uart_puts("\r\n Last address written to = ");
    uart_put_long(indx, 16);
	/* pass 2 - read, check, write reverse pattern */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        if (*ptr != 0x5a5a5a5a) {
            sprintf((char *)&(hd_if->errmsg),
            "Failed checkerboard test at 0x%x.\n" 
	    "expected: 0x5a5a5a5a, received: 0x%x.\n", (unsigned int)ptr, (unsigned int)*ptr); 
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, "%s", hd_if->errmsg);
	    return (FAILED); 
        } 
	*ptr = 0xa5a5a5a5;
    }
    uart_puts("\r\n Last address written to = ");
    uart_put_long(indx, 16);
    /* pass 3 - read, check the reverse pattern */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        if (*ptr != 0xa5a5a5a5) {
            sprintf((char *)&(hd_if->errmsg), 
                "Failed checkerboard test at 0x%x.\n" 
                "expected: 0xa5a5a5a5, received: 0x%x.\n", 
                (unsigned int)ptr, (unsigned int)*ptr); 
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, "%s", hd_if->errmsg);
            return (FAILED); 
        } 
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: test_mem_flipping
 *
 * Description: check DSP external memory with flipping bit test.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
static uint32_t test_mem_flipping(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr, val; 

    ptr = (uint32_t *) low_mem;        /* beginning of DARAM */ 
    
    /* patterns test through DARAM */ 
    /* perform a 32-bit alternating 0x00000000 and 0xffffffff test */ 
    sprintf((char *)&(hd_if->errmsg), "bit flipping test\n"); 
    /* pass 1  - write test data */
   for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        *ptr = 0x0; 
    } 
    /* pass 2 - read, check, write reverse pattern */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        if (*ptr != 0x0) {
            sprintf((char *)&(hd_if->errmsg), "Failed checkerboard test at 0x%x.\n" 
                    "expected: 0x0, received: 0x%x.\n", (unsigned int)ptr, (unsigned int)*ptr); 
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, "%s", hd_if->errmsg);
            return (FAILED); 
        } 
        *ptr = 0xffffffff;
    }
    /* pass 3 - read, check, write reverse pattern */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        if (*ptr != 0xffffffff) {
            sprintf((char *)&(hd_if->errmsg), "Failed checkerboard test at 0x%x.\n" 
                    "expected: 0xffffffff, received: 0x%x.\n", (unsigned int)ptr, (unsigned int)*ptr); 
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, "%s", hd_if->errmsg);
            return (FAILED); 
        } 
        *ptr = 0x0;
    }
    /* pass 4 - read, check reverse pattern */
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
        ptr = (uint32_t *)indx; 
        if ((val = *ptr) != 0x0) {
             sprintf((char *)&(hd_if->errmsg), "Failed checkerboard test at 0x%x.\n" 
                     "expected: 0x0, received: 0x%x.\n", (unsigned int)ptr, (unsigned int)val); 
            uart_puts((char *)hd_if->errmsg);
            cterr('f', 0, "%s", hd_if->errmsg);
            return (FAILED); 
       } 
    }
   return (PASSED);
}

/***********************************************************************
 *
 * Function: test_mem_marchc
 *
 * Description: check DSP external memory.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint32_t test_mem_marchc(uint32_t low_mem, uint32_t high_mem)
{
    uint32_t indx;
    uint32_t *ptr; 
    struct testmem tmem;
    register struct testmem *tmemp = &tmem;
    uint32_t patrn;

    ptr = (uint32_t *)low_mem;        /* beginning of DARAM */ 
    
    /* March C Memory Test */
    sprintf((char *)&(hd_if->errmsg),"\nMarch C Mem test\n");
    for (indx = low_mem; indx < high_mem; (indx+=4)) { 
	ptr = (uint32_t *) indx;

	if ((indx % 0x0200000) == 0)
	    sprintf((char *)&(hd_if->bufmsg), "fill %#.8x with %#.8x,", 
		   (unsigned int)indx, (unsigned int)dpatterns[0].rd_pat);
		*ptr++ = dpatterns[0].rd_pat;
    }

    tmemp->start = (unsigned *) low_mem;
    tmemp->length  = high_mem - low_mem;
    tmemp->passcount = 1;  /* do complete mem test once */
    tmemp->flag = INDIAGS | DRAM;
    
    for (patrn = 0; patrn < num_patrns; patrn++) {
	/* do march test */
	if (rvw_mem (tmemp, &dpatterns[patrn])) 
	    return (FAILED);
	
	if ((patrn == 3) && (tmemp->flag & ABBR_TEST))
	    break;
    }
    return (PASSED);
}

/* 
 * $Log: test_mem.c,v $
 * Revision 1.4  2012/08/15 14:52:23  srane
 * cleanup code.
 *
 * Revision 1.3  2012/06/07 22:50:59  srane
 * TDM external loopback, ECC memory test
 *
 * Revision 1.2  2012/05/10 22:57:58  srane
 * Add TDM support. Adjust the linker sections.
 *
 * Revision 1.1  2012/04/18 09:44:02  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

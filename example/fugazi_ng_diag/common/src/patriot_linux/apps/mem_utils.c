/* $Id: mem_utils.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: mem_utils.c
 *
 * Description: Tests for memory
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stropts.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "defs.h"
#include "patriot_main.h"
#include "common_utils.h"
#include "ds3170.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_etsec.h"
#include "p1021_immap.h"
#include "p1021_espi.h"

static int fd_mem =0;       /* dev/mem --manages mmap */
static int fd_mem_mgr =0;   /* /dev/mem_mgr --manages malloc_nm */
static uint large_memory_usage_cnt = 0;
extern uchar err_msg[];
extern uchar dismem_msg[];

typedef struct mem_list_t_ {
    struct mem_info_t_ addr;
    struct mem_list_t_ *next;
    struct mem_list_t_ *prev;
    uint32_t in_use;
} mem_list_t;

static mem_list_t mem_list[MAX_LIST_SIZE];

static struct testdat dpatterns[] =
{
    { 0x5a5aa5a5, 0xa5a55a5a, 1},
    { 0xa5a55a5a, 0x5a5aa5a5, 1},
    { 0x5a5aa5a5, 0xa5a55a5a, 0},
    { 0xa5a55a5a, 0x5a5aa5a5, 0},
    { 0x5a5aa5a5, 0x3c3cc3c3, 1},
    { 0x3c3cc3c3, 0xc3c33c3c, 0},
    { 0xc3c33c3c, 0x3c3cc3c3, 1},
    { 0x3c3cc3c3, 0xf0f0f0f0, 0},
    { 0xf0f0f0f0, 0x0f0f0f0f, 1},
    { 0x0f0f0f0f, 0xf0f0f0f0, 0},
    { 0xf0f0f0f0, 0x00ff00ff, 1},
    { 0x00ff00ff, 0xff00ff00, 0},
    { 0xff00ff00, 0x00ff00ff, 1},
    { 0x00ff00ff, 0x0000ffff, 0},
    { 0x0000ffff, 0xffff0000, 1},
    { 0xffff0000, 0x0000ffff, 0},
    { 0x0000ffff, 0xffffffff, 1},
    { 0xffffffff, 0x00000000, 0},
    { 0x00000000, 0xffffffff, 1},
    { 0xffffffff, 0x00000000, 0},
    };
    
unsigned int num_patrns = (sizeof(dpatterns)/sizeof(struct testdat));


/**********************************************************************
 *
 * Function: getmemfree
 *
 * Description: Get the MemFree value from /pfoc/meminfo 
 *
 * Input: none
 *
 * Return: Free memory reported by reading /proc/meminfo
 ***********************************************************************
 */
ulong
getmemfree(void)
{
    struct sysinfo sys_info;
    ulong freemem;

    if(sysinfo(&sys_info) != 0) {
        printf("%s() sysinfo call failed\n", __FUNCTION__);
	return -1;
    }
#ifdef DEBUG
    printf("MemTotal= %ld KB %ld MB\n",
	   sys_info.totalram/ONE_K, sys_info.totalram/ONE_MEG);
    printf("MemFree= %ld %ld MB\n",
	   sys_info.freeram/ONE_K, sys_info.freeram/ONE_MEG);
#endif
    return(sys_info.freeram);

}



/**********************************************************************
 *
 * Function: addr_vtop
 *
 * Description: Use the vtop klm to find the physical address
 *              corresponding to the virtual address from the page
 *              table of the current process.
 *
 * Input: vir_addr - A virtual address of the current process
 *        phy_addr - pointer to the caller's buffer for holding the
 *                   output result (i.e. physical address)
 *
 * Return: PASSED/FAILED
 ************************************************************************
 */
int
addr_vtop(ulong vir_addr, ulong *phy_addr)
{
    int fd_vtop;
    volatile char *addr;
    ulong page_mask;

    /* Open the device file for operation
     */
    if ((fd_vtop = open("/dev/addr_vtop", O_RDWR)) < 0) {
        printf("%s(): Fail to open /dev/addr_vtop", __FUNCTION__);
        return(FAILED);
    }

    addr = (char *)vir_addr;

    /* The user virtual address is passed in addr to the vtop driver.
     * the pfn of the physical page is returned in addr by the driver.
     */
    if (read(fd_vtop, &addr, sizeof(addr)) < 0) {
        printf("%s(): Fail to read /dev/addr_vtop", __FUNCTION__);
	return(FAILED);
    }

    /* Compose the actual physical address by adding the low bits
     */
    page_mask = (ulong)getpagesize() - 1;
    *phy_addr = (ulong)addr | (vir_addr & page_mask);

    if (close(fd_vtop) < 0) {
        printf("%s(): Fail to close /dev/addr_vtop", __FUNCTION__);
	return(FAILED);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: mem_addr_eq_data_test()
 *
 * This function is used to test the main memory with data line equal
 * address line pattern.
 * Parameter real_start_addr is needed for those memories which are
 * accessed thru another mapping layer.
 *
 * For memories with normal accessing method, real_start_addr should be
 * set equal to start.
 *
 * Input: starting and ending address of the memory block under test.
 *        real_start_addr
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_addr_eq_data_test (ulong start, ulong end, ulong real_start)
{
    ulong *addrptr, value;
    ulong phy_addr;

    printf("\nmem_addr_eq_data_test start\n");
    /*
     * Data = Address test pattern.
     */
    for (addrptr = (ulong *)start; addrptr < (ulong *)end; addrptr++) {
#if DEBUG	
	if (((ulong)addrptr % TWO_MEG) == 0)
	    printf( "write addr %#lx,", (ulong)addrptr);
#endif	
        *addrptr = (ulong)addrptr;  /* write : data = address */
    }

    for (addrptr = (ulong *)start; addrptr < (ulong *)end; addrptr++) {
#if DEBUG	
	if (((ulong)addrptr % TWO_MEG) == 0)
	    printf("verify addr %#lx,", (ulong)addrptr);
#endif	
	value = *addrptr;
        if (value != (ulong)addrptr) {  /* read : data = address */
	    if (addr_vtop((ulong)addrptr, &phy_addr) == FAILED) {
	        sprintf(err_msg, "%s, [#%d]:%s failed\n", __FUNCTION__,
	        		__LINE__,__FILE__);
	        print_err(FALSE, err_msg, LVL_1);
	    }
            sprintf(err_msg, "%s, [#%d]:failure at phy-addr: %#lx "
            		"(vir-addr %#.lx) expected: %#lx actual: %#lx.",
            		__FUNCTION__, __LINE__, phy_addr, addrptr, addrptr, value);
            print_err(FALSE, err_msg, LVL_1);

	    return(FAILED);
        }
    }
    printf("\nmem_addr_eq_data_test end\n");
    return(PASSED);
}

/**********************************************************************
 *
 * Function: rvw_mem
 *
 * Description: Verify the read data against the patterns
 *
 * Input: none
 *
 * Return: PASSED/FAILED
 ***********************************************************************
 */
static int
rvw_mem(struct testmem * tmemp, struct testdat * dpatterns)
{
    int count, rdata, adrinc, rd_pat, wr_pat, i, bytes;
    unsigned int *addr_ptr;
    uchar flag;
    char msg[128];
        
    flag = dpatterns->flag;
    rd_pat = dpatterns->rd_pat;
    wr_pat = dpatterns->wr_pat;
    printf("\nrvw_mem\n");
    if(flag) {
        adrinc = tmemp->incr;
        addr_ptr = tmemp->start;
    } else {
        adrinc = -tmemp->incr;
        addr_ptr = tmemp->start + tmemp->length/4 -1; /* length in bytes */
    }
    for (i=0, count = tmemp->length/4; count > 0; count--, i++) {
        rdata = *addr_ptr;
        if (rdata != rd_pat) {
            sprintf(err_msg, "%s, [#%d]:failed @%#x: march test : "
            		"expect %#x; get %#x", __FUNCTION__, __LINE__,
		   (int)addr_ptr, rd_pat, rdata);
            print_err(FALSE, err_msg, LVL_1);
            return (FAILED);
        }
        *addr_ptr = wr_pat;
        addr_ptr += adrinc;
#if DEBUG        
        if ( (i % PRINT_INTERVAL) == 0) {
            printf("marchtest : reading @%#x; pattern %#X\n", addr_ptr,
		   rd_pat);
        }
#endif	
    } /* end of for */

    return (PASSED);
} /* end of rvw_mem */


/**********************************************************************
 *
 * Function: march_test
 *
 * Description: Doing marching test on memory 
 *
 * Input: Pointer to struct testmem
 *
 * Return: PASSED/FAILED
 ***********************************************************************
 */
unsigned int march_test( struct testmem *tmemp)
{

    volatile unsigned int *end_addr, *addr_ptr;
    short patrn;

    addr_ptr = tmemp->start;
    end_addr = (unsigned int *)((unsigned int) tmemp->start + tmemp->length);
    if (addr_ptr > end_addr) {
        return (FALSE);
    }

    while (addr_ptr < end_addr) {
        *addr_ptr++ = dpatterns[0].rd_pat;
    } /* end of while */

    for (patrn = 0; patrn < num_patrns; patrn++) {
        /* do march test */
        if (rvw_mem(tmemp, &dpatterns[patrn])== FAILED) {    
            return (FAILED);
        }
    } /* end of for */

    printf("march test passed\n");
    return (PASSED);
} /* end of march_test */



/**********************************************************************
 *
 * Function: module_mem_test
 * This function test memory on the module side
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
module_mem_test(void)
{

    struct testmem tmem;
    char *origaddr;
    ulong start, end, freememsz, ovrhd_sz, adjust_size;
    ulong page_mask;
    int result = FAILED;

    freememsz = getmemfree();

    /* Give 8% for other overhead and buffer */
    ovrhd_sz = freememsz * 0.08;
    /* Test the rest of the free memory */
    adjust_size = freememsz - ovrhd_sz;
#ifdef DEBUG    
    printf("\nfreememsz = 0x%08x", freememsz);
    printf("\n adjust_size =  0x%08x", adjust_size);
#endif
    /* Get the start point of free memory */
    origaddr = (char *)malloc(adjust_size);
    if (!origaddr) {
        sprintf(err_msg, "%s, [#%d]: malloc failed\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
	return(FAILED);
    }

    /* Align the start and end address at page boundary
     */
    page_mask = (getpagesize() - 1);    
    start = ((ulong)origaddr + page_mask) & ~page_mask;
    end = ((ulong)origaddr + adjust_size) & ~page_mask;    
#ifdef DEBUG
    printf("\nstart = 0x%08x\n", start);
    printf("\nend = 0x%08x\n", end);
#endif    
    result = mem_addr_eq_data_test(start, end, start);

    if (result != PASSED) {
	free(origaddr);
	return FAILED;
    }
    
    tmem.start = (unsigned int *)start;
    tmem.length = end - start;
    tmem.passcount = 1;
    tmem.incr = 1;
    result = march_test(&tmem);

    if (result != PASSED) {
	free(origaddr);
	return FAILED;
    }
    free(origaddr);
    return PASSED;

}

/**********************************************************************
 *
 * Function: dismem_x
 *
 * Description: Utility to display memory combine with sending to host
 *
 * Input: buf  - return buf
 *        addr - pointer to the address to display
 *        length - length of the memory to display
 *        disaddr - display address
 *        fldsize - size of the data to display (long, word or byte)
 *
 * Return: None
 ***********************************************************************
 */
void
dismem_x (boolean host_display, char *msg, unsigned char *addr, int length,  unsigned long disaddr,
       int fldsize)
{
    register int value = 0;
    register unsigned char i, j, c, linepos, asciistart = 0;
    register unsigned char *end = (addr + length);  /* the end boundary */
    register unsigned char *linend, *linestart;
    unsigned char temp_msg[1024];
    unsigned char save_addr[1024];
    unsigned char save_hex[1024];
    unsigned char save_ascii[1024];
    unsigned char save_buf[1024];
    unsigned char nextline[10];
    unsigned char padspc[10];
    unsigned int len;
#ifdef TRYWITHOUT
    /* if not a byte field size, round down to even boundary (for 68k) */
    if(fldsize != 1) addr = (unsigned char *)((unsigned long)addr & 0xfffffffe);
    /* we also must adjust disaddr */
#endif
    memset(save_buf, 0, sizeof(save_buf));
    memset((uchar *)temp_msg, 0, sizeof(temp_msg));

    if (host_display) {
        sprintf(nextline, "\n"); /* start on a new line */
        len = strlen(nextline);
        strncat(save_buf, nextline, len);   /* save to buffer display later */
    }
    else {
    	putchar('\n');
    }

    while(addr < end) {
        if (host_display) {
            linepos = sprintf(save_addr, "%.6lx  ", disaddr);
            len = strlen(save_addr);
            strncat(save_buf, save_addr, len);  /* save to buffer display later */
        }
        else {
            linepos = printf("%.6lx  ", disaddr);  /* display the line address */
        }
        linestart = addr;  /* save for ASCII representation */
        linend = (addr + 16);
        for(i=0; i<2; i++) {  /* display twice, as hex and as ascii */
            addr = linestart;
            while(addr < linend) {
                switch(fldsize) {
                case 2:
                    value = *(unsigned short *)addr;
                    addr += 2;
                    break;
                case 1:
                    fldsize = 1;
                    value = *addr++;
                    break;
                case 4:
                    value = *(unsigned int *)addr;  /* show 32 bit value */
                    addr += 4;
                    break;
                default:
                    printf("dismem: invalid operation size\n");
                    break;
                }

                if(i == 0) {
                    if (host_display) {
                        linepos += sprintf(save_hex, "%.*x ",fldsize * 2,value);
                        len = strlen(save_hex);
                        strncat(save_buf, save_hex, len);  /* save to buffer display later */
                    }
                    else {
                        linepos += printf("%.*x ",fldsize * 2,value); /* hex */
                    }
                }
                else {  /* ascii representation */
                    for(j=(fldsize * 8); j;) {
                        j -= 8;
                        c = (value >> j);
                        if (host_display) {
                            sprintf(save_ascii, "%c", (c >= ' ') && (c < 0x7f) ? c : '.');
                            len = strlen(save_ascii);
                            strncat(save_buf, save_ascii, len);   /* save to buffer display later */
                        }
                        else {
                            putchar((c >= ' ') && (c < 0x7f) ? c : '.');
                        }
                    }
                }
                if(addr >= end) {
                    break;
                }
           }

           if(!asciistart) asciistart = linepos;  /* record start 1st time */
           else while(linepos++ < asciistart) {
               if (host_display) {
                   sprintf(padspc, "%c", ' ');
                   len = strlen(padspc);
                   strncat(save_buf, padspc, len);   /* save to buffer display later */
               }
               else {
                   putchar(' ');  /* pad w/spaces */
               }
           }
        }
        if (host_display) {
            sprintf(nextline, "\n");
            len = strlen(nextline);
            strncat(save_buf, nextline, len);   /* save to buffer display later */
        }
        else {
            putchar('\n');
        }
        disaddr += 16;
    }

    if (host_display) {
        sprintf(temp_msg, "%s%s",msg, save_buf);
        len = strlen(temp_msg);
        print_err(host_display, temp_msg, LVL_X);
    }
}


/**********************************************************************
 *
 * Function: dismem
 *
 * Description: Utility to display memory
 *
 * Input: addr - pointer to the address to display
 *        length - length of the memory to display
 *        disaddr - display address
 *        fldsize - size of the data to display (long, word or byte)
 *
 * Return: None
 ***********************************************************************
 */
void
dismem(unsigned char *addr, int length,  unsigned long disaddr,
       int fldsize)
{
	dismem_x(FALSE, NULL, addr, length, disaddr, fldsize);
	return;
}


/**********************************************************************
 *
 * Function: display_mem
 *
 * Description: Wrap around function to allow users to display memory
 *
 * Input: none
 *
 * Return: None
 ***********************************************************************
 */
void
display_mem(void)
{

    unsigned long addr, length;

    addr = gethex_answer("Enter in hex the start offset", 0, 0,
			 MEM_SIZE_256_MB - 1);

    length = gethex_answer("Enter bytes to read in hex", 0, 0,
			   MEM_SIZE_256_MB - 1);

    if ((addr + length) > MEM_SIZE_256_MB) {
	printf("\nTry to display an address out of bound 0x%08x\n",
	       (addr + length));
	return;
    }

    /* Translate to the mapping address */
    addr = addr + VIR_ADRSPC_RAM;
    dismem((unsigned char *)addr, length, addr, 1);
    return;
    
}

/**********************************************************************
 *
 * Function: alt_mem
 *
 * Description: This utility allows users to alter memory
 *
 * Input: start_addr - starting address
 *        opsiz - operation size, long, word or byte
 *
 * Return: None
 ***********************************************************************
 */
int
alt_mem(unsigned long start_addr, int opsiz)
{
    union location {
	unsigned char byte;
	unsigned short word;
	unsigned lword;
    };
    register union location *addr;
    int tmp;
    register char *c_ptr;
    char inbuf[16];
    unsigned long val;
    
    /* Translate to the mapping address */
    addr = (union location *)(start_addr + VIR_ADRSPC_RAM);
    
    while(1) {
	printf("%.6lx = ", (unsigned long)addr);
	switch(opsiz) {
	case 2:
	    printf("%.4x",addr->word);
	    break;
	case 4:
	    printf("%.8x",addr->lword);
	    break;
	case 1:
	default:
	    opsiz = 1;
	    printf("%.2x",addr->byte);
	    break;
	}
	puts(" > ");
	c_ptr = inbuf;
	fgets((((char *)inbuf)), sizeof(inbuf), (stdin));
	
	switch(*c_ptr) {
	case 'x':
	case 'q': return(0);  /* quit */
	case ',':
	case 'p': /* prev location */
	    addr = (union location *)((unsigned long)addr - opsiz);
	    /* fall through */
	case 'r':
	case '.': /* same location */
	    continue;
	case 0: break; /* next location */
	default:
	    c_ptr = take_0x_addr(c_ptr);
	    tmp = getnum(c_ptr,16,&val);
	    if(tmp == 0) {
		printf("bad value \"%s\"\n",c_ptr);
		continue; /* same location again */
	    } else {
		switch(opsiz) {
		case 1:
		    addr->byte = val;
		    break;
		case 2:
		    addr->word = val;
		    break;
		case 4:
		    addr->lword = val;
		    break;
		}
		c_ptr += tmp;
		if(*c_ptr == '.')
		    continue;  /* same location */
	    }
	    break; /* next location */
	}
	addr = (union location *)((unsigned long)addr + opsiz);
	/* bump address */
    }
}


/**********************************************************************
 *
 * Function: modify_mem
 *
 * Description: Wrap around function to allow users to modify memory
 *
 * Input: none
 *
 * Return: None
 ***********************************************************************
 */
void
modify_mem(void)
{

    unsigned long addr;
    char ch;
    int op;

    addr = gethex_answer("Enter in hex the start address", 0, 0,
			 MEM_SIZE_256_MB - 1);

    ch = getc_answer("Enter the operation size 'l'ong, 'w'ord or 'b'yte",
		     "lwb", 'b'); 
    switch(ch) {  /* convert back to a number */
    case 'l':
	op = 4;
	break;
    case 'w':
	op = 2;
	break;
    case 'b':
	op = 1;
	break;
    }

    alt_mem(addr, op);
    return;
}


void mem_mgr_open(void){
    
    if (fd_mem_mgr)
        return;
    if ((fd_mem_mgr = open("/dev/mem_mgr", O_RDWR)) < 0) {
        perror("mem_mgr_open: Can't open /dev/fd_mem_mgr");
        exit(EXIT_FAILURE);
    }

}
void mem_mgr_close(void)
{
    if (fd_mem_mgr) {
        if (close(fd_mem_mgr) < 0) {
            perror("mem_mgr_close: Can't close");
            exit (EXIT_FAILURE);
        }
    }
    fd_mem_mgr = 0;
}


unsigned long
mmap_dev(mem_info_t *addr, const uint64_t phy_addr, const uint32_t size)
{
    unsigned long virt_addr;
    
    if (!fd_mem) {

        if ((fd_mem = open("/dev/mem", O_RDWR)) < 0) {
            perror("unable to open /dev/mem");
            exit(0);
        }
    }
    
    virt_addr = 0;
#ifndef LINUX_64BIT
    int page_size, offset;
    uint32_t start_phy_addr;
    unsigned long start_virt_addr;
    if (addr) {
        page_size = getpagesize();

        /* find number of bytes in excess of page_size granularity. for example,
	   if user wants to map to address 0x1388 (5K). we need to map to 0x1000
	   (4K) because mmap can map only granularity size. */
        offset = 0;
        start_phy_addr = phy_addr - offset;
        start_virt_addr = (unsigned long)mmap(0, size + offset,
                                                PROT_READ | PROT_WRITE,
                                                MAP_SHARED,
                                                fd_mem,
                                                (start_phy_addr));
        if (((void *)start_virt_addr) == MAP_FAILED) {
            perror("Can't mmap:\n");
            exit(EXIT_FAILURE);
        }
#ifdef DEBUG	
        printf("phy addr to be mmaped %lx, size = 0x%08x \n", start_phy_addr,
	       size);
#endif        
	/*malloc_nm and memops passe in pointer to addr */
	addr->mmaped_size = size + offset;
	/* mmaped location. may not be the same as the requested location,
	   if the requtested
	   location is not page_size granular */
	addr->start_phy_addr = start_phy_addr; 
	addr->start_virt_addr = start_virt_addr;
	/* virtual address returned by mmap be need to be adjusted. in case of
	   mapping to address, 0x1388 (5k), we will mmap to address 0x1000. then
	   we need to add 0x388 (offset) to get to actual location that user
	   wants. */
	addr->phy_addr = phy_addr; /* location requested by user */
	addr->virt_addr = start_virt_addr + offset; /*location requested by user*/
	addr->size = size;
    } else {
	/* here we assume phy_addr has correct page size ... in all cases
	   when we want to map to a device, ie, goofy or NM, we will be in this
	   else statement */
	virt_addr = (unsigned long)mmap64(0, size,
					  PROT_READ | PROT_WRITE,
					  MAP_SHARED,
					  fd_mem,
					  ((off64_t)phy_addr));
	if (((void *)virt_addr) == MAP_FAILED) {
	    perror("Can't mmap:\n");
	    exit(EXIT_FAILURE);
	}
	printf("NULL: phy addr to be mmaped %#llx; virtual address is %#llx\n",
	       phy_addr,
	       virt_addr);
    }
#else
    if (addr) {
	addr->phy_addr = addr->start_phy_addr = phy_addr;
	addr->virt_addr = addr->start_virt_addr =phy_addr | 0x8000000000000000ULL;
	addr->size = addr->mmaped_size = size;	
	
#ifdef MEM_DEBUG
	printf("in mmap_dev: virt_addr is %p\n", (void *)addr->virt_addr);
#endif
    } else {
	virt_addr = phy_addr | 0x8000000000000000ULL;
    }

#endif

    return virt_addr;
}

void *
malloc_nm (unsigned long size)
{
    mem_info_t addr;
    int i;
    int large_mem = 0;
    
    mem_mgr_open();
    size = (unsigned long)ALLIGN_128B(size); /* make sure size is at least
						multiple of 128Bytes */
    addr.size = size;
    

    /* get physical memory using klm */
    
    if (size <= MEM_MGR_MAX_KMALLOC_SIZE) {
	
	if (read(fd_mem_mgr, &addr, sizeof(addr))<0) {
	    perror("can't get physical mmeory\n");
	    exit (0);
	}
    } else {
	printf("\nsize is larger than 0x%08x\n", MEM_MGR_MAX_KMALLOC_SIZE);
	exit(0);
    }
#ifdef DEBUG    
    printf("\naddr.phy_addr in malloc_nm = 0x%08x", addr.phy_addr);
#endif    
    /* now map physical memory to virtual */
    mmap_dev(&addr, addr.phy_addr, addr.size);

    
    i = 0;
    while (mem_list[i].in_use) {
        i++;
    }
    if (i>=MAX_LIST_SIZE) {
        printf("Can't support that many mallocs...try increasing array size");
        exit(0);
    }

    memcpy((void *)&mem_list[i].addr, &addr, sizeof(addr));
    mem_list[i].in_use = 1;
#ifdef DEBUG    
    printf("\nmalloc_nm: mem_list[%d].addr.virt_addr = %#x\n",
	   i, mem_list[i].addr.virt_addr);
#endif    
    return (void *)mem_list[i].addr.virt_addr;
}

void
free_nm (void *s)
{
    unsigned long virt_addr = (unsigned long)s;
    uint32_t i = 0;
    uint32_t size = 0;

    mem_mgr_open();
#ifdef DEBUG    
    printf("\nvirt_addr in free_nm = %#x", virt_addr);
#endif    
    while (i<MAX_LIST_SIZE) {
        if (mem_list[i].addr.virt_addr == virt_addr) {
            size = mem_list[i].addr.size;
            break;
        }
        i++;
    }
    if (i >= MAX_LIST_SIZE) {
        printf("invalid memory address...can't free address %p\n",
	       (void *)virt_addr);
        fflush(0);
        printf("exiting.......hit return\n");getchar();
        exit(0);
    }


    /* unmap --release virt address */
    
    if (munmap((void *)mem_list[i].addr.virt_addr,
	       mem_list[i].addr.size) < 0) {
	perror("Can't munmap in free_nm");
	printf("mem_list[%d].addr.virt_addr = 0x%08x\n",
	       i, mem_list[i].addr.virt_addr);
	exit (EXIT_FAILURE);
    }
   
    /* free physical memeory */
    if (mem_list[i].addr.size <= MEM_MGR_MAX_KMALLOC_SIZE) {
        if (write(fd_mem_mgr, &mem_list[i].addr,
                  sizeof(struct mem_info_t_))<0) {
            printf("mem_list[%d].addr.phy_addr %p; virt %p...exiting program..."
                   "hit return to continue....\n", i,
                  (void *)mem_list[i].addr.phy_addr,
                   (void *)mem_list[i].addr.virt_addr);getchar();
            perror("can't write to mem_mgr.c:\n");
            fflush(0);
            exit (0);
        }
    } else {
	printf("\nsize is larger than 0x%08x\n", MEM_MGR_MAX_KMALLOC_SIZE);
	exit(0);
    }
    memset((void *)&mem_list[i], 0, sizeof(mem_list_t));  /*  make it available
							     again */
    
}

unsigned long phy_addr(unsigned long s)
{

    unsigned long virt_addr = (unsigned long)s;
    unsigned long start, end, offset;
    uint32_t i = 0;

    for (i=0;i<MAX_LIST_SIZE;i++) {
        start  = mem_list[i].addr.virt_addr;
        end = start + mem_list[i].addr.size;
        if ( (virt_addr >= start) && (virt_addr < end)) {
            offset = virt_addr - start;
            break;
        }
    }

    if (i >= MAX_LIST_SIZE) {
        printf("Unable to get physical address using virtual address %p\n",
	       (void *)virt_addr);
        for (i=0;i<MAX_LIST_SIZE;i++) {
            start  = mem_list[i].addr.virt_addr;
            end = start + mem_list[i].addr.size;
            if (mem_list[i].in_use) {
                printf("virtual[%d]: %p to %p; phsical:%p to %p; size %d [%#x]\n",
		       i,
                       (void *)(mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.size+mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.phy_addr),
                       (void *)(mem_list[i].addr.size +mem_list[i].addr.phy_addr),
                       mem_list[i].addr.size, mem_list[i].addr.size);
                fflush(0);
            }
	    
        }
        printf("....exiting program...hit return to continue...\n");getchar();
        exit(0);
    }

    /* all's well...return physical address */
    return (unsigned long)(mem_list[i].addr.phy_addr + offset);

}

unsigned long vir_addr(ulong s)
{
    unsigned long phy_addr = (unsigned long)s;
    unsigned long start, end, offset;
    uint32_t i = 0;

    for (i=0;i<MAX_LIST_SIZE;i++) {
        start  = mem_list[i].addr.phy_addr;
        end = start + mem_list[i].addr.size;
        if ( (phy_addr >= start) && (phy_addr < end)) {
            offset = phy_addr - start;
            break;          
        }
    }

    if (i>=MAX_LIST_SIZE) {
        printf("Unable to get virtual address using physical address %p\n",
	       (void *)phy_addr);
        for (i=0;i<MAX_LIST_SIZE;i++) {
            start  = mem_list[i].addr.virt_addr;
            end = start + mem_list[i].addr.size;
            if (mem_list[i].in_use) {
                printf("virtual[%d]: %p to %p; phsical:%p to %p; size %d [%#x]\n",
		       i,
                       (void *)(mem_list[i].addr.virt_addr),
                       (void *)(mem_list[i].addr.size+mem_list[i].addr.virt_addr),
                       (void *)mem_list[i].addr.phy_addr,
                       (void *)(mem_list[i].addr.size+ mem_list[i].addr.phy_addr),
                       mem_list[i].addr.size, mem_list[i].addr.size);
                fflush(0);
            }
                    
        }
        printf("....exiting program...hit return to continue...\n");getchar();
        exit(0);
    }

    /* all's well...return physical address */
    return (unsigned long)(mem_list[i].addr.virt_addr+offset);
}


/******** History ******** 
/*------------------------------------------------------------------------------
 * $Log: mem_utils.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.4  2012/12/04 13:04:44  steja
 * 1. backing back the DLB to ALB for framer interrupt
 * 2. add missing error message that left before.
 *
 * Revision 1.3  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.6  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.5  2012/01/19 23:50:12  huanngo
 * Fix bug in memory test
 *
 * Revision 1.1.4.4  2012/01/09 23:06:17  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.5  2011/08/06 00:17:39  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.4  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.3  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.2  2011/07/01 22:13:01  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.1  2011/06/28 06:27:55  huanngo
 * Update code to support Patriot SM
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

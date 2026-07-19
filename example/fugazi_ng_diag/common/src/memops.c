/* $Id: memops.c,v 1.11 2017/07/14 02:51:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/memops.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Memory operations.
** These utilities will access memory as words (16 bits) (default), bytes,
** or longwords (32 bits) with the exception of memtest which tests in
** longwords for efficiency.
*/
#include "endians.h"
#include "common.h"
#include "types.h"
#include "byteswap.h"
#include "testmem.h"
#include "setjmps.h"
#include "pcmap.h"
#include "signals.h"
#include "nvsysvars.h"
#include "error.h"
#include "monitor.h"
#include "queryflags.h"
#include "proto.h"
#include "defs.h"
#include <sys/mman.h>

#ifdef LINUX_APP
#include <unistd.h> /* support getopt */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "linux_api.h"
#include "mem_mgr.h"
#else
#include "strings.h"
#endif /* LINUX_APP */

/*
 * sizeof(word) MUST BE A POWER OF TWO
 * SO THAT wmask BELOW IS ALL ONES
 */
typedef int word;               /* "word" used for optimal copy speed */

#define wsize   sizeof(word)
#define wmask   (wsize - 1)
#define ADDR_BIT63_MASK  (0x8000000000000000ull)

#define MEMOPS_DEBUG

static char optstr[] = "lbw";
extern int optind;
extern int test_parity(unsigned long location, int);

#ifdef LINUX_APP
extern void query_user_ex(QUERYFLAG queryflag, unsigned long *tmp0, 
			  unsigned long *tmp1, unsigned long *tmp2,
                          unsigned long *tmp3);
#else /* Diagmon */
extern long memsize;
extern long shmemsize;
ulong rand (void);
ulong seed;
#endif /* LINUX_APP */

static int
getopsize(int argc, char *argv[])
{
    register int i;
    
    MEMOP_SIZE = 2;  /* default is word size operations */
    while((i = getopt(argc,argv,optstr)) >= 0) {
	switch(i) {
	case 'l':
	    MEMOP_SIZE = 4;
	    break;
	case 'b':
	    MEMOP_SIZE = 1;
	    break;
	case 'w':
	    MEMOP_SIZE = 2;
	    break;
	case 'd':
	    MEMOP_SIZE = 8;
	    break;
	case '?':
	    return(-1);
	}
    }
    return(0);
}

static int
getaddr(char *argptr, ulong *locptr)
{
#ifdef MEMOPS_DEBUG
    printf("\ngetaddr(locptr=%p)...\n", (void *)locptr);
#endif
    
    if(getnum(argptr,16, (uint *)locptr) == 0) return(-1);
    return(0);
}

/**********************************************************************
 *
 * Function: memops_mmap
 *
 * Description: Map physical address to Linux virtual address for
 * the specified length in bytes. If bit 63 of the phyaddr is set,
 * it is a cavium direct physical address and not mapping is needed.
 *
 * Input:
 * phyaddr - physical address
 * length - length in bytes
 * viraddr_p - pointer to the viraddr holder from the caller
 * pg_viraddr_p - pointer to the page viraddr holder from the caller
 *
 * Return: PASS/FAIL - call to Linux mmap is successful or not
 */
int
memops_mmap(ulong phyaddr, ulong length,
		  ulong *viraddr_p, ulong *pg_viraddr_p)
{
    ulong viraddr, pg_phyaddr, pg_viraddr;

    if (phyaddr & ADDR_BIT63_MASK) {
        /* If bit 63 is set, it is a cavium direct physical address, and
	 * no virtual addressing mapping is needed.
	 * Pass the 0 in pg_viraddr_p for memops_munmap to not
	 * do the unmap.
	 */ 
	*viraddr_p = phyaddr;
	*pg_viraddr_p = 0;
    }
    else {
        /* ioptov_mmap maps the phy_addr to the user space viraddr.
	 * This mapping must be released by calling munmap later.
	 */
        pg_phyaddr = pg_align_addr(phyaddr);
	if (FAIL == ioptov_mmap(pg_phyaddr, length, &pg_viraddr)) {
            perror("failed");
	    printf("%s() /dev/mem failed\n", __func__);
	    return(FAIL);
	}
	viraddr = pg_merge_addr(pg_viraddr, phyaddr);
	// printf("\nphyaddr %lx mapped to viraddr %lx\n", phyaddr, viraddr);

	*viraddr_p = viraddr;
	*pg_viraddr_p = pg_viraddr;
    }

    return(PASS);
}

/**********************************************************************
 *
 * Function: memops_munmap
 *
 * Description: Unmap the virtual address acquired by memops_mmap 
 *
 * Input:
 * pg_viraddr - page viraddr holder
 * length - length in bytes
 *
 * Return: return value from Linux munmap call
 */
int
memops_munmap(ulong pg_viraddr, ulong length)
{
    if (pg_viraddr == 0) {
        /* memops_mmap did not do a mapping
	 */
        return(PASS);
    }
    else {
        return(munmap((void *)pg_viraddr, length));
    }
}

int
linux_cmp_mem(int argc, char *argv[])
{
    ulong src_viraddr, src_pg_viraddr, dst_viraddr, dst_pg_viraddr;
    int (*cmpfunc)() = 0;    /* to supress warning from compiler  :^(*/

    printf("Note: Physical address should be entered in the query.\n");
    if(argc == 1) {
        query_user_ex(QU_SOURCE | QU_DEST | QU_SIZE | QU_OPSIZ ,
		      &MEMOP_SRCADDR, &MEMOP_DESTADDR,
		      &MEMOP_LENGTH, (ulong *)&MEMOP_SIZE);
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_SRCADDR) < 0 ||
	   getaddr(argv[optind++],&MEMOP_DESTADDR) < 0 ||
	   getnum(argv[optind++],16,(uint *)&MEMOP_LENGTH) == 0 || optind != argc) {
	    printf("usage: %s [-%s] addr0 addr1 length (in bytes)\n",
		   argv[0],optstr);
	    return(1);
	}
    }

    if (memops_mmap(MEMOP_SRCADDR, MEMOP_LENGTH,
		    &src_viraddr, &src_pg_viraddr) == FAIL) {
        printf("%s() source addr mmap failed\n", __FUNCTION__);
	return(FAIL);
    }

    if (memops_mmap(MEMOP_DESTADDR, MEMOP_LENGTH,
		    &dst_viraddr, &dst_pg_viraddr) == FAIL) {
        printf("%s() destination addr mmap failed\n", __FUNCTION__);
	return(FAIL);
    }

    switch(MEMOP_SIZE) {
    case 1:
	cmpfunc = cmpbyte;
	break;
    case 2:
	cmpfunc = cmpword;
	break;
    case 4:
	cmpfunc = cmplword;
	break;
    case 8:
	cmpfunc = cmpdlword;
	break;
    }
    (*cmpfunc)(src_viraddr, dst_viraddr, MEMOP_LENGTH);

    memops_munmap(src_pg_viraddr, MEMOP_LENGTH);
    memops_munmap(dst_pg_viraddr, MEMOP_LENGTH);
    return(PASS);
}

int
cmp_mem(int argc, char *argv[])
{
    return(linux_cmp_mem(argc, argv));
}

int
cmpbyte(unsigned char *addr0, unsigned char *addr1, int length)
{
    register unsigned char dat0, dat1, *end;
    int i = 0, j = 0, retval = 0;
    
    end = (unsigned char *)((unsigned long)addr0 + length);

    /* 
     * even if more than the first 5 data miscompare, 
     * only display max of 5 errors 
     */
    while ((addr0 < end) && (i <= 5)) {
	dat0 = *addr0++;
	dat1 = *addr1++;
	if (dat0 != dat1) {
            if (i < 5)
                printf("\nbyte %#x miscompare: %p = %#.2x    %p = %#.2x",
                       j, (void *)(addr0-1), dat0,
                       (void *)(addr1-1), dat1);
            else if (i == 5)
                printf("\n");
	    retval = 1;
	    i++;
	}
	j++;
    }
    return(retval);
}

int
cmpword(unsigned short *addr0, unsigned short *addr1, int length)
{
    register unsigned short dat0, dat1, *end;
    int i = 0, j = 0, retval = 0;

    end = (unsigned short *)((unsigned long)addr0 + length);

    /* 
     * even if more than the first 5 data miscompare, 
     * only display max of 5 errors 
     */
    while ((addr0 < end) && (i <= 5)) {
	dat0 = *addr0++;
	dat1 = *addr1++;
	if (dat0 != dat1) {
            if (i < 5)
                printf("\nword %#x miscompare: %p = %#.4x    %p = %#.4x",
                       j, (void *)(addr0-1), dat0, (void *)(addr1-1), dat1);
            else if (i == 5)
                printf("\n");
	    retval = 1;
	    i++;
	}
	j++;
    }
    return(retval);
}

int
cmplword(unsigned *addr0, unsigned *addr1, int length)
{
    register unsigned dat0, dat1, *end;
    int i = 0, j = 0, retval = 0;

    end = (unsigned *)((unsigned long)addr0 + length);

    /* 
     * even if more than the first 5 data miscompare, 
     * only display max of 5 errors 
     */
    while ((addr0 < end) && (i <= 5)) {
	dat0 = *addr0++;
	dat1 = *addr1++;
	if (dat0 != dat1) {
            if (i < 5)
	        printf("\nlword %#x miscompare: %p = %#.8x    %p = %#.8x",
	               j, (void *)(addr0-1), dat0, (void *)(addr1-1), dat1);
            else if (i == 5)
                printf("\n");
	    retval = 1;
	    i++;
	}
	j++;
    }
    return(retval);
}

int
cmpdlword(unsigned long *addr0, unsigned long *addr1, int length)
{
    register unsigned long dat0, dat1, *end;
    int i = 0, j = 0, retval = 0;

    end = (unsigned long *)((unsigned long)addr0 + length);

    /* 
     * even if more than the first 5 data miscompare, 
     * only display max of 5 errors 
     */
    while ((addr0 < end) && (i <= 5)) {
	dat0 = *addr0++;
	dat1 = *addr1++;
	if (dat0 != dat1) {
            if (i < 5)
	        printf("\ndlword %#x miscompare: %#.8lx = %#.8lx    %#.8lx = %#.8lx",
	               j, (ulong)(addr0-1), dat0, (ulong)(addr1-1), dat1);
            else if (i == 5)
                printf("\n");
	    retval = 1;
	    i++;
	}
	j++;
    }
    return(retval);
}

#ifdef LINUX_APP
int
linux_mov_mem(int argc, char *argv[])
{
    ulong src_viraddr, src_pg_viraddr, dst_viraddr, dst_pg_viraddr;
    void (*movfunc)() = 0;    /* to supress warning from compiler  :^(*/

    printf("Note: Physical address should be entered in the query.\n");
    if(argc == 1) {
        query_user_ex(QU_SOURCE | QU_DEST | QU_SIZE | QU_OPSIZ ,
		      &MEMOP_SRCADDR, &MEMOP_DESTADDR,
		      &MEMOP_LENGTH, (ulong *)&MEMOP_SIZE);
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_SRCADDR) < 0 ||
	   getaddr(argv[optind++],&MEMOP_DESTADDR) < 0 ||
	   getnum(argv[optind++],16,(uint *)&MEMOP_LENGTH) == 0 || optind != argc) {
	    printf("usage: %s [-%s] addr0 addr1 length (in bytes)\n",
		   argv[0],optstr);
	    return(1);
	}
    }

    if (memops_mmap(MEMOP_SRCADDR, MEMOP_LENGTH,
		    &src_viraddr, &src_pg_viraddr) == FAIL) {
        printf("%s() source addr mmap failed\n", __FUNCTION__);
	return(FAIL);
    }

    if (memops_mmap(MEMOP_DESTADDR, MEMOP_LENGTH,
		    &dst_viraddr, &dst_pg_viraddr) == FAIL) {
        printf("%s() destination addr mmap failed\n", __FUNCTION__);
	return(FAIL);
    }

    switch(MEMOP_SIZE) {
    case 1:
	movfunc = movbyte;
	break;
    case 2:
	movfunc = movword;
	break;
    case 4:
	movfunc = movlword;
	break;
    case 8:
	movfunc = movdlword;
	break;
    }
    (*movfunc)(src_viraddr, dst_viraddr, MEMOP_LENGTH);

    memops_munmap(src_pg_viraddr, MEMOP_LENGTH);
    memops_munmap(dst_pg_viraddr, MEMOP_LENGTH);
    return(PASS);
}

#endif //LINUX_APP

int
mov_mem(int argc, char *argv[])
{
#ifdef LINUX_APP
    return(linux_mov_mem(argc, argv));
#else
    void (*movfunc)() = 0;    /* to supress warning from compiler  :^(*/

    if(argc == 1) {
	query_user(QU_SOURCE | QU_DEST | QU_SIZE | QU_OPSIZ ,
		   &MEMOP_SRCADDR, &MEMOP_DESTADDR,
		   &MEMOP_LENGTH, &MEMOP_SIZE);
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_SRCADDR) < 0 ||
	   getaddr(argv[optind++],&MEMOP_DESTADDR) < 0 ||
	   getnum(argv[optind++],16,(int *)&MEMOP_LENGTH) == 0 || optind != argc) {
	    printf("usage: %s [-%s] addr0 addr1 length (in bytes)\n",
		   argv[0],optstr);
	    return(1);
	}
    }
    switch(MEMOP_SIZE) {
    case 1:
	movfunc = movbyte;
	break;
    case 2:
	movfunc = movword;
	break;
    case 4:
	movfunc = movlword;
	break;
    }
    (*movfunc)(MEMOP_SRCADDR,MEMOP_DESTADDR,MEMOP_LENGTH);
    return(0);
#endif //LINUX_APP
}

void
movbyte(unsigned char *addr0, unsigned char *addr1, int length)
{
  register unsigned char *end;

  end = (unsigned char *)((unsigned long)addr0 + length);
  while((ulong)addr0 < (ulong)end) {
    *addr1++ = *addr0++;
  }
}

void
movword(unsigned short *addr0, unsigned short *addr1, int length)
{
  register unsigned short *end;

  end = (unsigned short *)((unsigned long)addr0 + length);
  while((ulong)addr0 < (ulong)end) {
    *addr1++ = *addr0++;
  }
}

void
movlword(unsigned *addr0, unsigned *addr1, int length)
{
  register unsigned int *end;

  end = (unsigned int *)((unsigned long)addr0 + length);
  while((ulong)addr0 < (ulong)end) {
    *addr1++ = *addr0++;
  }
}

void
movdlword(unsigned long *addr0, unsigned long *addr1, int length)
{
  register unsigned long *end;

  end = (unsigned long *)((unsigned long)addr0 + length);
  while((ulong)addr0 < (ulong)end) {
    *addr1++ = *addr0++;
  }
}

int
linux_fil_mem(int argc, char *argv[])
{
    ulong addr, viraddr, pg_viraddr;

    /* 
    ** check for incrementing fill command
    ** note that argv is is chacked for validity - when this
    ** routine is called from the menu() routine, argv will
    ** be zero
    */
    if(argv && strcmp(argv[0],"ifill") == 0) hkeepflags |= H_INCFILL;
    else hkeepflags &= ~H_INCFILL;

    printf("Note: Physical address should be entered in the query.\n");
    if(argc == 1) {
        query_user_ex(QU_START | QU_SIZE | QU_VALUE | QU_OPSIZ ,
		   (unsigned long *)&MEMOP_START,
                      (unsigned long *)&MEMOP_LENGTH,
		   (unsigned long *)&MEMOP_VALUE,
                    (unsigned long *)&(MEMOP_SIZE));
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0 ||
	   getaddr(argv[optind++],&MEMOP_LENGTH) < 0 ||
	   getnum(argv[optind++],16,(uint *)&MEMOP_VALUE) == 0 || optind != argc) {
	    printf("usage: %s [-%s] addr length (in bytes) value\n",
		   argv[0],optstr);
	    return(1);
	}
    }

    if (memops_mmap(MEMOP_START, MEMOP_LENGTH,
		    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    addr = viraddr;
    switch(MEMOP_SIZE) {
    case 1:
	filbyte((uchar *)addr,MEMOP_LENGTH,MEMOP_VALUE);
	break;
    case 2:
	filword((ushort *)addr,MEMOP_LENGTH,MEMOP_VALUE);
	break;
    case 4:
	fillword((uint *)addr,MEMOP_LENGTH,MEMOP_VALUE);
	break;
    case 8:
	fildlword((ulong *)addr,MEMOP_LENGTH,MEMOP_VALUE);
	break;
    default:
        printf("fil_mem: wrong mem operation size\n");
        break;
    }

    memops_munmap(pg_viraddr, MEMOP_LENGTH);
    return(PASS);
}

int
fil_mem(int argc, char *argv[])
{
    return(linux_fil_mem(argc, argv));
}

void
filbyte(unsigned char *addr, int length, unsigned char val)
{
  register unsigned char *end = (unsigned char *)((unsigned long)addr + length);

  if(hkeepflags & H_INCFILL) while(addr < end) *addr++ = val++;
  else while(addr < end) *addr++ = val;
}

void
filword(unsigned short *addr, int length, unsigned short val)
{
  register unsigned short *end = (unsigned short *)((unsigned long)addr + length);
  if(hkeepflags & H_INCFILL) while(addr < end) *addr++ = val++;
  else while(addr < end) *addr++ = val;
}

void
fillword(unsigned int *addr, int length, unsigned int val)
{
  register unsigned int *end = (unsigned int *)((unsigned long)addr + length);
  if(hkeepflags & H_INCFILL) while(addr < end) *addr++ = val++;
  else while(addr < end) *addr++ = val;
}

/* Fill double long word */
void
fildlword(unsigned long *addr, int length, unsigned long val)
{
  register unsigned long *end = (unsigned long *)((unsigned long)addr + length);
  if(hkeepflags & H_INCFILL) while(addr < end) *addr++ = val++;
  else while(addr < end) *addr++ = val;
}

int
dis_mem(int argc, char *argv[])
{
    ulong val = 0;
    ulong viraddr, pg_viraddr;
    if(argc == 1) {
#ifdef DEBUG
	printf("\nMEMOP_START @%#p MEMOP_LENGTH @%#.8x MEMOP_SIZE @%#.8x\n",
	       MEMOP_START, MEMOP_LENGTH, MEMOP_SIZE);
#endif

        query_user_ex(QU_START | QU_SIZE | QU_OPSIZ,
		   (unsigned long *)&MEMOP_START,
                      (unsigned long *)&MEMOP_LENGTH,
                      (unsigned long *)&MEMOP_SIZE,
                      (unsigned long *)&val);
    } else {
	if(getopsize(argc,argv) < 0 ||
	   (argc - optind) != 2 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0 ||  /* and physical addr */
	   getnum(argv[optind],16,(uint *)&MEMOP_LENGTH) == 0) {
	    printf("usage: %s [-%s] addr length (in bytes)\n",argv[0],optstr);
	    return(1);
	}
    }

    if (memops_mmap(MEMOP_START, FOUR_K,
		    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    dismem((unsigned char *)viraddr, MEMOP_LENGTH, MEMOP_START, MEMOP_SIZE);

    memops_munmap(pg_viraddr, MEMOP_LENGTH);


    return PASSED;

}

void
dismem(unsigned char *addr, int length, unsigned long disaddr, int fldsize)
{
  register utype_t value = 0;
  register unsigned char i, j, c, linepos, asciistart = 0;
  register unsigned char *end = (addr + length);  /* the end boundary */
  register unsigned char *linend, *linestart;


#ifdef TRYWITHOUT
  /* if not a byte field size, round down to even boundary (for 68k) */
  if(fldsize != 1) addr = (unsigned char *)((unsigned long)addr & 0xfffffffe);
  /* we also must adjust disaddr */
#endif

  putchar('\n'); /* start on a new line */

  while(addr < end) {
    linepos = printf("%.6lx  ", disaddr);  /* display the line address */
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
            value = *(unsigned int *)addr;  /* show 32 or 64 bit value */
            addr += 4;
          break;
	case 8: /* 64 bit long word */
            value = *(unsigned long *)addr;  /* show 32 or 64 bit value */
            addr += 8;
          break;
        default:
            printf("dismem: invalid operation size\n");
	  break;
	}

	if(i == 0) linepos += printf("%.*lx ", fldsize * 2, value);  /* hex */

	else {  /* ascii representation */
#ifdef NATIVE
	  for(j=0; j<(fldsize * 8); j += 8) {
#else
	  for(j=(fldsize * 8); j;) {
	    j -= 8;
#endif
	    c = (value >> j);
	    putchar((c >= ' ') && (c < 0x7f) ? c : '.');
	  }
	}
	if(addr >= end) {
            break;
        }

      }
      if(!asciistart) asciistart = linepos;  /* record start 1st time */
      else while(linepos++ < asciistart) putchar(' ');  /* pad w/spaces */
    }
    putchar('\n');
    disaddr += 16;
    }

}

int
linux_alt_mem(unsigned long phyaddr, unsigned long viraddr)
{
    union location {
        unsigned char byte;
        unsigned short word;
        unsigned int lword;
        unsigned long dword;
    };
    register union location *addr;
    int opsiz, tmp;
    unsigned long startaddr = phyaddr;
    register char *c_ptr;
    char inbuf[24];
    ulong val = 0;
    //    ulong viraddr, phyaddr, pg_viraddr;
    //    phyaddr = MEMOP_START;
    addr = (union location *)viraddr;
    opsiz = MEMOP_SIZE;
    //    printf("linux_alt_mem: opsize %d\n", opsiz);
    while(1) {
        printf("%.6lx (va %.6lx) = ", phyaddr, (unsigned long)addr);
	switch(opsiz) {
	case 2:
	    printf("%.4x",addr->word);
	    break;
	case 4:
	    printf("%.8x",addr->lword);
	    break;
	case 8:
	    printf("%#.8lx",addr->dword);
	    break;
	case 1:
	default:
	    opsiz = 1;
	    printf("%.2x",addr->byte);
	    break;
	}

	fputs(" > ", stdout);

	c_ptr = inbuf;
        if (get_line(inbuf, sizeof(inbuf))) {


            switch(*c_ptr) {
            case 'x':
            case 'q':
                return(PASS);  /* quit */
            case ',':
            case 'p': /* prev location */
                if ((phyaddr-opsiz) >= startaddr) {
                    addr = (union location *)((unsigned long)addr - opsiz);
                    phyaddr -= opsiz;
                } 

                /* fall through */
            case 'r':
            case '.': /* same location */
                continue;
            case 0xa: // new line character
            case 0xd: // carriage return
                break; /* next location */
            default:
                c_ptr = take_0x_addr(c_ptr);
                tmp = getnum(c_ptr,16, (uint *)&val);
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
                    case 8:
                        addr->dword = val;
                        break;
                    }
                    c_ptr += tmp;
                    if(*c_ptr == '.')
                        continue;  /* same location */
                }
                break; /* next location */
            }
        }
        addr = (union location *)((unsigned long)addr + opsiz);
        phyaddr += opsiz;
            /* bump address */
    }
}

int
alt_mem(int argc, char *argv[])
{
    ulong val = 0;
    ulong viraddr, pg_viraddr;
    ulong mapsz = FOUR_K; // Thought 4K should be enough for peek/poke

    printf("Note: Only physical address is accepted.\n");
    if(argc == 1) {
        query_user_ex(QU_START | QU_OPSIZ, &MEMOP_START,
		      (ulong *)&MEMOP_SIZE, &val, &val);
    } else {
        if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0 || optind != argc) {
	    printf("usage: %s [-%s] addr\n",argv[0],optstr);
	    return(1);
	}
    }

    //    printf("memops.c: memory length %d  line %d\n", MEMOP_LENGTH, __LINE__);
    if (memops_mmap(MEMOP_START, mapsz,
		    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

   linux_alt_mem(MEMOP_START, viraddr);

   memops_munmap(pg_viraddr, mapsz); // release the memory map
    
   return PASSED;

}

/*
** Call a routine at specified address (location) passing argc and argv
** to represent argv[2...].
** The value returned from the routine is returned to the caller.
*/
#ifndef LINUX_APP 
int
jump(int argc, char *argv[])
{
  int (*location)();

  if(argc < 2 || getnum(argv[1],16,(uint *)&location) == 0) {
    printf("usage: %s location [arg1, arg2 ... argN]\n",argv[0]);
    return(1);
  }
  return((*location)(argc - 2, &argv[2]));
}

/*
** Similar to above except that argv[2...5] are converted to hex numbers
** before they are passed to the routine and the return value is printed
** on the console upon return;
*/
int
call(int argc, char *argv[])
{
  register char i;
  uint arg[5];

  if(argc < 2 || argc > 6) {
usage:
    printf("usage: %s address [arg0 arg1 arg2 arg3]\n",argv[0]);
    return(1);
  }
  for(i=1; i < argc; i++)
    if(getnum(argv[(int)i], 16, (uint *)&arg[i-1]) == 0) {
      puts("all arguments must be ascii hex numbers\n");
      goto usage;
    }
  i--;
  while(i < 5) arg[(int)i++] = 0;
  printf("*** return(0x%02x) ***\n",
	 (*(int (*)())arg[0])(arg[1],arg[2],arg[3],arg[4]));
  return(0);
}
#endif
/*
** memdebug - write a longword location with 0x55555555, write the next
** location with 0xaaaaaaaa, read with verify the first location, clear
** the two locations and do over continuously.  Written values may be
** changed with the -p option.
** Report a verify error.
** Bus slam test consists of writing zeroes into the locations then
** verify and update with 0xffffffff;  This makes the data bus slam
** between all zeroes and all ones. Report a verify error with a 
** trigger.
*/
int
memdebug(int argc, char *argv[])
{
    int i, numchars;
    register unsigned long *locptr, *nlocptr;
    register unsigned long zero = 0;
    register unsigned long pat1 = 0x55555555;
    register unsigned long pat2 = 0xaaaaaaaa;
    unsigned long ltemp;
    char slam = 0;
    register unsigned long *trigger, temp;
    static char optstr[] = "st:p:";
    extern int optind;
    extern char *optarg;
    ulong viraddr, pg_viraddr, tr_viraddr, tr_pg_viraddr;
    ulong mapsz = FOUR_K; // Thought 4K should be enough for peek/poke

    printf("Note: Only physical address is accepted.\n");

    if (temp) {
        /* 
         * Variable "temp" is needed in this function
         * Latest compiler will deem this variable as unused so add comments here
         */
    }

    if(argc == 1) {
#ifdef LINUX_APP
        unsigned long val = 0;
	query_user_ex(QU_START | QU_TRIGGER, &MEMOP_START, &MEMOP_TRIGGER,
                      &val, &val);
#else
	query_user(QU_START | QU_TRIGGER, &MEMOP_START, &MEMOP_TRIGGER);
#endif
    } else {
	while((i = getopt(argc,argv,optstr)) >= 0) {
	    switch(i) {
	    case 't':  /* specify a trigger */
		getnum(optarg,16,(uint *)&MEMOP_TRIGGER);
		break;
	    case 's':  /* bus slam test */
		slam = 1;
		break;
	    case 'p':  /* specify write patterns */
		numchars = getnum(optarg,16, (uint *)&ltemp);
		if(numchars) {
		    pat1 = ltemp;
		    optarg += numchars;
		    if(*optarg == ',') {  /* pat2 specified too? */
			if(getnum(++optarg,16, (uint *)&ltemp)) pat2 = ltemp;
		    }
		}
		break;
	    default:
usage:
		printf("usage: %s [-s] startaddr\n",argv[0]);
		puts("-p:  specify write patterns <pattern1>[,<pattern2>]\n"
		     "-t:  specify a scope trigger address\n"
		     "-s   bus slam test\n");
		return(1);
	    }
	}
	if((argc - optind) != 1 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0) goto usage;
    }



    if (memops_mmap(MEMOP_START, mapsz,
                    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    if (memops_mmap(MEMOP_TRIGGER, mapsz,
                    &tr_viraddr, &tr_pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    locptr = (unsigned long *)viraddr;
    nlocptr = (unsigned long *)(viraddr + 8); /* support 64 bit */
    trigger = (unsigned long *)tr_viraddr;

    quitmsg();
    if(slam) {
	pat1 = 0xffffffff;
	pat2 = 0x0;
	*locptr = pat1;
	*nlocptr = pat1;
	while(1) {
	    if(*locptr != pat1) temp = *trigger;
	    *locptr = pat2;
	    if(*nlocptr != pat1) temp = *trigger;
	    *nlocptr = pat2;
	    if(*locptr != pat2) temp = *trigger;
	    *locptr = pat1;
	    if(*nlocptr != pat2) temp = *trigger;
	    *nlocptr = pat1;
	}
    } else {
	while(1) {

	    *locptr = zero;
	    *nlocptr = zero;
	    *locptr = pat1;
	    *nlocptr = pat2;
	    if(*locptr != pat1) {
		temp = *trigger;
		puts("\n1error\n");
	    }

	    if(*nlocptr != pat2) {
		temp = *trigger;
		puts("\n2error\n");
	    }
	}
    }

    memops_munmap(pg_viraddr, mapsz); 
    memops_munmap(tr_pg_viraddr, mapsz); 

    return(0);
}

int
memtest(int argc, char *argv[])
{
    ulong viraddr, pg_viraddr;
    register int i;
    struct testmem tmem;
    register struct testmem *tmemp = &tmem;
    static char optstr[] = "qmap:r:";
    extern int optind;
    extern char *optarg;
    unsigned long temp = 0;

    tmemp->flag = 0;
    tmemp->passcount = 0;

    printf("Note: Only physical address is accepted.\n");

    if(argc == 1) {
#ifdef LINUX_APP
        query_user_ex(QU_START | QU_SIZE | QU_PASSES | QU_ABBREV,
		   &MEMOP_START, &MEMOP_LENGTH, &tmemp->passcount, 
		   &temp);
#else
	query_user(QU_START | QU_SIZE | QU_PASSES | QU_ABBREV,
		   &MEMOP_START, &MEMOP_LENGTH, &tmemp->passcount, 
		   &temp);
#endif
        if (temp) tmemp->flag |= ABBR_TEST;
	tmemp->flag |= INDIAG;
#if defined (CANCUN) || defined (XX) || defined (SIERRA) || defined (JANEIRO) || defined (ATLANTIS)
	tmemp->flag |= DRAM;
#endif
    } else {
	while((i = getopt(argc,argv,optstr)) >= 0) {
	    switch(i) {
	    case 'q':  /* quit on error */
		tmemp->flag |= QUITONERR;
		break;
	    case 'm':  /* mute */
		tmemp->flag |= MUTE;
		break;
	    case 'a':  /* abbreviated test */
		tmemp->flag |= (ABBR_TEST | DRAM | INDIAG);
		tmemp->passcount = 1;
		break;
	    case 'p':  /* specified pass count */
		getnum(optarg,16, (uint *)&tmemp->passcount);
		break;
	    case 'r':
		getnum(optarg,16,(uint *)&tmemp->rdaddr);
		tmemp->flag |= RDADDR;
		break;
	    default:
usage:
		printf("usage: %s [-%s] startaddr length (in bytes)\n",
		       argv[0],optstr);
		puts("\
-q  quit on error\n\
-m  mute operation\n\
-a  abbreviated test\n\
-p:  specify pass count in decimal\n\
-r:  specify read address separately\n");
		return(1);
	    }
	}
	if((argc - optind) != 2 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0 ||
	   getnum(argv[optind],16,(uint *)&MEMOP_LENGTH) == 0) goto usage;
    }

    if (memops_mmap(MEMOP_START, MEMOP_LENGTH,
                    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

//    tmemp->start = (utype_t *)MEMOP_START;
    tmemp->start = (utype_t *)viraddr;
    tmemp->length = MEMOP_LENGTH;
    if(tmemp->length < 8) {
	printf("length must be at least 8 bytes\n");
	if(argc == 1) return(1);
	else goto usage;
    }

    testpass = 1;
    switch(tmemp->passcount) {
    case 0:
	quitmsg();  /* run until user escapes */
	break;
    case 1:
	testpass = 0;
	break;
    }
    testname("memory");
    if(testmem(tmemp) < 0) return(2);
    prcomplete(testpass,errcount, 0);
    return(0);
}

int
memloop(int argc, char *argv[])
{
    ulong viraddr, pg_viraddr;
    ulong mapsz = FOUR_K; // Thought 4K should be enough for peek/poke
    register unsigned long *addr;
    register unsigned long value;
    utype_t rw = 'r';  /* init */
    
    printf("Note: Only physical address is accepted.\n");

    if(argc == 1) {
#ifdef LINUX_APP
        query_user_ex(QU_START | QU_R_WR | QU_VALUE | QU_OPSIZ,
		      &MEMOP_START, &rw, &MEMOP_VALUE,
		      (ulong *)&MEMOP_SIZE);
#else
	query_user(QU_START | QU_R_WR | QU_VALUE | QU_OPSIZ,
		   &MEMOP_START, &rw, &MEMOP_VALUE,
		   &MEMOP_SIZE);
#endif
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&MEMOP_START) < 0) {
usage:
	    printf("usage: %s [-%s] addr [writedata]\n",argv[0],optstr);
	    return(1);
	}
	switch(argc - optind) {
	case 0: break;  /* no more arguments */
	case 1:
	    if(getnum(argv[optind],16,(uint *)&MEMOP_VALUE) == 0) goto usage;
	    rw = 'w';
	    break;
	default:
	    goto usage;
	}
    }

    if (memops_mmap(MEMOP_START, mapsz,
                    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    quitmsg();
    addr =  (unsigned long *)viraddr;
    value = MEMOP_VALUE;
    switch(MEMOP_SIZE) {
    case 1:  /* byte operation */
	if(rw == 'w') {
	    while(1) *addr = (unsigned char)value;
	} else while(1) value = (unsigned char)*addr;
	break;
    case 2:  /* word operation */
	if(rw == 'w') {
	    while(1) *addr = (unsigned short)value;
	} else while(1) value = (unsigned short)*addr;
	break;
    case 4:  /* longword operation */
	if(rw == 'w') {
	    while(1) *addr = value;
	} else while(1) value = *addr;
	break;
    }
    return(0);
}

#ifdef CANCUN
/****** this is needed atleast by Brasil but may be a problem for volcano ****/
int
wrloop(int argc, char *argv[])
{
    register volatile unsigned int *addr;
    register unsigned long value, temp;
    unsigned int rw = 'r';  /* init */
    
    if(argc == 1) {
#ifdef LINUX_APP
        unsigned long val = 0;
	query_user_ex(QU_START | QU_VALUE | QU_OPSIZ,
		   &(NVRAM)->start, &(NVRAM)->value,
		   &(NVRAM)->memopsiz, &val);
#else
	query_user(QU_START | QU_VALUE | QU_OPSIZ,
		   &(NVRAM)->start, &(NVRAM)->value,
		   &(NVRAM)->memopsiz);
#endif
    } else {
	if(getopsize(argc,argv) < 0 ||
	   getaddr(argv[optind++],&(NVRAM)->start) < 0) {
usage:
	    printf("usage: %s [-%s] addr [writedata]\n",argv[0],optstr);
	    return(1);
	}

	switch(argc - optind) {
	case 0: break;  /* no more arguments */
	case 1:
	    if(getnum(argv[optind],16,(uint *)&(NVRAM)->value) == 0) goto usage;
	    rw = 'w';
	    break;
	default:
	    goto usage;
	}
    }
    quitmsg();
    addr = (unsigned int*)(NVRAM)->start;
    value = (NVRAM)->value;
    switch((NVRAM)->memopsiz) {
    case 1:  /* byte operation */
	while(1){
	    *(volatile unsigned char *)addr = value;	    
	    temp = *(volatile unsigned char *)addr;
	    temp = *(volatile unsigned char *)addr;
	}
	break;
    case 2:  /* word operation */
	while(1){
	    *(volatile unsigned short *)addr = value;	    
	    temp = *(volatile unsigned short *)addr;
	    temp = *(volatile unsigned short *)addr;
	}

	break;
    case 4:  /* longword operation */
	while(1){
	    *addr = value;	    
	    temp = *addr;
	    temp = *addr;
	}

	break;
    }
    return(0);
}

#endif


void
addrtest(unsigned long phyaddr, unsigned long length, unsigned long viraddr)
{
    register int offset = 1;
    register unsigned long x;
    register unsigned long end =  (phyaddr + length);

    register unsigned long *addr; 
    addr = (unsigned long *)viraddr;

    if (x) {
        /* 
         * Variable "x" is needed in this function
         * Latest compiler will deem this variable as unused so add comments here
         */
    }

    /* start with even addr (offset will make it odd) */
    if(phyaddr & 1) phyaddr--;
    while(1) {
      phyaddr += offset;  /* add in our offset */
      if(phyaddr >= end) break;  /* we exceed our bound */

      addr = addr + offset;
      x = *addr;             /* read the address (non-destructive) */

      if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%.6lx (va %.6lx) = ", phyaddr, (unsigned long)addr);
        printf("%#.8lx",*addr);
      }

      offset <<= 1;
    }
}

int
addrloop(int argc, char *argv[])
{
    ulong viraddr, pg_viraddr;

    printf("Note: Only physical address is accepted.\n");

    if(argc == 1) {
#ifdef LINUX_APP
        unsigned long val = 0;
	query_user_ex(QU_START | QU_SIZE,
		   &MEMOP_START, &MEMOP_LENGTH, &val, &val);
#else
	query_user(QU_START | QU_SIZE,
		   &MEMOP_START, &MEMOP_LENGTH);
#endif
    } else {
	if(getopsize(argc,argv) < 0 ||  /* done for initialization only? */
	   getaddr(argv[optind++],&MEMOP_START) < 0 ||
	   getnum(argv[optind++],16,(uint *)&MEMOP_LENGTH) == 0 ||
	   optind != argc) {
	    printf("usage: %s addr length\n",argv[0]);
	    return(1);
	}
    }

    if (memops_mmap(MEMOP_START, MEMOP_LENGTH,
                    &viraddr, &pg_viraddr) == FAIL) {
      printf("%s() mmap failed\n", __FUNCTION__);
      return(FAIL);
    }

    while(1) addrtest(MEMOP_START, MEMOP_LENGTH, viraddr);

    memops_munmap(pg_viraddr, MEMOP_LENGTH);

    return PASSED;
}

/*
** Scan the specified range of addresses, byte by byte, and report the
** addresses that cause bus errors.
** The variable prev_acc_stat (previous access status) is used to keep
** state as follows:
**    -1 = initial value to signify first time through loop
**     0 = previous access did not cause a bus error
**     1 = previous access caused a bus error
*/
int
berrscan(int argc, char *argv[])
{
    char prev_acc_stat = -1;  /* init */
    static char *berr_scan_msg = "bus error from location";
    register volatile unsigned char *addr, *end;
    unsigned char temp;
    char buffer[120];
    char *bptr;

    if (temp) {
        /* 
         * Variable "temp" is needed in this function
         * Latest compiler will deem this variable as unused so add comments here
         */
    }

    if(argc == 1) {
#ifdef LINUX_APP
        unsigned long val = 0;
        query_user_ex(QU_START | QU_SIZE ,
		   &MEMOP_START, &MEMOP_LENGTH, &val, &val);
#else
	query_user(QU_START | QU_SIZE ,
		   &MEMOP_START, &MEMOP_LENGTH);
#endif
    } else {
	if(optind ==0) optind++; /* beginning of command */
	if(getaddr(argv[optind++],&MEMOP_START) < 0 ||
	   getnum(argv[optind++],16,(uint *)&MEMOP_LENGTH) == 0 || optind != argc) {
	    printf("usage: %s start length (in bytes)\n",argv[0]);
	    return(1);
	}
    }
    addr = (unsigned char *)MEMOP_START;
    end = addr + MEMOP_LENGTH;
    printf("scanning from %#lx to %#lx with logging\n", (ulong)addr, (ulong)end - 1);
    hkeepflags &= ~H_BUSERR;  /* initialize flag */
#ifndef LINUX_APP
    savfcn = signal(SIGBUS,SIG_IGN);  /* do not report bus errors to console */
#endif
    bptr = buffer;
    for(; addr < end; addr++) {
	temp = *addr;  /* read the address */
        BUSERR_NOP_X4();
	if(hkeepflags & H_BUSERR) {  /* this access caused an error */
	    hkeepflags &= ~H_BUSERR;  /* clear flag */
	    if(prev_acc_stat != 1) {   /* previous access did not cause a bus error */
		if(prev_acc_stat == 0) {  /* not first time through loop */
		    sprintf(bptr, "%p\n", (void *)(addr-1));
		    logprintf(buffer);
		    bptr = buffer;
		}
		bptr += sprintf(bptr, "%s %p to ", berr_scan_msg, (void *)addr);
		prev_acc_stat = 1;  /* set for next access */
	    }
	} else {  /* this access did not cause an error */
	    if(prev_acc_stat != 0) {  /* previous access caused an error */
		if(prev_acc_stat > 0) {  /* not first time through loop */
		    bptr += sprintf(bptr, "%p\n", (void *)(addr-1));
		}
		bptr += sprintf(bptr, "no %s %p to ", berr_scan_msg, (void *)addr);
		prev_acc_stat = 0;  /* set for next access */
	    }
	}
    }
    sprintf(bptr, "%p\n", (void *)(addr-1));
    logprintf(buffer);
#ifndef LINUX_APP
    signal(SIGBUS,savfcn);
#endif
    return(0);
}

/*
** Test parity on the user specified range.
** Longword aligned main and shared memory addresses are valid.
*/
int
paritytest(int argc, char *argv[])
{
#ifdef LINUX_APP
    printf("paritytest: not support on linux diagnostics\n");
#else
    unsigned long location, end;

    if(argc == 1) {
#ifdef LINUX_APP
        unsigned long val = 0;
        query_user_ex(QU_START | QU_SIZE,
		   &MEMOP_START, &MEMOP_LENGTH, &val, &val);
#else
	query_user(QU_START | QU_SIZE,
		   &MEMOP_START, &MEMOP_LENGTH);
#endif
    } else {
	if(optind ==0) optind++; /* beginning of command */
	if(getaddr(argv[optind++],&MEMOP_START) < 0 ||
	   getaddr(argv[optind++],&MEMOP_LENGTH) < 0 || optind != argc) {
	    printf("usage: %s addr length\n",argv[0]);
	    return(1);
	}
    }

    location = MEMOP_START;
    end = location + MEMOP_LENGTH;
#ifdef ATLANTIS
    for(; location < end; location += 8) {
#else
    for(; location < end; location += 4) {
#endif
	switch(test_parity(location, 1)) {
	case 0: break;
	case 2:
	    if(memsize == 0 || shmemsize == 0)
		printf("size memory before running\n");
	default:
	    return(1);
	}
    }
#endif /* LINUX_APP */
    return(0);
}

void
quitmsg()
{
  printf("enter <break> to quit\n");
}

#ifndef LINUX_APP
int
memory_checksum(int argc, char *argv[])
{
    extern int optind;
    unsigned short chksum;
    uint addr, size, end;

    if(argc != 3) goto usage;

    getnum(argv[1],16,(uint *)&addr);
    getnum(argv[2],16,(uint *)&size);   
    end = addr + size;

    chksum = 0; /* init */
#ifdef DEBUG
    printf("\naddr = %#x, size = %#x\n",addr, size);
#endif
    for(;addr<end;addr++)
	chksum += *(unsigned char *)addr;
    printf("\nChecksum is %#x\n", chksum);
    return(0);
usage:
    printf("usage: %s <start address> <size in bytes> \n",argv[0]);
    return(1);
}

#endif /* LINUX_APP */


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function: test_mem_6n
 *
 *  Test 32-bit-wide memory with modulo 6 patterns.  First fill entire
 *  range with patterns.  Verify integrity with readback, replacing
 *  each cell with the complement of the previous contents.  Finally
 *  check the range for the complemented patterns.  In case of an
 *  error, format a message at err_msg.
 *
 *  Returns: 0 (PASSED) for no errors; 
 *           1 (FAILED) for HSSI nim test failure.
 */
int 
test_mem_6n (ulong *start, ulong num_long, char *err_msg)
{
    ulong *end = start + num_long;
    ulong *p, incr, val;

    p = start;
    incr = 0;
    while (p < end) {
	*p++ = num_pattern[incr % num_pats];
	incr++;
    }

    p = start;
    incr = 0;
    while (p < end) {
	val = *p;
	if (val != num_pattern[incr % num_pats]) {
	    sprintf(err_msg, "Readback failed at %p: "
		    "got %#.8lx; expected %#.8lx\n",
		    (void *)p, val, num_pattern[incr % num_pats]);
	    return(FAILED);
	}
	*p++ = ~val;
	incr++;
    }

    p = start;
    incr = 0;
    while (p < end) {
	val = *p;
	if (val != ~num_pattern[incr % num_pats]) {
	    sprintf(err_msg, "Readback failed at %p: "
		    "got %#.8lx; expected %#.8lx\n",
		    (void *)p, val, ~num_pattern[incr % num_pats]);
	    return(FAILED);
	}
	*p++ = ~val;
	incr++;
    }
    return PASSED;
}


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *  Function: test_masked_mem_6n
 *
 *  Test 32-bit-wide memory with modulo 6 patterns masked by the given
 *  argument.  First fill entire range with the patterns ANDed with
 *  the mask.  Verify integrity with readback, replacing each cell
 *  with the masked complement of the previous contents.  Finally
 *  check the range for the complemented patterns.  In case of an
 *  error, format a message at err_msg.
 *
 *  Returns: 0 (PASSED) for no errors; 
 *           1 (FAILED) for HSSI nim test failure.
 */
int 
test_masked_mem_6n (ulong *start, ulong num_long, ulong mem_mask, char *err_msg)
{
    ulong *end = start + num_long;
    ulong *p, incr, val, masked_pattern;

    p = start;
    incr = 0;
    while (p < end) {
	*p++ = num_pattern[incr % num_pats] & mem_mask;
	incr++;
    }

    p = start;
    incr = 0;
    while (p < end) {
	val = *p & mem_mask;
	masked_pattern = num_pattern[incr % num_pats] & mem_mask;
	if (val != masked_pattern) {
	    sprintf(err_msg, "Readback failed at %p: "
		    "got %#.8lx; expected %#.8lx\n",
		    (void *)p, val, masked_pattern);
	    return(FAILED);
	}
	*p++ = (~val) & mem_mask;
	incr++;
    }

    p = start;
    incr = 0;
    while (p < end) {
	val = *p & mem_mask;
	masked_pattern = (~num_pattern[incr % num_pats]) & mem_mask;
	if (val != masked_pattern) {
	    sprintf(err_msg, "Readback failed at %p: "
		    "got %#.8lx; expected %#.8lx\n",
		    (void *)p, val, masked_pattern);
	    return(FAILED);
	}
	*p++ = ~val;
	incr++;
    }
    return PASSED;
}

#ifndef LINUX_APP

void srand (ulong data)
{
    seed = data;
}

/*
 * random number generator
 * ported from IOS - produces a random number.
 */
ulong
rand (void)
{
#define   PIE   0x42b249C5
#define   EXP   0x1033c728

    ullong    temp;

    /*
     * This random generator is uniform, i.e. it generates numbers
     * that spread out evenly
     */
    temp = seed;
    temp *= PIE;
    seed = (ulong) temp ^ ((ulong) (temp >> 32));
    seed += EXP;
    return (seed);
}
#endif /* LINUX_APP */

/*
 * Copy a block of memory, handling overlap.
 * This is the routine that actually implements
 * (the portable versions of) bcopy, memcpy, and memmove.
 */
void *
memmove(void *dst0, const void *src0, register size_t length)
{
        register char *dst = dst0;
        register const char *src = src0;
        register size_t t;

        if (length == 0 || dst == src)          /* nothing to do */
                goto done;

        /*
         * Macros: loop-t-times; and loop-t-times, t>0
         */
#define TLOOP(s) if (t) TLOOP1(s)
#define TLOOP1(s) do { s; } while (--t)

        if ((unsigned long)dst < (unsigned long)src) {
                /*
                 * Copy forward.
                 */
                t = (type_t)src;   /* only need low bits */
                if ((t | (type_t)dst) & wmask) {
                        /*
                         * Try to align operands.  This cannot be done
                         * unless the low bits match.
                         */
                        if ((t ^ (type_t)dst) & wmask || length < wsize)
                                t = length;
                        else
                                t = wsize - (t & wmask);
                        length -= t;
                        TLOOP1(*dst++ = *src++);
                }
                /*
                 * Copy whole words, then mop up any trailing bytes.
                 */
                t = length / wsize;
                TLOOP(*(word *)dst = *(word *)src; src += wsize; dst += wsize);
                t = length & wmask;
                TLOOP(*dst++ = *src++);
        } else {
                /*
                 * Copy backwards.  Otherwise essentially the same.
                 * Alignment works as before, except that it takes
                 * (t&wmask) bytes to align, not wsize-(t&wmask).
                 */
                src += length;
                dst += length;
                t = (type_t)src;
                if ((t | (type_t)dst) & wmask) {
                        if ((t ^ (type_t)dst) & wmask || length <= wsize)
                                t = length;
                        else
                                t &= wmask;
                        length -= t;
                        TLOOP1(*--dst = *--src);
                }
                t = length / wsize;
                TLOOP(src -= wsize; dst -= wsize; *(word *)dst = *(word *)src);
                t = length & wmask;
                TLOOP(*--dst = *--src);
        }
done:
        return (dst0);
}


/******** History ******** 
$Log: memops.c,v $
Revision 1.11  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.10  2012/08/16 08:10:31  alpeng
add statement before using memory related CLI cmds

Revision 1.9  2012/08/15 14:26:15  alpeng
support CLI cmds memdebug, memloop and memtest

Revision 1.8  2012/08/14 09:46:38  alpeng
support CLI cmd addrloop

Revision 1.7  2012/07/21 00:53:31  ptong
Fix compile warning

Revision 1.6  2012/07/20 19:03:19  ptong
Fix compile warning

Revision 1.5  2012/06/06 15:00:17  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/05 09:33:45  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/05/11 23:29:38  ptong
Remove unwanted printf

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

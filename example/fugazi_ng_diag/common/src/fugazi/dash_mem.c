/* $Id: dash_mem.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/dash_mem.c,v $
 *------------------------------------------------------------------
 *
 * Filename: dash_mem.c
 
 * Description: code for reading/writing to fpga registers. 
 * Copyright (c) 2012-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include "proto.h"
#include "queryflags.h"
#include "uio_utils.h"

#define DASH_SIZE  0x50000 //0xC000
#define CPLD_SIZE  0x1000
#define PASS 0
#define FAIL 1

extern unsigned long dash_cpld;
int dash_alt_mem(int);
int dash_dis_mem(int);
int dash_fil_mem(int);

static char opsizstr[] = "bwlldddd";  /* must be in this order with 4 d's */

static
char *querystrings[] = {  /* strings to match the queryflag bits */
    "source address",
    "destination address",
    "start address",
    "test size or length in bytes",
    "read or write the location (r/w)",
    "pattern to be written",
    "number of passes",
    "operation size 'd'ouble-long, 'l'ong, 'w'ord, or 'b'yte",
    "incrementing pattern",
    "scope trigger address",
    "abbreviated test?",
};

static unsigned long MEMOP_START = 0;
static unsigned int MEMOP_LENGTH = 0;
static unsigned int MEMOP_SIZE = 4;
static unsigned int MEMOP_VALUE = 0;
static void *map = 0;

/*-------------------------------------------------------------------
 *
 * Function: dash_filbyte
 * Description: fil memory by byte
 * Input: addr -- pointer to destination addr; length -- number of bytes
 *        to write; val -- value to write 
 *
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
static void dash_filbyte(unsigned char *addr, int length, unsigned char val)
{
    register unsigned char *end = (unsigned char *)((unsigned long)addr + length);

    while(addr < end) *addr++ = val;
}

/*-------------------------------------------------------------------
 *
 * Function: dash_filword
 * Description: fil memory by word
 * Input: addr -- pointer to destination addr; length -- number of bytes
 *        to write; val -- value to write 
 *
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
static void dash_filword(unsigned short *addr, int length, unsigned short val)
{
    register unsigned short *end = (unsigned short *)((unsigned long)addr + length);
    while(addr < end) *addr++ = val;
}

/*-------------------------------------------------------------------
 *
 * Function: dash_fillword
 * Description: fil memory by word
 * Input: addr -- pointer to destination addr; length -- number of bytes
 *        to write; val -- value to write 
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
void dash_fillword(unsigned int *addr, int length, unsigned int val)
{
    register unsigned int *end = (unsigned int *)((unsigned long)addr + length);
    while(addr < end)
        *addr++ = val;
}

/*-------------------------------------------------------------------
 *
 * Function: query_user_ex
 * Description: query user for addr/length/value.
 * Input: queryflag; type of query
 *        tmp0, tmp1, tmp2, tmp3 -- pointer to answers given by user
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
static void query_user_ex(QUERYFLAG queryflag, unsigned int *tmp0, 
                          unsigned int *tmp1, unsigned int *tmp2, 
                          unsigned int *tmp3)
{
    unsigned int i;
    unsigned int temp, mask, wrpat, *temp_ptr;
    char rd_wr;
    char buffer[80];
    unsigned int *arg_list[4];
    int cnt;

    if (sizeof(arg_list)/sizeof(long) > 4) {
	printf("sizeof(arg_list) = %ld; sizeof(long) = %ld; num_of_arg= %ld\n",
	       sizeof(arg_list), sizeof(long), sizeof(arg_list)/sizeof(long));
	printf("file: %s function:%s -- incorrect number of arguments\n", 
	       __FILE__, __FUNCTION__);
        exit (0);
    } else {
        arg_list[0] = tmp0;
        arg_list[1] = tmp1;
        arg_list[2] = tmp2;
        arg_list[3] = tmp3;
    }
    
    wrpat = mask = rd_wr = 0;

    cnt = 0;
    for(i=0; i < (sizeof(QUERYFLAG)*8); i++) {
	if((queryflag >> i) & 1) {                      /* bit is set */
            temp_ptr = (unsigned int *)arg_list[cnt++];/* get next argument */
	    temp = *temp_ptr;
	    switch(i) {
	    case QU_VALUE_BIT:
		/*
                 ** If user wishes to read, there is no
                 ** need to query for write value.  Also
                 ** protect against operation size checking
                 ** on "wrpat" by turning off the QU_VALUE
                 ** bit.
                 */
		if(rd_wr == 'r') {
		    queryflag &= ~QU_VALUE;
		    break;
		}
		/* fall through */
	    case QU_SOURCE_BIT: case QU_DEST_BIT:
	    case QU_START_BIT: case QU_SIZE_BIT:
	    case QU_TRIGGER_BIT: case QU_PASSES_BIT:
		sprintf(buffer, "Enter in hex the %s", querystrings[i]);
		temp = gethex_answer(buffer, temp, 0, (unsigned int)0xFFFFFFFF);
		if( i == QU_VALUE_BIT )
                    wrpat = temp;
		break;
	    case QU_R_WR_BIT:

		sprintf(buffer, "Do you wish to %s?",
			querystrings[QU_R_WR_BIT]);
		rd_wr = temp = getc_answer(buffer, "rw", temp);
		break;
	    case QU_ABBREV_BIT:

		sprintf(buffer,querystrings[QU_ABBREV_BIT]);
		if( getc_answer(buffer,"yn",'n') == 'y' )
                    temp = 1;
		else
                    temp = 0;
		break;
	    case QU_OPSIZ_BIT:

                if (temp < 1)
                    temp = 'b';
                else
                    temp = opsizstr[temp-1];  /* convert to ascii char */
		if(!strchr(opsizstr, temp)) temp = 'b';  /* validate it */
      		sprintf(buffer, "Enter the %s",
			querystrings[QU_OPSIZ_BIT]);
		temp = getc_answer(buffer, opsizstr, temp);
		switch(temp) {  /* convert back to a number */
		case 'd': // double long (64 bit)
		    temp = 8;
		    mask = 0;
		    break;
		case 'l':
		    temp = 4;
		    mask = (unsigned int)0;
		    break;
		case 'w':
		    temp = 2;
		    mask = (unsigned int)0xffff0000;
		    break;
		case 'b':
		    temp = 1;
		    mask = (unsigned int)0xffffff00;
		    break;
		}
                //                printf("here %d\n", __LINE__);
		if((queryflag & QU_VALUE) && (wrpat & mask)) {
		    sprintf(buffer, "Value (0x%x) is larger than the op. "
			    "size - respecify? y/n", wrpat);
		    if(getc_answer(buffer, "yn", 'y') == 'y') {
			i--;  /* do this (QU_OPSIZ) over again */
                        //va_rollback(argptr, unsigned *);
                        cnt--;  
			continue;
		    }
		}
		break;
	    default:
                printf("unsuported bit\n");
		continue;  /* in case of unsupported bit */
	    }
	    *temp_ptr = temp;  /* set new value */

        }
    }

}

/*-------------------------------------------------------------------
 *
 * Function: dash_dismem
 * Description: display address
 * Input: addr -- src addr; length -- number of bytes to show
 *        disaddr -- addr to be shown in the output
 *        field size -- size of each data 
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
static void dash_dismem(unsigned char *addr, unsigned int length,
                        unsigned long disaddr, unsigned int fldsize)
{
    register int value = 0;
    register unsigned char i, j, c, linepos, asciistart = 0;
    register unsigned char *end = (addr + length);  /* the end boundary */
    register unsigned char *linend, *linestart;

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
                    value = *(unsigned int *)addr;  /* show 32 bit value */
                    addr += 4;
                    break;
                default:
                    printf("%s: invalid operation size %d\n", __FUNCTION__, fldsize);
                    return ;
                }

                if(i == 0) linepos += printf("%.*x ",fldsize * 2,value);  /* hex */
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

  
/*-------------------------------------------------------------------
 *
 * Function: dash_dis_mem
 * Description: alt memory for FPGA/CPLD
 * Input: argc - number of argument
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
int dash_dis_mem(int argc)
{
    ulong val = 0;

    if(argc == 1) {
        query_user_ex(QU_START | QU_SIZE | QU_OPSIZ,
                      (unsigned int *)&MEMOP_START,
                      (unsigned int *)&MEMOP_LENGTH,
                      (unsigned int *)&MEMOP_SIZE,
                      (unsigned int *)&val);
    } else {
        printf("Not supported %d\n", __LINE__);
        return(1);
    }


    printf("\n display  mem MEMOP_START=%#.8lx MEMOP_LENGTH=%#.8x MEMOP_SIZE=0x%d\n",
           MEMOP_START, MEMOP_LENGTH, MEMOP_SIZE);


    if (!(unsigned long)map) {
        assert("mmap of uio-fpga not called!!");
        exit(0);
    }

    dash_dismem((unsigned char *)(((unsigned long)map) + MEMOP_START),
                MEMOP_LENGTH,
                MEMOP_START,
                MEMOP_SIZE);

    return(PASS);
}

/*-------------------------------------------------------------------
 *
 * Function: dash_set_map
 * Description: set type of memory to be manipulated, either cpld or
 *                fpga
 * Input: argc - number of argument
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
int dash_set_map (int verbose)
{
    static int mode = 0;

    if (mode == 0) {
        mode = 1;
        map = (void *)uio_get_regs();
        if (verbose)
            printf("\nSwitch to FPGA address\n");

    } else {
        mode = 0;
        map = (void *)dash_cpld;
        if (verbose)
            printf("\nSwitch to CPLD address \n");

    }
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function: dash_alt_mem
 * Description: alt memory for FPGA/CPLD
 * Input: argc - number of argument
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
int dash_alt_mem (int argc)
{
    union location {
        unsigned char byte;
        unsigned short word;
        unsigned lword;
        ulong    dword;
    };
    register union location *addr;
    int opsiz, tmp;
    register char *c_ptr;
    char inbuf[24];
    unsigned int dispaddr;
    ulong val = 0;

    if(argc == 1) {
        query_user_ex(QU_START | QU_OPSIZ, (uint *)&MEMOP_START,
                      &MEMOP_SIZE, (uint *)&val, (uint *)&val);
    } else {
        printf("not suported \n");
        return(1);
    }

    dispaddr = (unsigned int)MEMOP_START;
    addr = (union location *)(map + dispaddr);
    opsiz = MEMOP_SIZE;
    while(1) {
        printf("%.6lx = ", (unsigned long)dispaddr);
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
        fgets((char *)inbuf, sizeof(inbuf), stdin);
        switch(*c_ptr) {
        case 'x':
        case 'q':
            /* The memory map must be release with exit
             */
            return(PASS);  /* quit */
        case ',':
        case 'p': /* prev location */
            addr = (union location *)((unsigned long)addr - opsiz);
            dispaddr = dispaddr - opsiz;
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
        addr = (union location *)((unsigned long)addr + opsiz);
        dispaddr = dispaddr + opsiz;
        /* bump address */
    }
}

/*-------------------------------------------------------------------
 *
 * Function: dash_fil_mem
 * Description: fil memory for FPGA/CPLD
 * Input: argc - number of argument
 *
 * Output: PASSED or FAILED
 *
 *-------------------------------------------------------------------
 */
int dash_fil_mem(int argc)
{
    if(argc == 1) {
        query_user_ex(QU_START | QU_SIZE | QU_VALUE | QU_OPSIZ ,
                      (unsigned int *)&MEMOP_START,
                      (unsigned int *)&MEMOP_LENGTH,
                      (unsigned int *)&MEMOP_VALUE,
                      (unsigned int *)&(MEMOP_SIZE));

    } else {
        printf("not supported\n");
        return(1);

    }
    ulong addr = MEMOP_START ;

    switch(MEMOP_SIZE) {
    case 1:
        dash_filbyte((unsigned char *)addr,MEMOP_LENGTH,MEMOP_VALUE);
        break;
    case 2:
        dash_filword((unsigned short *)addr,MEMOP_LENGTH,MEMOP_VALUE);
        break;
    case 4:
        dash_fillword((unsigned int *)(addr + (unsigned long)(map)),MEMOP_LENGTH,MEMOP_VALUE);
        break;
    default:
        printf("fil_mem: wrong mem operation size\n");
        break;
    }

    return(0);
}

/*-------------------------------------------------
 * $Log: dash_mem.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */

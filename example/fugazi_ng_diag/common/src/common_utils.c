/* $Id: common_utils.c,v 1.12 2020/08/19 09:49:17 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/common_utils.c,v $
 *------------------------------------------------------------------
 *
 * Filename: common_utils.c
 *
 * Copyright (c) 2017-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "pcmap.h"
#include "cpu.h"
#include "pci.h"
#include "nvsysvars.h"
#include "testmem.h"
#include "proto.h"
#include "extern.h"
#include "queryflags.h"
#include "pm_utils.h"

#ifdef LINUX_APP
#include <stdio.h>
#include <stdlib.h>
#include "linux_api.h"
#else
extern void puts(char *);
extern void sleep(int msecs);	/* cannot use proto.h, since it causes
				 * dis_mem() and fil_mem() to fail the
				 * compilation */
#endif /* LINUX_APP */

#undef DEBUG


#ifdef OVERLORD 
struct testdat march_patterns[] = {
    {(unsigned long)0x5a5aa5a55a5aa5a5,
     (unsigned long)0xa5a55a5aa5a55a5a, 1},
    {0xa5a55a5aa5a55a5a, 0x5a5aa5a55a5aa5a5, 1},
    {0x5a5aa5a55a5aa5a5, 0xa5a55a5aa5a55a5a, 0},
    {0xa5a55a5aa5a55a5a, 0x5a5aa5a55a5aa5a5, 0},
    {0x5a5aa5a55a5aa5a5, 0x3c3cc3c33c3cc3c3, 1},
    {0x3c3cc3c33c3cc3c3, 0xc3c33c3cc3c33c3c, 0},
    {0xc3c33c3cc3c33c3c, 0x3c3cc3c33c3cc3c3, 1},
    {0x3c3cc3c33c3cc3c3, 0xf0f0f0f0f0f0f0f0, 0},
    {0xf0f0f0f0f0f0f0f0, 0x0f0f0f0f0f0f0f0f, 1},
    {0x0f0f0f0f0f0f0f0f, 0xf0f0f0f0f0f0f0f0, 0},
    {0xf0f0f0f0f0f0f0f0, 0x00ff00ff00ff00ff, 1},
    {0x00ff00ff00ff00ff, 0xff00ff00ff00ff00, 0},
    {0xff00ff00ff00ff00, 0x00ff00ff00ff00ff, 1},
    {0x00ff00ff00ff00ff, 0x0000ffff0000ffff, 0},
    {0x0000ffff0000ffff, 0xffff0000ffff0000, 1},
    {0xffff0000ffff0000, 0x0000ffff0000ffff, 0},
    {0x0000ffff0000ffff, 0xffffffffffffffff, 1},
    {0xffffffffffffffff, 0x0000000000000000, 0},
    {0x0000000000000000, 0xffffffffffffffff, 1},
    {0xffffffffffffffff, 0x0000000000000000, 0},
};
#else //OVERLORD 

struct testdat march_patterns[] = {
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

#endif // LINUX_APP

int num_march_patrns = (sizeof(march_patterns)/sizeof(struct testdat));

/*
 * Static definition
 */
static int rvw_mem(struct testmem *tmemp, struct testdat *d_patterns);

int march_test(struct testmem *tmemp);
extern void msleep(int msecs);	/* cannot use proto.h, since it causes
				 * dis_mem() and fil_mem() to fail the
				 * compilation */
#ifdef DISPLAY_ENGINEERING_BANNER
void engineering_banner(void); /*
                                * Engineering banner to be displayed
                                */
#endif

/*
 * Global variables and extern
 */
ushort testphase;

extern int netflashbooted;
extern unsigned long memsize;          /* size of main memory */

extern int dis_mem(), fil_mem();

#ifdef DISPLAY_ENGINEERING_BANNER
/*********************************************************************
 *
 * Function: engineering_banner
 *
 * This function display the engineering banner
 *
 * Input:
 *
 * Output:
 **********************************************************************
 */
void
engineering_banner(void)
{
	char *banner = "\n"
        "              $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n"
        "              $                                                  $\n"
        "              $NOT an offical release image, for ENGINEERING ONLY$\n"
        "              $                                                  $\n"
        "              $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n";

        printf("%s", banner);
}
#endif

/**********************************************************************
 *
 * Function: register_read
 *
 * This function reads and returns the value from a specified address.
 *
 * Input :  register address, register size
 *
 * Output: register's value
 *
 **********************************************************************
 */
ulong 
register_read(ulong reg_addr, int reg_size)
{
    volatile uint *l_ptr;
    volatile ushort *s_ptr;
    volatile uchar *c_ptr;

    switch (reg_size) {
    case BW_32BITS_LE:
        l_ptr = (volatile uint *)reg_addr;
        return(*(uint32_le *)l_ptr);
        break;
    case BW_16BITS_LE:
        s_ptr = (volatile ushort *)reg_addr;
        return(*(uint16_le *)s_ptr);
        break;
    case BW_8BITS_LE:
        c_ptr = (volatile uchar *)reg_addr;
        return(*(uchar_le *)c_ptr);
        break;
    case BW_32BITS:
        l_ptr = (volatile uint *)reg_addr;
        return(*l_ptr);
        break;
    case BW_16BITS:
        s_ptr = (volatile ushort *)reg_addr;
        return(*s_ptr);
        break;
    case BW_8BITS:
    default:
        c_ptr = (volatile uchar *)reg_addr;
        return(*c_ptr);
        break;
    }
}

/**********************************************************************
 *
 * Function: register_write
 *
 * This function writes a value to a specified address.
 *
 * Input : register address, data value, register size
 *
 * Output: NONE
 *
 **********************************************************************
 */
void 
register_write(ulong reg_addr, ulong value, int reg_size)
{
    volatile uint *l_ptr;
    volatile ushort *s_ptr;
    volatile uchar *c_ptr;

    switch (reg_size) {
    case BW_32BITS_LE:
        l_ptr = (volatile uint *)reg_addr;
        *(uint32_le *)l_ptr = value;
        break;
    case BW_16BITS_LE:
        s_ptr = (volatile ushort *)reg_addr;
        *(uint16_le *)s_ptr = value;
        break;
    case BW_8BITS_LE:
        c_ptr = (volatile uchar *)reg_addr;
        *(uchar_le *)c_ptr = value;
        break;
    case BW_32BITS:
        l_ptr = (volatile uint *)reg_addr;
        *l_ptr = value;
        break;
    case BW_16BITS:
        s_ptr = (volatile ushort *)reg_addr;
        *s_ptr = value;
        break;
    case BW_8BITS:
    default:
        c_ptr = (volatile uchar *)reg_addr;
        *c_ptr = value;
        break;
    }
}

/**********************************************************************
 *
 * Function: new_register_read
 *
 * This function reads and returns the value from a specified address.
 *
 * Input :  reg_info_t * - points to the register test info struct.
 *	    reg_addr - points to the tested register.
 *	    buf * - read data buffer pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static uint
new_register_read(reg_info_t *reg_ptr, ulong reg_addr, volatile ulong *buf)
{
    if (reg_ptr->type & REG_ACCESS) {
	/* User provided read function */
	return((*reg_ptr->size.ext->rd_ptr)(reg_addr, reg_ptr->size.ext->size,
					    (ulong *)buf, reg_ptr->size.ext->param));
    } else {
	/* Direct memory read */
	*buf = register_read(reg_addr, reg_ptr->size.size);
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function: new_register_write
 *
 * This function writes a value to a specified address.
 *
 * Input : reg_info_t * - points to the register test info struct.
 *	   reg_addr - points to the tested register.
 *	   value - data to be written.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static uint
new_register_write(reg_info_t *reg_ptr, ulong reg_addr, ulong value)
{
    if (reg_ptr->type & REG_ACCESS) {
	/* User provided write function */
	return((*reg_ptr->size.ext->wr_ptr)(reg_addr, reg_ptr->size.ext->size,
					    value, reg_ptr->size.ext->param));
    } else {
	/* Direct memory write */
	register_write(reg_addr, value, reg_ptr->size.size);
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function: register_display
 *
 * For each register from reg_ptr, this function just displays the 
 * current value
 *
 * Input : Address of the first register, info for all registers
 *
 * Output: Always return PASSED
 *
 **********************************************************************
 */
int
register_display(ulong base_addr, reg_info_t *reg_ptr)
{
    ulong readval;
    ulong reg_addr;
    int rc;

    while (reg_ptr->size.size != 0) {
	reg_addr = base_addr + reg_ptr->offset;
	/*
	 * display current value
	 */
	printf("\n %25s",reg_ptr->name);
	rc = new_register_read(reg_ptr, reg_addr, &readval);
	if (rc != PASSED) {
	    cterr ('f',0,"Register display failed when reading %s "
		   "Register at %#x.",  reg_ptr->name, reg_addr);
	    return(FAIL);
	}
	printf(" , %#010x = %#010x, (mask:%#010x, size:%u)",
		(unsigned int)reg_addr, (unsigned int)readval, reg_ptr->mask, reg_ptr->size.ext->size);

	reg_ptr++;
    }
    return(PASS);

}

/**********************************************************************
 *
 * Function: register_alter
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and displays the current value, and allows a modification if possible.
 *
 * Input : Address of the first register, info for all registers
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int
register_alter(ulong base_addr, reg_info_t *reg_ptr)
{
    int tmp;
    ulong readval;
    ulong reg_addr;
    char buffer[48], *c_ptr;

    /* calling routines should put in the prpass() */
    printf("\n in register_alter");

    while (reg_ptr->size.size != 0) {
        reg_addr = base_addr + reg_ptr->offset;

	/*
	 * display current value
	 */
	printf("\n %25s",reg_ptr->name);
	new_register_read(reg_ptr, reg_addr, &readval);

	printf(" , %#.8x = %#.8x, (mask:%#.8x, size:%ld)",
		(unsigned int)reg_addr, (unsigned int)readval, reg_ptr->mask, reg_ptr->size.size);

	c_ptr = buffer;
	get_line(c_ptr,sizeof(buffer));
	switch(*c_ptr) {
	case 'x':
	case 'q':
	    return(0);	/* quit */
	case ',':
	case 'p':	/* prev location */
	    reg_ptr--;
	    /* fall through */
	case 'r':
	case '.':	/* same location */
	    continue;
	case 0:
	    break;	/* next location */
	default:
	    tmp = getnum(c_ptr, 16, (uint*)&readval);
	    if(tmp == 0) {
		printf("bad value \"%s\"\n",c_ptr);
		continue;	/* same location again */
	    } else {
		if (reg_ptr->type & READ_ONLY) {
		    printf(" READ ONLY");
		    continue;	/* same location again */
		} else {
		    register_write(reg_addr, readval, reg_ptr->size.size);
		    reg_ptr--;	/* redisplay this value, to verify the write */
		    c_ptr += tmp;
		    if (*c_ptr == '.')
			continue;	/* same location */
		}
		break;	/* next location */
	    }
        }

	reg_ptr++;
    }
    return(PASS);
}


/**********************************************************************
 *
 * Function: register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : Address of the first register, info for all registers
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int 
register_tests(ulong base_addr, reg_info_t *reg_ptr)
{
    unsigned int  i, rc;
    ulong reg_addr, original_data, readval, temp;
    uint data, size;

    /* calling routines should put in the prpass() */

    while (reg_ptr->size.size != 0) {
	if (reg_ptr->type & REG_ACCESS) {
	    /* Caller provided access functions */
	    reg_addr = base_addr + reg_ptr->offset;
	} else {
	    /* Direct memory access register */
	    reg_addr = base_addr + reg_ptr->offset;
	    /*
	     * Verify that a register is accessible.
	     */
#ifndef LINUX_APP
	    if (chkberr((volatile uint *)reg_addr, reg_ptr->size.size, TRUE)) {
		cterr ('f', 0, "Bus Error occurs when accessing "
			       "%s Register at %#x.",  reg_ptr->name, reg_addr);
		return(FAIL);
	    }
#endif /* LINUX_APP */
        }

        /*
         * Test a register if it's a R/W register
         */
        if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
	    if (reg_ptr->type & SAVE_RESTORE) {
		/* Save and restore the original data */
		rc = new_register_read(reg_ptr, reg_addr, &original_data);
		if (rc != PASSED) {
		    cterr('f', 0, "%s Register first read failed at offset %#x",
				   reg_ptr->name, reg_ptr->offset);
		    return(FAIL);
		}
		/* If the mask bits is set (read/writeable), the unmasked bits
		 * (read only/write only, or reserved) must be preserved.
		 */
		/* Some devices have read only bit that may change from time
		 * to time. These read only bits cannot be trusted and tested.
		 * (CSCso39166)
		 */
	    }
	    if (reg_ptr->type & REG_ACCESS) {
		/* Caller provided read/write */
		size = reg_ptr->size.ext->size;
	    } else {
		/* Direct memory access */
		if (reg_ptr->size.size > BW_56BITS) {
		    size = (reg_ptr->size.size >> 8);
		} else {
		    size = reg_ptr->size.size;
		}
	    }

            /* 
             * ripple 1 test
             */
            for (i = 0; i < (size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp)
                    continue;

		rc = new_register_write(reg_ptr, reg_addr, temp);
		if (rc != PASSED) {
		    cterr('f', 0, "Ripple one test failed when writing %s "
				   "register at offset %#x with %#x",
				   reg_ptr->name, reg_ptr->offset, temp);
		    return(FAIL);
		}

                rc = new_register_read(reg_ptr, reg_addr, &readval);
		if (rc != PASSED) {
		    cterr('f', 0, "Ripple one test failed when reading %s "
				  "register at offset %#x with %#x",
				   reg_ptr->name, reg_ptr->offset, temp);
		    return(FAIL);
		}

		if ((readval&reg_ptr->mask) != temp) {
		    cterr ('f',0,"Ripple one test failed when accessing %s "
				 "Register at %#x. Expect: %#x, Read: %#x.",
				 reg_ptr->name, reg_addr, temp, readval);
#ifdef REG_TEST_DEBUG
		    temp = 0xBB;
		    break;
#endif
		    return(FAIL);
                }
            }

#ifdef REG_TEST_DEBUG
	    if (temp == 0xBB) {
		reg_ptr++;
		continue;
	    }
#endif

            /* 
             * ripple 0 test
             */
            for (i = 0; i < (size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp)
                    continue;
                temp = (~(1 << i)) & reg_ptr->mask;

                rc = new_register_write(reg_ptr, reg_addr, temp);
		if (rc != PASSED) {
		    cterr ('f',0,"Ripple zero test failed when writing %s"
				 "Register at %#x.",  reg_ptr->name, reg_addr);
		    return(FAIL);
		}

		rc = new_register_read(reg_ptr, reg_addr, &readval);
		if (rc != PASSED) {
		    cterr ('f',0,"Ripple zero test failed when reading %s"
				 "Register at %#x.",  reg_ptr->name, reg_addr);
		    return(FAIL);
		}

		if ((readval&reg_ptr->mask) != temp) {
		    cterr ('f',0,"Ripple zero test failed when accessing "
				 "%s Register at %#x. Expect: %#x, Read: "
				 "%#x.", reg_ptr->name, reg_addr, temp,
				 readval);
		    return(FAIL);
                }
            }

	    /*
	     * pattern test
	     */
	    data = PATTERN;
	    for (i=0;i<2;i++){
		temp = data &reg_ptr->mask;

		if (!temp) {
		    continue;
		}

		rc = new_register_write(reg_ptr, reg_addr, temp);
		if (rc != PASSED) {
		    cterr ('f',0,"Pattern test failed when writing %s "
				 "Register at %#x.", reg_ptr->name, reg_addr);
		    return(FAIL);
		}

		rc = new_register_read(reg_ptr, reg_addr, &readval);
		if (rc != PASSED) {
		    cterr ('f',0,"Pattern test failed when reading %s "
				 "Register at %#x.", reg_ptr->name, reg_addr);
		    return(FAIL);
		}

		if ((readval&reg_ptr->mask) != temp) {
		    cterr ('f',0,"Pattern test failed when accessing %s "
				 "Register at %#x. Expect: %#x, Read: %#x.",
				 reg_ptr->name, reg_addr, temp, readval);
		    return(FAIL);
		}

		data = ~PATTERN; /* complement data pattern */
	    }
	    
	    /*
	     * restore reset value
	     */
	    if (reg_ptr->type & SAVE_RESTORE) {
		/* Restore the save value */
		rc = new_register_write(reg_ptr, reg_addr, original_data);
	    } else {
		/* Restore reset value */
		rc = new_register_write(reg_ptr, reg_addr, (ulong)reg_ptr->reset_val);
	    }

	    if (rc != PASSED) {
		cterr('f', 0, "Write failed when %s %s Register at %#x",
			      (reg_ptr->type & SAVE_RESTORE) ? "restoring" :
			      "resetting", reg_ptr->name, reg_addr);
		return(FAIL);
	    }
	}
	reg_ptr++;
    }
    return(PASS);
}


/**********************************************************************
 *
 * Function: register_def_tests
 *
 * For each register from reg_ptr, this function checks for default 
 * value.
 *
 * Input : Address of the first register, info for all registers
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int register_def_tests(ulong base_addr, reg_info_t *reg_ptr)
{
    unsigned int  rc;
    ulong reg_addr, readval;

    /* calling routines should put in the prpass() */

    while (reg_ptr->size.size != 0) {
	if (reg_ptr->type & REG_ACCESS) {
	    /* Caller provided access functions */
	    reg_addr = base_addr + reg_ptr->offset;
	} else {
	    /* Direct memory access register */
	    reg_addr = base_addr + reg_ptr->offset;
	    /*
	     * Verify that a register is accessible.
	     */
#ifndef LINUX_APP
	    if (chkberr((volatile uint *)reg_addr, reg_ptr->size.size, TRUE)) {
		cterr ('f', 0, "Bus Error occurs when accessing "
			       "%s Register at %#x.",  reg_ptr->name, reg_addr);
		return(FAIL);
	    }
#endif /* LINUX_APP */
        }

        /*
         * Read the register
         */
		rc = new_register_read(reg_ptr, reg_addr, &readval);
		if (rc != PASSED) {
		    cterr('f', 0, "%s Register read failed at offset %#x",
				   reg_ptr->name, reg_ptr->offset);
		    return(FAIL);
		}
		
		if ((readval & reg_ptr->mask) != reg_ptr->reset_val) {
			cterr('f', 0, "%s Register read from %#x : %#x, but it should be %#x mask %#x",
				   reg_ptr->name, reg_ptr->offset, readval, reg_ptr->reset_val, reg_ptr->mask);
		    return(FAIL);
		}
	reg_ptr++;
    }
    return(PASS);
}

/**********************************************************************
 *
 * Function: size_mem()
 *
 * Description:
 *    This function just display main memory size
 *
 * Input: None
 * 
 * Output: always PASSED (using void will get compiler warning since 
 *                        this function is called from diag.c)
 *
 **********************************************************************
 */
int
size_mem (void)
{
#ifndef LINUX_APP
    printf("\nmain memory size = %d MB\n", memsize/ONE_MEG);
#else
    printf("\nFIX ME:size_me \n");
    exit(0);
#endif
    return(0);
}

/**********************************************************************
 *
 * Function: mem_march_test()
 *
 * This function tests a block of memory from start_addr to end_addr
 * using the memory march test.
 * It can run memory test using L1, L2, or L3 cache to speed up
 * the testing process. If cache is used,  the testing code is moved 
 * to KSEG0.
 *
 * Input: test_type  = SHORT_UNCACHE to run a quick march test uncached
 *                     SHORT_CACHE   to run a quick march test with cache
 *                     LONG_UNCACHE  to run extensive march test uncached
 *                     LONG_CACHE    to run extensive march test with cache
 *        start_addr = memory location to start testing (Virtual Address)
 *        end_addr   = memory location where the test ends (Virtual Address)
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_march_test(MEM_CACH_TYPE cache_type, ulong start_addr, ulong end_addr)
{
#ifndef LINUX_APP
    ulong saved_config;
#endif
    int (*march_ptr)(struct testmem *);
    struct testmem tmem;
    register struct testmem *tmemptr = &tmem;
    int mem_status = PASSED; 
    boolean with_cache, short_test;

    if (with_cache) {
        /*
         * with_cache variable will be used in this function
         * Add dummpy comments here to fix latest compiler's error 
         */
    }

    switch (cache_type) {
       case SHORT_CACHE:
	   prpass(testpass, "Cached quick march test");
	   with_cache = TRUE;
	   short_test = TRUE;
	   break;

       case LONG_CACHE:
	   prpass(testpass, "Cached extensive march test");
	   with_cache = TRUE;
	   short_test = FALSE;
	   break;

       case SHORT_UNCACHE:
	   prpass(testpass, "Uncached quick march test");
	   with_cache = FALSE;
	   short_test = TRUE;
	   break;

       default: /* Long uncache */
	   prpass(testpass, "Uncached extensive march test");
	   with_cache = FALSE;
	   short_test = FALSE;
    }
#ifndef LINUX_APP
    if (with_cache) {
	/* 
	 * Enable all available layers of cache before testing
	 */
	if (enable_mem_cache(&saved_config))
            return(FAIL);

        /* 
         * Move the beginning of free memory and the end 
         * of RAM to cache section for faster testing.
         */
//        get_memory_location(&start_addr, &end_addr);
    }
#endif
    
    tmemptr->start = (utype_t *) start_addr;
    tmemptr->length  = end_addr - (unsigned long)start_addr;
    tmemptr->passcount = 1;  /* do complete mem test once */
    tmemptr->flag = INDIAGS | DRAM;

    /*
     * Run a long or short march_test() depend on flag
     */
    if (short_test)
        tmemptr->flag |= ABBR_TEST;
   
    /*
     * Run memory march test
     */
    march_ptr = march_test;

#ifdef LINUX_APP
    mem_status = (*march_ptr)(tmemptr);
#else
    /* Load the actual source code into cache if flag is on */
    if (with_cache) {
        march_ptr = (PFI) PHY_TO_CACHE(march_ptr);
    }

    mem_status = (*march_ptr)(tmemptr);

    if (with_cache) {
        /*
         * Disable cache layers that have been enabled above
         */
        restore_mem_cache(saved_config);
    }
#endif //LINUX_APP

    return(mem_status);

    //    printf("FIX ME: %s %s\n", __FILE__, __FUNCTION__);
}

/**********************************************************************
 *
 * Function: test_mainmem()
 *
 * This function is called from the main menu.
 * It tests the main memory using L1, L2, or L3 cache to speed up
 * the testing process. The code used to test it is moved to KSEG0.
 * It performs the following tests on main memory:
 *
 * - The main memory march test
 * - The platform specific memory test
 * 
 * Input: test_type  = SHORT_UNCACHE to run a quick march test uncached
 *                     SHORT_CACHE   to run a quick march test with cache
 *                     LONG_UNCACHE  to run extensive march test uncached
 *                     LONG_CACHE    to run extensive march test with cache
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
test_mainmem(int test_type)
{
    ulong block_start, block_end;
    int mem_status = PASSED;
#ifdef LINUX_APP
    block_start = block_end = 0;
    printf("FIX me : test_mainmem\n\n");
    return FAILED;
#endif
    /*
     * Note: This function is brought in from the le-diag tree to
     * support Shinkansen platform only. The xformers have created
     * their new functions.
     */
    testname("main memory");

    if (! memsize ) {
       cterr('f',0,"Memory has not been sized");
       return(FAILED);
    }
    else
	size_mem();

    /* 
     * Only for shinkanshin.
     * Hack: Need to add 64M so that memory test does not delete memory
     * created by devices using malloc_dev.
     */
    block_start = (getfreememstart() + SIXTYFOUR_MEG);
    block_end   = PHY_TO_KSEG1(PHY_ADRSPC_RAM + memsize);

    /*
     * to do a march test
     */
    if (mem_march_test(test_type, block_start, block_end))
      mem_status = FAILED;

    prcomplete(testpass, errcount, 0);

    return(mem_status);
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
#ifdef LINUX_APP
    ulong phy_addr;
#endif

    testname("addr equal data pattern");

    /*
     * Data = Address test pattern.
     */
    for (addrptr = (ulong *)start; addrptr < (ulong *)end; addrptr++) {
	if (((ulong)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "write addr %#lx,", (ulong)addrptr);
        *addrptr = (ulong)addrptr;  /* write : data = address */
    }

    for (addrptr = (ulong *)start; addrptr < (ulong *)end; addrptr++) {
	if (((ulong)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "verify addr %#lx,", (ulong)addrptr);
	value = *addrptr;
        if (value != (ulong)addrptr) {  /* read : data = address */

#ifdef LINUX_APP
	    if (addr_vtop((ulong)addrptr, &phy_addr) == FAIL) {
	        printf("%s %s() failed\n",__FILE__,__FUNCTION__);
	    }
            cterr('f',0,"failure at phy-addr: %#lx (vir-addr %#.lx) expected: %#lx "
                  "actual: %#lx.", phy_addr, addrptr, addrptr, value);
#else
            cterr('f',0,"failure at addr: %#lx expected: %#lx "
                  "actual: %#lx.", addrptr, addrptr, value);
#endif
	    return(FAILED);
        }
    }
    return(PASSED);
}


/**********************************************************************
 *
 * Function: mem_checkerboard_test()
 *
 * This function is used to test the main memory with checkerboard
 * or inverse checkerboard pattern.
 *
 * Input: starting and ending address of the memory block under test.
 *        test_sel: CHECKER_BOARD or INVERSE_CHECKER
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_checkerboard_test (ulong start, ulong end, ulong test_sel)
{
    long *addrptr, value;
    long pat;
    char *pattern;

    if (test_sel == CHECKER_BOARD) {
	pattern = "checkerboard";
	pat = 0x5a5a5a5a;
    } else if (test_sel == INVERSE_CHECKER) {
	pattern = "inverse checkerboard";
	pat = 0xa5a5a5a5;
    }
    else { /* User specific pattern */
	pattern = "user-specific pattern";
	pat = gethex_answer("\nEnter test pattern:", 0, 0, 0xffffffff);
    }
    testname("%s", pattern);

    for (addrptr = (long *)start; addrptr < (long *)end; addrptr++) {
	if (((long)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "write %#.8x to %#.8x,", 
		   pat, (long)addrptr);
	*addrptr = pat;
    }
 
    for (addrptr = (long *)start; addrptr < (long *)end; addrptr++) {
	if (((long)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "verify addr %#.8x,", (long)addrptr);
	value = *addrptr;
	if (value != pat) {
	    cterr('f',0,"failure at addr: %#.8x expected: %#.8x "
		  "actual: %#.8x.", addrptr, pat, value);
	    return(FAILED);
	}
    }
    return(PASSED);
}


/**********************************************************************
 *
 * Function: mem_walking_test()
 *
 * This function is used to test the main memory with walking 1's or
 * walking 0's test pattern.
 *
 * Input: starting and ending address of the memory block under test.
 *        pat_sel: WALKING_1 or WALKING_0
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_walking_test (ulong start, ulong end, ulong pat_sel) 
{
    volatile ulong *wrptr, *wrend;
    ulong pat;

    if (pat_sel)
	testname("walking 1's");
    else
	testname("walking 0's");

    wrptr = (volatile ulong *)start;
    wrend = (volatile ulong *)end;
    pat   = (pat_sel == 1) ? 1 : 0xfffffffe;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n start %#.8lx end %#.8lx\nwrptr %p wrend %p pat %#lx\n", 
	           start, end, (void *)wrptr, (void *)wrend, pat);
    }

    /* Writing phase */
    while (wrptr < wrend) {
	if (((ulong) wrptr % TWO_MEG) == 0)
	    prpass(testpass, "write %#.8x to %#.8x,", pat, wrptr);
	*wrptr++ = pat;
        pat = ROTATE_LEFT(pat);
    }

    /* Verifying phase */
    if (verify_walking_data(start, end, pat_sel))
	return(FAILED);

    return(PASSED);
}


/**********************************************************************
 *
 * Function: mem_random_num_test()
 *
 * This function is used to test the main memory with random number
 * pattern.
 *
 * Input: starting and ending address of the memory block under test.
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_random_num_test (ulong start, ulong end, ulong dummy)
{
    long  *addrptr, *rand_addr;
    unsigned long saved_val;
    ulong pat;

    testname("pseudo random");

    /*
     * random number test pattern
     */
    for (addrptr = (long *)start; addrptr < (long *)end; addrptr++) {
	pat = rand() & 0x0fffffff;

	/* make it long align */
	rand_addr = (long *)(((long)addrptr + (long)pat) & 0xfffffffc);

        if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("\n rand_addr %p rand %#.8lx", (void *)rand_addr, pat);
        }

	if (rand_addr < (long *)start)
	    rand_addr = (long *)start;
	if (rand_addr >= (long *)end)
	    rand_addr = (long *)((long)end - 4);

	pat = rand();
        *rand_addr = pat;

        if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("\n addr %p pat %#.8lx", (void *)addrptr, pat);
        }

	if (((long)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "write addr %#.8x data %#.8x,", 
		   (long)rand_addr, pat);

	saved_val = *rand_addr;
	if (saved_val != pat) {
            cterr('f',0,"failure at addr: %#.8x expected: %#.8x "
		  "actual: %#.8x.", rand_addr, pat, saved_val);
	    return(FAILED);
	}
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function: verify_walking_data()
 *
 * This function is used to verify the walking 1's or walking 0's
 * pattern written by the walking data test above.
 *
 * Input: starting and ending address of the memory block under test.
 *        pat_sel: WALKING_0 or WALKING_1
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
verify_walking_data (ulong start, ulong end, ulong pat_sel)
{
    ulong value, pat;
    volatile ulong *rdptr, *rdend;
 
    rdptr = (volatile ulong *)start;
    rdend = (volatile ulong *)end;
    pat = (pat_sel == 1) ? 1 : 0xfffffffe;

    while (rdptr < rdend) {
	if (((ulong)rdptr % FOUR_MEG) == 0)
	    prpass(testpass, "verify addr %#.8x,", (ulong)rdptr);

	if ((value = *rdptr++) != pat) {  /* check location for pattern */
	    cterr('f',0, "addr %#.8x expected %#.8x actual %#.8x", 
		  rdptr - 1, pat, value);
	    return(FAILED);
	}
	pat = ROTATE_LEFT(pat);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: memory_write()
 *
 * This function just write to memory locations without displaying onto
 * the console. The purpose is to try to capture the memory parity
 * error caused by memory read
 *
 * Input: starting and ending address of the memory block under test.
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
void
memory_write (ulong start, ulong end, ulong pattern)
{
    long  *addrptr;
 
    testname("main memory write");

    for (addrptr = (long *)start; addrptr < (long *)end; addrptr++) {
	if (((long)addrptr % TWO_MEG) == 0)
	    prpass(testpass, "write addr %#.8x with %#.8x,", 
		   (long)addrptr, pattern);
	*addrptr = pattern;
    }
}

/**********************************************************************
 *
 * Function: memory_read()
 *
 * This function just read from memory locations without displaying onto
 * the console. The purpose is to try to capture the memory parity
 * error caused by memory read
 *
 * Input: starting and ending address of the memory block under test.
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
void
memory_read (ulong start, ulong end, ulong tmp)
{
    ulong dummy; 
    long  *addrptr;
 
    testname("main memory read");
    if (dummy) {
       /*
        * dummy will be used for read data out. 
        * Add dummy comments here to fix latest compiler's error
        */
    }

    for (addrptr = (long *)start; addrptr < (long *)end; addrptr++) {
        if (((long)addrptr % TWO_MEG) == 0) {
            prpass(testpass, "read addr %#.8x,", (long)addrptr);
        }
        dummy = *addrptr;
    }
}

/**********************************************************************
 *
 * Function: mem_grp_test()
 *
 * This function performs a group of individual memory tests.
 *
 * Input: starting and ending address of the memory block under test.
 *
 * Output PASSED or FAILED
 *
 **********************************************************************
 */
int
mem_grp_test (ulong start, ulong end, ulong dummy)
{
    ulong pattern = 0x0f0f0f0f;
    int   ret = PASSED;

    testname("main memory group");

    ret  = mem_walking_test(start, end, (ulong)WALKING_0);
    ret |= mem_walking_test(start, end, (ulong)WALKING_1);
    ret |= mem_random_num_test(start, end, dummy);
    ret |= mem_addr_eq_data_test(start, end, start);
    ret |= mem_checkerboard_test(start, end, (ulong)CHECKER_BOARD);
    ret |= mem_checkerboard_test(start, end, (ulong)INVERSE_CHECKER);
    ret |= mem_access_test(start, dummy, dummy);
    memory_write(start, end, pattern);
    memory_read(start, end, dummy);

    testname("main memory group");
    return(ret);
}


/**********************************************************************
 *
 * Function: mem_util()
 *
 * Description:
 *    This function is a memory test utility. It allows user to select
 * individual test function. Except for test 9 and b (cached main memory
 * test) all tests will run uncached to verify the functionality of
 * the main SDRAM memory itself, and not the cache.
 *
 * Input: None
 *
 * Output PASSED or FAILED
 **********************************************************************
 */
int
mem_util (void)
{
#ifndef LINUX_APP
    ulong block_start, block_end, saved_config;
    ulong dummy = 0, pattern = 0x0f0f0f0f;
    int (*func_ptr)(ulong, ulong, ulong) = NULL;
    int mem_status = PASSED;

    /* 
     * some cache routines are in cache already, so we need this
     * variable to make sure the func_ptr will not be casted into
     * cache again
     */
    int cache_only = 0;

    /*
     * Declare testnum to be static to allow the test to continuously 
     * run without asking user for the test number again.
     */
    static int testnum, flag = 0;
    static int cache;

    if (!flag) {
	if (getc_answer("Running with cache?", "yn", 'n') == 'y')
	    cache = 1;
	else 
	    cache = 0;
    }

    if ((NVRAM)->diagflag & D_CONTINUOUS)
	flag = 1;

    if (testpass <= 1) {
	printf("\n0 = walking 0's,");
	printf("\n1 = walking 1's,");
	printf("\n2 = pseudo random,");
	printf("\n3 = data equal address,");
	printf("\n4 = checkerboard,");
	printf("\n5 = inverse checkerboard,");
	printf("\n6 = user-specific pattern");
	printf("\n7 = memory write only");
	printf("\n8 = memory read only");
	printf("\n9 = 8, 16, 32 and 64-bit memory access test");
	printf("\na = extensive cached main memory test");
	printf("\nb = extensive uncached main memory test");
	printf("\nc = quick cached main memory test");
	printf("\nd = quick uncached main memory test");
	printf("\ne = platform-specific memory test\n");
	testnum = gethex_answer("\nEnter test number:", 0, 0, 0xe);
    }

    /*
     * Run a simple memory test from begining
     * of free memory to the end of RAM.
     */
    block_start = getfreememstart();
    block_end   = PHY_ADRSPC_RAM + memsize;

    if (cache) {
	enable_mem_cache(&saved_config);
	block_start = PHY_TO_CACHE(block_start);
	block_end   = PHY_TO_CACHE(block_end);
    }

    /* 
     * The block_end address is different for ROM and RAM version.
     * Adjust it accordingly.
     */
    if (netflashbooted) {
	if (cache)
	    block_end = (PHY_TO_CACHE(block_end));
	else
	    block_end = (PHY_TO_KSEG1(block_end));
    }

    /* 
     * test_mainmem() already print the same. Skip here to avoid redundancy
     */
    if (testnum < LG_CACHE_MEM) {
	size_mem();
	printf("block_start %#.8x block_end %#.8x\n", block_start, block_end);
    }

    switch (testnum) {
    case WALKING_0:
    case WALKING_1:
	if (cache)
	    func_ptr = mem_walking_test;
	else
	    mem_walking_test(block_start, block_end, testnum);
	break;

    case PSEUDO_RANDOM:
	if (cache)
	    func_ptr = mem_random_num_test;
	else
	    mem_random_num_test(block_start, block_end, dummy);
	break;

    case ADDR_EQ_DATA:
	if (cache)
	    func_ptr = mem_addr_eq_data_test;
	else
	    mem_addr_eq_data_test(block_start, block_end, block_start);
	break;

    case CHECKER_BOARD:
    case INVERSE_CHECKER:
    case USER_PATTERN: /* Test using user specific pattern */
	if (cache)
	    func_ptr = mem_checkerboard_test;
	else
	    mem_checkerboard_test(block_start, block_end, testnum);
	break;

    case MEMORY_WRITE:
	if (cache) {
	    testnum = pattern;
	    func_ptr = (PFI)memory_write;
	    testnum = 7;
	} else
	    memory_write(block_start, block_end, pattern);
	break;

    case MEMORY_READ:
	if (cache)
	    func_ptr = (PFI)memory_read;
	else
	    memory_read(block_start, block_end, dummy);
	break;

    case MEM_ACCESS_TEST:
	if (cache)
	    func_ptr = mem_access_test;
	else
	    mem_access_test(block_start, dummy, dummy);
	break;
    
    case LG_CACHE_MEM:
	cache_only = 1;
	test_mainmem(LONG_CACHE);
	break;

    case LG_UNCACHE_MEM:
	cache_only = 1;
	test_mainmem(LONG_UNCACHE);
	break;

    case SH_CACHE_MEM:
	cache_only = 1;
	test_mainmem(SHORT_CACHE);
	break;

    case SH_UNCACHE_MEM:
	cache_only = 1;
	test_mainmem(SHORT_UNCACHE);
	break;

    default:
	cterr('w',0, "Invalid test number %#x", testnum);
	break;
    }

    if ((cache) && (cache_only == 0)) {
	func_ptr = (PFI) PHY_TO_CACHE(func_ptr);
	mem_status |= (*func_ptr)(block_start, block_end, testnum);
    }

    /* 
     * test_mainmem() already print this. Skip here to avoid redundancy
     */
    if (testnum < LG_CACHE_MEM)
	prcomplete(testpass, errcount, 0);
#endif /* LINUX_APP */
    return(0);
}

/**********************************************************************
 *
 * Function name:   march_test()
 *
 * Description:
 *    Memory test using the MARCH C- pattern for word oriented memories
 * The MARCH C- algorithm either increments or decrements through
 * the entire memory, first reading and verifying the current contents
 * and then writing with the complement value.  After read and compare
 * of the complement value, then write again, complementing the current
 * value in memory or writing with a new data value. This continues
 * until all data patterns are tested.
 * The data pattern sequence was chosen to allow reduction
 * of test time by providing the shortest abbreviated test.
 *
 * This test will detect stuck-at and transition faults
 * because all cells are read in states 0, 1, 0 ...
 * This test also satisfies the conditions for detecting
 * address decoder faults and unlinked idempotent (state
 * transition in an agressor cell forces the contents of a victim
 * cell to a certain value, 0 or 1) and unlinked inversion
 * coupling faults (state transition in an agressor cell inverts
 * the contents of a victim cell).  An unlinked fault links
 * one agressor cell to one victim cell whereas a linked
 * fault links 2 or more agressor cells to one victim cell.
 *
 * Algorithm:				      Phase
 *      incr, W0x5a5aa5a5                     1
 *      incr, R0x5a5aa5a5, W0xa5a55a5a        2
 *      incr, R0xa5a55a5a, W0x5a5aa5a5        3
 *      decr, R0x5a5aa5a5, W0xa5a55a5a        4
 *      decr, R0xa5a55a5a, W0x5a5aa5a5        5, ABBREV test to here
 *      incr, R0x5a5aa5a5, W0x3c3cc3c3        6
 *      decr, R0x3c3cc3c3, W0xc3c33c3c        7
 *      incr, R0xc3c33c3c, W0x3c3cc3c3        8
 *      decr, R0x3c3cc3c3, W0xf0f0f0f0        9
 *      incr, R0xf0f0f0f0, W0x0f0f0f0f        10
 *      decr, R0x0f0f0f0f, W0xf0f0f0f0        11
 *      incr, R0xf0f0f0f0, W0x00ff00ff        12
 *      decr, R0x00ff00ff, W0xff00ff00        13
 *      incr, R0xff00ff00, W0x00ff00ff        14
 *      decr, R0x00ff00ff, W0x0000ffff        15
 *      incr, R0x0000ffff, W0xffff0000        16
 *      decr, R0xffff0000, W0x0000ffff        17
 *      incr, R0x0000ffff, W0xffffffff        18
 *      decr, R0xffffffff, W0x00000000        19
 *      incr, R0x00000000, W0xffffffff        20
 *      decr, R0xffffffff, W0x00000000        21
 * where:
 *    incr indicates incrementing through memory
 *    decr indicates decrementing through memory
 *    W    indicates pattern to write
 *    R    indicates pattern to compare read data against
 *
 * Input: test memory structure
 *
 * Output: PASSED if complete successfully, FAILED otherwise
 *
 **********************************************************************
 */
int
march_test(struct testmem *tmemp)
{
    register utype_t *end_addr, *addr_ptr;
    short patrn;

    addr_ptr = tmemp->start;
    end_addr = (utype_t *)((unsigned long)tmemp->start + tmemp->length);

    if (addr_ptr >= end_addr) {
        printf("\ntest aborted, start addr of %p >= end addr of %p\n",
                (void *)addr_ptr, (void *)end_addr);
        return(FAILED);
    }
    testphase = 1;
    prpass(testpass, "fill mem %#.8lx to %#.8lx w/ %#.8x", addr_ptr, 
	   end_addr, march_patterns[0].rd_pat);

    /* 
     * Initialize memory, and print out message every 32MB. 
     */
    while (addr_ptr < end_addr) {
	if (((long)addr_ptr % THIRTYTWO_MEG) == 0)
	    prpass(testpass, "fill %#.8lx with %#.8lx,", 
		   addr_ptr, march_patterns[0].rd_pat);

	*addr_ptr++ = march_patterns[0].rd_pat;
    }

    for (patrn = 0; patrn < num_march_patrns; patrn++) {
	/* do march test */
	if (rvw_mem(tmemp, &march_patterns[patrn])) 
	    return(FAILED);
	
	if ((patrn == 3) && (tmemp->flag & ABBR_TEST))
	    break;
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function name:   display_nearby_mem
 *
 * Description:
 *     Call dismem to display a block of memory centered on target_addr
 * with spread mem_scope bytes.  Ensure that this extent doesn't exceed
 * the start & end in the given testmem struct.
 *
 * Input: test memory structure
 *        pointer middle of block
 *        scope of dump in bytes on either side
 *
 * Output: none
 *
 **********************************************************************
 */
static void
display_nearby_mem (struct testmem *tmemp, utype_t *target_addr, ulong mem_scope)
{
    utype_t *lo_memp, *hi_memp, *hi_bound;
    uint length;   /* total # bytes to display */

    lo_memp = target_addr - mem_scope/VOIDPTRSIZE;
    if (lo_memp < tmemp->start) {
        lo_memp = tmemp->start;
        length = target_addr - tmemp->start;
    } else {
        length = mem_scope;
    }

    /* 
     * CSCsr95584 fix:
     * cast tmemp->length with "unsigned" to accomodate memory bigger than 2GB 
     * Since tmemp->length is declared as signed long, the MSb of this variable 
     * will be treated as signed bit and any arithmetic operation on 
     * tmemp->length which is bigger than 2GB (bigger than 0x7FFF_FFFF)
     * will be a signed operation and will mess up the desired result. 
     * Therefore this fix was necessary to test memory lengths larger than 
     * 2GB.
     */
    hi_bound = tmemp->start + (utype_t) tmemp->length/VOIDPTRSIZE - 1;
    hi_memp = target_addr + mem_scope/VOIDPTRSIZE;
    if (hi_memp > hi_bound) {
        length += hi_bound - target_addr;
    } else {
        length += mem_scope;
    }
    dismem((unsigned char *)lo_memp, length, (unsigned long)lo_memp, sizeof(long));
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
#define NUM_SCOPE_BYTES 256
    register utype_t count, rdata, rd_pat, wr_pat;
    register utype_t *addr_ptr;
    register int adrinc;
    uchar flag;
    char buffer[80];
    ulong phy_addr;

    flag = dpatterns->flag;
    rd_pat = dpatterns->rd_pat;
    wr_pat = dpatterns->wr_pat;

    if (flag) {                 /* increment through memory */
        adrinc = 1;
        addr_ptr = tmemp->start;
    } else {
        adrinc = -1;            /* decrement through memory */
        addr_ptr = tmemp->start + (unsigned long) tmemp->length/VOIDPTRSIZE - 1; /* length in bytes 	      */
								  /* cast tmemp->length with  */
								  /* "unsigned" to accomodate */
								  /* memory bigger than 2GB   */
								  /* See CDETS CSCsr95584     */
    }
    sprintf(buffer, "%s rd_pat %#lx, wr_pat %#lx,",
	    flag ? "increment" : "decrement", rd_pat, wr_pat);
    prpass(testpass, "phase %d, %s", testphase++, buffer);

    /* 
     * CSCsr95584 fix:
     * cast tmemp->length with "unsigned" to accomodate memory bigger than 2GB 
     * Since tmemp->length is declared as signed long, the MSb of this variable 
     * will be treated as signed bit and any arithmetic operation on 
     * tmemp->length which is bigger than 2GB (bigger than 0x7FFF_FFFF)
     * will be a signed operation and will mess up the desired result. 
     * Therefore this fix was necessary to test memory lengths larger than 
     * 2GB.
     */
    for (count = (unsigned long) tmemp->length/VOIDPTRSIZE; count > 0; count--) {
        rdata = *addr_ptr;
        if (rdata != rd_pat) {
            puts("Memory read error dump:");
            display_nearby_mem(tmemp, addr_ptr, NUM_SCOPE_BYTES);

	    if (addr_vtop((ulong)addr_ptr, &phy_addr) == FAIL) {
	        printf("%s %s() failed\n",__FILE__,__FUNCTION__);
	    }
            printf("Memory read error when %s\n"
                "phy-adr %#.lx (vir-adr %p), expect %#.lx, read %#.lx\n",
                flag ? "ascending" : "descending",
                phy_addr, (void *)addr_ptr, rd_pat, rdata);

            return(FAILED);
        }
        *addr_ptr = wr_pat;
        addr_ptr += adrinc;
    }
    return(PASSED);
#undef NUM_SCOPE_BYTES
}

/**********************************************************************
 *
 * FUNCTION NAME:  mem_access8
 *
 * DESCRIPTION:
 *  This will perform byte accesses to main memory
 *  Two tests will be done,
 *  1.  read after write with count down pattern
 *  2.  write all then read all with count up pattern
 *
 * PARAMETERS:  Starting address of memory
 *
 * RETURNS: PASSED if successful, FAILED otherwise
 *
 **********************************************************************
 */
static int 
mem_access8 (char *addr)
{
    int count;
    volatile char *mem_adr;
    uchar rd_val;

    /* test read after write */
    mem_adr = addr;
    for (count = 0xff; count >= 0; count--) {
        *mem_adr = count;
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access8 read after write.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAILED);
        }
        mem_adr++;
    }

    /* test write, then read */
    mem_adr = addr;
    for (count = 0; count <= 0xff; count++) {
        *mem_adr++ = count;
    }
    mem_adr = addr;
    for (count = 0; count <= 0xff; count++) {
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access8 write, then read.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAILED);
        }
        mem_adr++;
    }

    return(PASSED);
}

/**********************************************************************
 *
 * FUNCTION NAME:  mem_access16
 *
 * DESCRIPTION:
 *  This will perform word accesses to main memory
 *  Two tests will be done,
 *  1.  read after write with count down pattern
 *  2.  write all then read all with count up pattern
 *
 * PARAMETERS:  Starting address of memory
 *
 * RETURNS: PASSED if successful, FAILED otherwise
 *
 **********************************************************************
 */
static int 
mem_access16 (short *addr)
{
    int count;
    volatile short *mem_adr;
    unsigned short rd_val;

    /* test read after write */
    mem_adr = addr;
    for (count = 0xff00; count > 0; count -= 0x0100) {
        *mem_adr = count;
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access16 read after write.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
    }

    /* test write, then read */
    mem_adr = addr;
    for (count = 0x0000; count <= 0xff00; count += 0x0100) {
        *mem_adr++ = count;
    }
    mem_adr = addr;
    for (count = 0x0000; count <= 0xff00; count += 0x0100) {
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access16 write, then read.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
    }

    return(PASS);
}

/**********************************************************************
 *
 * FUNCTION NAME:  mem_access32
 *
 * DESCRIPTION:
 *  This will perform longword accesses to main memory
 *  Two tests will be done,
 *  1.  read after write with count down pattern
 *  2.  write all then read all with count up pattern
 *
 * PARAMETERS:  Starting address of memory
 *
 * RETURNS: PASSED if successful, FAILED otherwise
 *
 **********************************************************************
 */
static int 
mem_access32 (int *addr)
{
    uint count, rd_val;
    volatile int *mem_adr;

    /* test read after write */
    mem_adr = addr;
    for (count = 0x00ff0000; count > 0; count -= 0x00010000) {
        *mem_adr = count;
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access32 read after write.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
    }

    /* test write, then read */
    mem_adr = addr;
    for (count = 0; count <= 0x00ff0000; count += 0x00010000) {
        *mem_adr++ = count;
    }
    mem_adr = addr;
    for (count = 0; count <= 0x00ff0000; count += 0x00010000) {
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access32 write, then read.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
    }

    return(PASS);
}

/**********************************************************************
 *
 * FUNCTION NAME:  mem_access64
 *
 * DESCRIPTION:
 *  This will perform double longword accesses to main memory
 *  Two tests will be done,
 *  1.  read after write with count down pattern
 *  2.  write all then read all with count up pattern
 *
 * PARAMETERS:  Starting address of memory
 *
 * RETURNS: PASSED if successful, FAILED otherwise
 *
 **********************************************************************
 */
static int 
mem_access64 (long long *addr)
{
    int cnt;
    long long count;
    volatile long long *mem_adr;
    long long rd_val;

    /* test read after write with count down pattern */
    mem_adr = addr;
    count = 0xff0000ff;
    for (cnt = 0; cnt < 256; cnt++) {
        *mem_adr = count;
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access64 read after write.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
	count -= 0x01000001;
    }

    /* test write, then read with count up pattern */
    mem_adr = addr;
    count = 0;
    for (cnt = 0; cnt < 256; cnt++) {
        *mem_adr++ = count;
        count += 0x0100001000001000ll;
    }
    mem_adr = addr;
    count = 0;
    for (cnt = 0; cnt < 256; cnt++) {
        rd_val = *mem_adr;
        if (rd_val != count) {
	    cterr('f', 0, "Data mismatch in mem_access64 write, then read.\n"
		  "Expect %#x, actual %#x", count, rd_val);
	    return(FAIL);
        }
        mem_adr++;
        count += 0x0100001000001000ll;
    }
    return(PASS);
}

/**********************************************************************
 *
 * FUNCTION NAME:  mem_access_test
 *
 * DESCRIPTION:
 *  This will perform byte, short, word, double longword
 *  accesses to main memory.
 *  Since the same patterns are used, we increment the
 *  starting address by 0x10 each time to ensure that
 *  subsequent R/W are performed by the type of access
 *  that we expect.
 *  This test will take 2K + 0x30 bytes of memory
 *
 * PARAMETERS:  None
 *
 * RETURNS: Starting address
 *
 * RETURNS: PASSED if successful, FAILED otherwise
 *
 **********************************************************************
 */
int
mem_access_test (ulong start_addr, ulong dummy1, ulong dummy2)
{
    int retval;

    testname ("memory access");

    prpass(testpass, "8-bit access,");
    retval = mem_access8((char *)start_addr);

    if (retval == PASS) {
	prpass(testpass, "16-bit access,");
	retval = mem_access16((short *)(start_addr + 0x10));
    }

    if (retval == PASS) {
	prpass(testpass, "32-bit access,");
	retval = mem_access32((int *)(start_addr + 0x20));
    }

    if (retval == PASS) {
	prpass(testpass, "64-bit access,");
	retval = mem_access64((long long *)(start_addr + 0x30));
    }

    return retval;
}


/*
 ****************************************************************************
 *
 *  Function: mem_specific_test
 *
 *  This function tests a memory block with the specified information.
 *  Sometimes memory is accessed via a fixed window. By changing some
 *  mapping values the window is accessing different memmory location.
 *  Thus, a real_start_addr is needed for the pattern equals address test.
 *
 *  Inputs:  memory test information
 *
 *  Outputs: PASS/FAIL
 *
 ****************************************************************************
 */
int mem_specific_test(mem_test_info_t *test_info_p)
{
    volatile ulong *addrptr;
    ulong          pat, increment, loop_count, temp_val, temp1;
    ulong          start, end, u_pattern, test_freq, i;
    MEM_TEST_TYPE  test_type;
    char          *pattern;

    start = test_info_p->start_addr;
    end  = test_info_p->end_addr;
    u_pattern = test_info_p->test_pattern;
    test_freq = test_info_p->test_freq;
    test_type = test_info_p->test_type;

    /*
     * Make sure test frequency is not 0. test_freq = 1 means test every
     * memory location, test_freq = 2 means test every other location. 
     */
    test_freq = (test_freq) ? test_freq : 1;
    switch (test_type) {
    case ADDR_EQ_DATA:
        pattern = "addr equal data";
        pat = test_info_p->real_start_addr;
        increment = 4;
        break;
    case INVERSE_CHECKER:
        pattern = "inverse checkerboard";
        pat = 0xa5a5a5a5;
        increment = 0;
        break;
    case USER_PATTERN:
        pattern = "user specified";
        pat = u_pattern;
        increment = 0;
        break;
    case INCREMENT_PAT:
        pattern = "incrementing";
        pat = u_pattern;
        increment = 1;
        break;
    case MARCH_TEST:
        return(mem_march_test(test_info_p->cache_type, start, end));
        break;
    case ALL_0_THEN_1:
        return(mem_all_zeros_ones_test(start, end, test_freq));
        break;
    case CHECKER_BOARD:
    default:
        pattern = "checkerboard";
        pat = 0x5a5a5a5a;
        increment = 0;
        break;
   
    }

    loop_count = 0;
    prpass(testpass, "memory_test, %s pattern, %#.8x", pattern, pat);

    /*
     * write pattern to memory
     */
    for (addrptr = (volatile ulong *)start; addrptr < (ulong *)end;
                                 addrptr += test_freq, loop_count++) {
        temp_val = (pat + (loop_count * increment));
        if (((ulong)addrptr % TWO_MEG) == 0)
            prpass(testpass, "write addr %#.8x %s pattern, %#.8x,",
                                     (ulong)addrptr, pattern, temp_val);
        *addrptr = temp_val;
    }

    if (test_info_p->retention_time) {
	/* Data retention time needed between write and read */
	for (i = 0; i < test_info_p->retention_time; i++) {
	    msleep(1);	/* wait for one milli-second */
	    if (!(i & 0xFFF)) {
		/* Print every 4096 milliseconds */
		printf(".");
	    }
	}
    }

    /*
     * Read back and verify
     */
    loop_count = 0;
    for (addrptr = (volatile ulong *)start; addrptr < (ulong *)end;
                                 addrptr += test_freq, loop_count++) {
        temp_val = pat + (loop_count * increment);
        if (((ulong)addrptr % TWO_MEG) == 0)
            prpass(testpass, "verify addr %#.8x %s pattern, %#.8x,",
                   (ulong)addrptr, pattern, temp_val);

        if ((temp1 = *addrptr) != temp_val) {
            cterr('f',0,"Memory %s, failure at addr: %#.8x.\nExpected: %#.8x,"
                        " Read: %#.8x, Count %#x, re-read: %#x.", pattern,
                         addrptr, temp_val, temp1, loop_count, *addrptr);
            return(FAILED);
        }
    }
    return(PASSED);
}


/*
 ****************************************************************************
 *
 *  Function: mem_all_zeros_ones_test
 *
 *  This function tests a memory block with pattern alternating between
 *  0x00000000 and 0xffffffff.
 *
 *  Inputs:  start location
 *           end location
 *           frequency
 *
 *  Outputs: PASS/FAIL
 *
 ****************************************************************************
 */
int mem_all_zeros_ones_test(ulong start, ulong end, ulong test_freq)
{
    ulong temp_val, temp_val1, pat;
    volatile ulong *addrptr;

    prpass(testpass, "memory_test, all ones/zeros pattern");

    /*
     * write pattern to memory
     */
    pat =  0;
    for (addrptr = (volatile ulong *)start; addrptr < (ulong *)end;
                                               addrptr += test_freq) {
        temp_val = pat = (pat) ? 0 : 0xffffffff;
        if (((ulong)addrptr % TWO_MEG) == 0)
            prpass(testpass, "ones/zeros test, write addr %#.8x, value %#.8x,",
                                                      (ulong)addrptr, temp_val);
        *addrptr = temp_val;

    }

    /*
     * read back and verify
     */
    pat =  0;
    for (addrptr = (volatile ulong *)start; addrptr < (ulong *)end;
                                               addrptr += test_freq) {
        temp_val = pat = (pat) ? 0 : 0xffffffff;
        if (((ulong)addrptr % TWO_MEG) == 0)
            prpass(testpass, "ones/zeros test, verify addr %#.8x,",
                                                     (ulong)addrptr);

        if ((temp_val1 = *addrptr) != temp_val) {
            cterr('f',0,"Memory ones/zeros test failed at %#.8x.\n"
                        "Expected: %#.8x, Read: %#.8x, re-read: %#x.",
                         addrptr, temp_val, temp_val1, *addrptr);
            return(FAILED);
        }
    }

    return(PASSED);
}


/*
 ****************************************************************************
 *
 *  Function: init_mem_test_info_ds
 *
 *  This function initializes a memory test ds to some default values.
 *
 *  Inputs:  memory test information
 *
 *  Outputs: PASS/FAIL
 *
 ****************************************************************************
 */
void init_mem_test_info_ds(mem_test_info_t *mem_info_p)
{
    mem_info_p->start_addr      = 0;
    mem_info_p->end_addr        = 0;
    mem_info_p->test_type       = CHECKER_BOARD;
    mem_info_p->cache_type      = SH_UNCACHE_MEM;
    mem_info_p->test_pattern    = 0;
    mem_info_p->test_freq       = 1;
    mem_info_p->real_start_addr = 0;
    mem_info_p->retention_time  = 0;
}

/**********************************************************************
 *
 * Function: dis_mem_sub
 *
 * a wrapper function for dis_mem to be used for submenu.
 *
 * Input : a dummy parameter
 *
 * Output: NONE
 *
 **********************************************************************
 */
void dis_mem_sub (void *dummy)
{
    dis_mem(1, 0);
}

/**********************************************************************
 *
 * Function: fil_mem_sub
 *
 * a wrapper function for fil_mem to be used for submenu.
 *
 * Input : a dummy parameter
 *
 * Output: NONE
 *
 **********************************************************************
 */
void fil_mem_sub (void *dummy)
{
    fil_mem(1, 0);
}

/**********************************************************************
 *
 * Function: pci_conf_wr_util
 *
 * This function writes to a PCI configuration register specified by
 * a user.
 *
 * Input : PCI bus number, PCI device number
 *
 * Output: NONE
 *
 **********************************************************************
 */
void
pci_conf_wr_util (ulong bus_num, ulong pci_dev_num)
{
    ulong readval, offset, value;

    offset      = gethex_answer("Offset : ", 0, 0, 0xff);
    readval     = pci_config_read(bus_num, pci_dev_num, 0, offset);
    value       = gethex_answer("Value  : ", readval, 0, 0xffffffff);
    pci_config_write(bus_num, pci_dev_num, 0, offset, value);

    if (getc_answer("Read back? ", "yn", 'y') == 'y') {
        readval = pci_config_read(bus_num, pci_dev_num, 0, offset);
        printf("%#.2lx: %#.8lx\n", offset, readval);
    }
}

/**********************************************************************
 *
 * Function: pci_conf_rd_util
 *
 * This function reads a PCI configuration register specified by
 * a user.
 *
 * Input : PCI bus number, PCI device number
 *
 * Output: NONE
 *
 **********************************************************************
 */
void
pci_conf_rd_util (ulong bus_num, ulong pci_dev_num)
{
    uint  i;
    ulong readval, offset;

    offset = gethex_answer("Offset (ff for all): ", 0, 0, 0xff);
    if (offset == 0xff) {
        for (i=0; i < 0x60; i+=4) {
            readval = pci_config_read(bus_num, pci_dev_num, 0, i);
            printf("%#.2x: %#.8lx\n", i, readval);
        }
    } else {
        readval = pci_config_read(bus_num, pci_dev_num, 0, offset);
        printf("%#.2lx: %#.8lx\n", offset, readval);
    }
}


/**********************************************************************
 *
 * Function: pcie_conf_wr_util
 *
 * This function writes to a PCIe configuration register specified by
 * a user.
 *
 * Input : PCI bus number, PCI device number
 *
 * Output: NONE
 *
 **********************************************************************
 */
void
pcie_conf_wr_util (uint32_t pcie_port, uint32_t bus_num, uint32_t pci_dev_num)
{
    uint32_t readval, offset, value;

    offset      = gethex_answer("Offset : ", 0, 0, 0xff);
    readval     = pci_config_read(bus_num, pci_dev_num, 0, offset);
    value       = gethex_answer("Value  : ", readval, 0, 0xffffffff);
    pci_config_write(bus_num, pci_dev_num, 0, offset, value);

    if (getc_answer("Read back? ", "yn", 'y') == 'y') {
        readval = pcie_config_read(pcie_port, bus_num,
                                   pci_dev_num, 0, offset);
        printf("%#.2x: %#.8x\n", offset, readval);
    }
}

/**********************************************************************
 *
 * Function: pcie_conf_rd_util
 *
 * This function reads a PCIe configuration register specified by
 * a user.
 *
 * Input : PCI bus number, PCI device number
 *
 * Output: NONE
 *
 **********************************************************************
 */
void
pcie_conf_rd_util (uint32_t pcie_port, uint32_t bus_num, uint32_t pci_dev_num)
{
    uint  i;
    uint32_t readval, offset;

    offset = gethex_answer("Offset (ff for all): ", 0, 0, 0xff);
    if (offset == 0xff) {
        for (i=0; i < 0x60; i+=4) {
            readval = pcie_config_read(pcie_port,
                                       bus_num, pci_dev_num, 0, i);
            printf("%#.2x: %#.8x\n", i, readval);
        }
    } else {
        readval = pcie_config_read(pcie_port, bus_num,
                                   pci_dev_num, 0, offset);
        printf("%#.2x: %#.8x\n", offset, readval);
    }
}

/**********************************************************************
 *
 * Function: pci_find_capability
 *
 * This function walks the capability list in the pcie config space
 * searching for the register containing the cap id.
 * once the cap id is found, it returns the register offset.
 * some of the interesting cap id are:  PCI_MSI_CAP_ID, PCI_EXP_CAP_ID, etc...
 * 
 *
 * Input : PCI bus number, PCI device number, PCI function number,
 *         cap -- the cap id we are want to search for
 *
 * Output: cap_offset- pcie register offset of the desired cap id; or
 *         '0' if cap id is not found.
 *
 **********************************************************************
 */
int
pci_find_capability(int bus, int dev, int func, int cap)
{

    int cap_id, cap_offset, cap_block;
    
    cap_offset = pci_config_read(bus, dev, func, PCI_CAP_PTR_OFFSET);
    
    while (cap_offset != 0) {
        cap_block = pci_config_read(bus, dev, func, cap_offset);
        cap_id = cap_block & PCI_EXP_CAP_ID_MASK;
        
        if (cap_id == cap) {
            /* configure MSI cap block here */
#ifdef MSI_DBG
            printf("pci_find_capability cap_offset %#x\n",  cap_offset);
#endif
            return cap_offset;
        } 
        cap_offset = (cap_block & PCI_EXP_CAP_NEXTPTR_MASK) >> 
	    PCI_EXP_CAP_NEXTPTR_SHIFT;
    }
    return 0;
}

/**********************************************************************
 *
 * Function: msi_set_enable
 *
 * This function turns on or off msi interrupt bit in the msi control register
 * in the pcie config space. Some modules that do not conform completely with
 * PCIE spec may not have this register.
 *
 * Input : PCI bus number, PCI device number, PCI function number,
 *         enable -- interrupt flag,  set for enabling interrupt.
 *
 * Output: PASSED or FAILED;
 *
 **********************************************************************
 */
int
msi_set_enable(int bus, int dev, int func, int enable)
{
    uint32_t cap_offset, data = 0;
    
    cap_offset = pci_find_capability(bus, dev, func, PCI_MSI_CAP_ID);
    
    if (!cap_offset) {
        return FAILED;
    }

    data = pci_config_read(bus, dev, func, cap_offset);
    if (enable) 
        data |= 1<<15;
    else
        data &= ~(1<<15);
    pci_config_write(bus, dev, func, cap_offset, data);

    return PASSED;
}

/**********************************************************************
 *
 * Function: write_msi_message
 *
 * This function writes msi data and msi address to the device's PCIE config
 * space.
 *
 * Input : PCI bus number, PCI device number, PCI function number,
 *         msi_msg_t structure
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
int
write_msi_msg(int bus, int dev, int func, msi_msg_t *msi)
{
    uint32_t cap_offset;

    cap_offset = pci_find_capability(bus, dev, func, PCI_MSI_CAP_ID);
    if (!cap_offset) {
        return FAILED;
    }

    pci_config_write(bus, dev, func, cap_offset+0x4, 
                     msi->address_lo);
    pci_config_write(bus, dev, func, cap_offset+0x8, 
                     msi->address_hi);
    pci_config_write(bus, dev, func, cap_offset+0xc, 
                     msi->data);
    
    return PASSED;
}

/* End of File */


/******** History ******** 
$Log: common_utils.c,v $
Revision 1.12  2020/08/19 09:49:17  markzha
*** empty log message ***

Revision 1.11  2018/08/31 03:59:29  chieyang
Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2

Revision 1.10  2018/08/06 02:57:21  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.9.34.1  2018/03/29 06:37:42  lucywang
Fixed bug of Thermal Sensor Test

Revision 1.9  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.8  2014/08/18 22:15:14  yuetwang
add engineering banner

Revision 1.7  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.6  2013/05/09 19:25:15  mcharon
remove unused header files. fixed dependancy compile problem

Revision 1.5  2012/11/06 20:39:49  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.4  2012/06/05 09:33:44  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/05/22 07:31:31  alpeng
Remove cterr from march C test, should put cterr outside of diag

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

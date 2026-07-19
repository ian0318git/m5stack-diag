/* $Id: linux_memory_tester.c,v 1.14 2020/08/19 09:49:17 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_memory_tester.c,v $
 *-----------------------------------------------------------------------------
 * linux_memory_tester.c - Porting from Linux API memtester ver 4.2.1.
 *
 * July 2011, Alan Peng
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *DESCRIPTION
 *     memtester is an effective userspace tester for stress-testing the
 *     memory subsystem.  It is very effective at finding intermittent and
 *     non-deterministic faults.  Note that problems in other hardware areas
 *     (overheating CPU, out-of-specification power supply, etc.) can cause
 *     intermittent memory faults, so it is still up to you to determine
 *     where the fault lies through normal hardware diagnostic procedures;
 *     memtester just helps you determine whether a problem exists.
 *
 *     memtester will malloc(3) the amount of memory specified, if possible.
 *     If this fails, it will decrease the amount of memory requested until
 *     it succeeds.  It will then attempt to mlock(3) this memory; if it
 *     cannot do so, testing will be slower and much less effective.  Run
 *     memtester as root so that it can mlock the memory it tests.
 *
 *     Note that the maximum amount of memory that memtester can test will be
 *     less than the total amount of memory installed in the system; the
 *     operating system, libraries, and other system limits take some of the
 *     available memory.  memtester is also limited to the amount of memory
 *     available to a single process; for example, on 32-bit machines with
 *     more than 4GB of memory, memtester is still limited to less than 4GB.
 *
 *     Note that it is up to you to know how much memory you can safely
 *     allocate for testing.  If you attempt to allocate more memory than is
 *     available, memtester should figure that out, reduce the amount
 *     slightly, and try again.  However, this can lead to memtester
 *     successfully allocating and mlocking essentially all free memory on
 *     the system -- if other programs are running, this can lead to
 *     excessive swapping and slowing the system down to the point that it is
 *     difficult to use.  If the system allows allocation of more memory than
 *     is actually available (overcommit), it may lead to a deadlock, where
 *     the system halts.  If the system has an out-of-memory process killer
 *     (like Linux), memtester or another process may be killed by the OOM
 *     killer.
 *     So choose wisely
 *
 *-----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/kernel.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "linux_memory_tester.h"
#include "linux_memory_tester_utils.h"
#include "common_utils.h"
#include "setjmps.h"
#include "nvmonvars.h"
#include "queryflags.h"

/* define test method for memtester */
struct test_method tests[] = {
    { "March C",              mem_march_test               },
    { "Random Value",         test_random_value            },
    { "Compare XOR",          test_xor_comparison          },
    { "Compare SUB",          test_sub_comparison          },
    { "Compare MUL",          test_mul_comparison          },
    { "Compare DIV",          test_div_comparison          },
    { "Compare OR",           test_or_comparison           },
    { "Compare AND",          test_and_comparison          },
    { "Sequential Increment", test_seqinc_comparison       },
    { "Solid Bits",           test_solidbits_comparison    },
    { "Block Sequential",     test_blockseq_comparison     },
    { "Checkerboard",         test_checkerboard_comparison },
    { "Bit Spread",           test_bitspread_comparison    },
    { "Bit Flip",             test_bitflip_comparison      },
    { "Walking Ones",         test_walkbits1_comparison    },
    { "Walking Zeroes",       test_walkbits0_comparison    },
    { "Stuck Address",        test_stuck_address           },
#ifdef TEST_NARROW_WRITES
    { "8-bit Writes",         test_8bit_wide_random        },
    { "16-bit Writes",        test_16bit_wide_random       },
#endif
    { NULL,                   NULL                         }
};

#define MEMTESTER_DEBUG 0

/*******************************************************************************
 *                            Function prototypes
 *******************************************************************************/
void check_posix_system(void);
int memtester_pagesize(void);
static ulong getmemfree(void);
int test_patterns(int);
int start_test(int , void volatile *, void volatile *, ulv *, ulv *, int, ull);
extern int mem_march_test(MEM_CACH_TYPE, ulong, ulong);
int linux_memory_tester(int);
int linux_memory_tester_with_ecc_check(int);


/*******************************************************************************
 *                                 Globals
 *******************************************************************************/
/* Some systems don't define MAP_LOCKED.  Define it to 0 here
   so it's just a no-op when ORed with other constants. */
#ifndef MAP_LOCKED
  #define MAP_LOCKED 0
#endif
/* decide the test is automatically or not */
static uint donot_query = FALSE;


/*******************************************************************************
 *                                   Menus
 *******************************************************************************/

/*
 * submenu for memtester tests
 */
static submenu_xtable_t memtester_test_table[] = {
    {"March C",                 (PFT)test_patterns,      0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Random Value",            (PFT)test_patterns,      1,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare XOR",             (PFT)test_patterns,      2,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare SUB",             (PFT)test_patterns,      3,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare MUL",             (PFT)test_patterns,      4,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare DIV",             (PFT)test_patterns,      5,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare OR",              (PFT)test_patterns,      6,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Compare AND",             (PFT)test_patterns,      7,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Sequential Increment",    (PFT)test_patterns,      8,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Solid Bits",              (PFT)test_patterns,      9,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Block Sequential",        (PFT)test_patterns,      10,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Checkerboard",            (PFT)test_patterns,      11,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Bit Spread",              (PFT)test_patterns,      12,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Bit Flip",                (PFT)test_patterns,      13,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Walking Ones",            (PFT)test_patterns,      14,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Walking Zeroes",          (PFT)test_patterns,      15,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"Stuck Address",           (PFT)test_patterns,      16,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
#ifdef TEST_NARROW_WRITES
    {"8-bit Writes",            (PFT)test_patterns,      17,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
    {"16-bit Writes",           (PFT)test_patterns,      18,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,    (type_t(*)())0,       0},
#endif

};

#define MEMTESTER_TEST_TABLE_SIZE  \
    (sizeof(memtester_test_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t memtester_tests_primary_items[MEMTESTER_TEST_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t memtester_tests_secondary_items[MEMTESTER_TEST_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

menuinfo_t memtester_testmenu = {
    "Memtester Menu",             /* title */
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    memtester_tests_primary_items,
};

menuinfo_t *memtester_testmenup = &memtester_testmenu;

/*
 * Function: mem_ecc_check
 *
 * Description : Using mcelog to check ECC
 *
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
static int mem_ecc_check (void)
{
    FILE *fp;
    int rc = PASSED;
    char status_file[32];
    char buf[64];

    sprintf(status_file, "/var/log/mcelog");

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("Unable to open '%s'\n", status_file);
        fclose(fp);
        rc = FAILED;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "ECC")) {
            printf("%s\n", buf);
            rc = FAILED;
        }
    }

    fclose(fp);
    return (rc);
}

/*******************************************************************************
 * Function   : linux_memory_tester_with_ecc_check
 *
 * Description: To build memtester main menu and check memory ECC error.
 *
 * Inputs     : None.
 *
 * Outputs    : None.
 *******************************************************************************
 */
int linux_memory_tester_with_ecc_check (int exe_march_c_test)
{
    int rc = FAILED;
    FILE *fp;
    char status_file[64];
    char mv_cmd[64], touch_cmd[64];

    sprintf(status_file, "/var/log/mcelog");

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("Unable to open '%s'\n", status_file);
        fclose(fp);
        return (rc);
    }

    if (fgetc(fp) != EOF) {
        fclose(fp);
        if (mem_ecc_check() == PASSED) {
            /* No ECC error found but mcelog file got some hardware events */
            printf("mcelog file is located in directory /var/log/\n");
            sprintf(mv_cmd, "mv /var/log/mcelog /var/log/mcelog_tmp");
            system(mv_cmd);
            sprintf(touch_cmd, "touch /var/log/mcelog");
            system(touch_cmd);
        } else {
            cterr('f',0,"Get memory ECC error, check mcelog file(/var/log/mcelog) for details\n");
            return (rc);
        }
    } else {
        fclose(fp);
    }

    /* Run memory testing */
    if (linux_memory_tester(exe_march_c_test) == PASS) {
        /* Check memory ECC after memory testing */
        if (mem_ecc_check() == PASSED) {
            rc = PASSED;
        } else {
            cterr('f',0,"Get memory ECC error, check mcelog file(/var/log/mcelog) for details\n");
        }
    }

    return (rc);
}

/*******************************************************************************
 * Function   : linux_memory_tester
 *
 * Description: To build memtester main menu.
 *
 * Inputs     : None.
 *
 * Outputs    : None.
 *******************************************************************************
 */
int linux_memory_tester (int exe_march_c_test)
{
    build_primary_submenu(memtester_test_table, MEMTESTER_TEST_TABLE_SIZE,
			                    "Memory Tester Menu", &memtester_testmenup);
    build_secondary_submenu(memtester_test_table, MEMTESTER_TEST_TABLE_SIZE,
			                      memtester_tests_secondary_items);

    if (exe_march_c_test) {
        menu(&memtester_testmenu, memtester_tests_secondary_items, 0);
        return PASS;
    } else {
        /* avoid to ask user memory size in test_patterns.
         * Select memory size automatically,
         * and process infinite test.
         * For now is only march_C test.
         */
        donot_query = TRUE;
    	   if (!exec_doall_menu_items(&memtester_testmenu)) {
             /*
              * User did <BREAK>.  Display accumulated errors here only if
              * not a continuous run because display will occur in menu() as
              * a result of <BREAK>.
              */
             if (!(DIAGFLAG & D_CONTINUOUS)) {
                donot_query = TRUE;
                menu_pr_err_accum();
             }
             if (monjmpptr) {
                longjmp(*monjmpptr, 1);  /* Back to previous point */
             }
         }
    }
    return PASS;

}


/*******************************************************************************
 * Function   :	check_posix_system
 *
 * Description:	Sanity checks and portability helper macros.
 *
 * Inputs     :	NONE
 *
 * Outputs    : NONE
 *******************************************************************************
 */

#ifdef _SC_VERSION
void check_posix_system (void) {
    if (sysconf(_SC_VERSION) < 198808L) {
        fprintf(stderr, "A POSIX system is required.  Don't be surprised if "
            "this craps out.\n");
        fprintf(stderr, "_SC_VERSION is %lu\n", sysconf(_SC_VERSION));
    }
}
#else
#define check_posix_system()
#endif


/*******************************************************************************
 * Function   :	memtester_pagesize
 *
 * Description:	get the pagesize
 *
 * Inputs     :	NONE
 *
 * Outputs    : NONE
 *******************************************************************************
 */

#ifdef _SC_PAGE_SIZE
int memtester_pagesize (void) {
    int pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        cterr('f',0,"get page size failed\n");
        exit(EXIT_FAIL_NONSTARTER);
    }
#ifdef MEMTESTER_DEBUG
    printf("pagesize is %ld\n", (long) pagesize);
#endif
    return pagesize;
}
#else
int memtester_pagesize (void) {
    printf("sysconf(_SC_PAGE_SIZE) not supported; using pagesize of 8192\n");
    return 8192;
}
#endif


/**********************************************************************
 *
 * Function: getmemfree
 *
 * Description: Get the MemFree value from /proc/meminfo
 *
 * Input: none
 *
 * Return: Free memory reported by reading /proc/meminfo
 ***********************************************************************/
static ulong getmemfree (void)
{
    struct sysinfo sys_info;

    if(sysinfo(&sys_info) != 0) {
        cterr('f',0,"%s() sysinfo call failed\n",__FUNCTION__);
	return -1;
    }
    /*
    printf("MemTotal= %ld KB %ld MB\n",
	   sys_info.totalram/ONE_K, sys_info.totalram/ONE_MEG);
    printf("MemFree= %ld KB %ld MB\n",
	   sys_info.freeram/ONE_K, sys_info.freeram/ONE_MEG);
    */
    return(sys_info.freeram);
}


/*******************************************************************************
 * Function   :	init_test_env
 *
 * Description:	init the test environment. Get the free memory size and pass it.
 *
 * Inputs     :	None.
 *
 * Outputs    : adjust_size.
 *******************************************************************************/
ull init_test_env (void)  {

    ull freememsz, ovrhd_sz, adjust_size;
    float ovrhd_factor;

    /* Testing DRAM in Linux has constrants.
     * We can malloc all the free mem available but we can't
     * test all of it. We must adjust the free memory size that
     * the malloc gave us by a factor.
     * This only happened on the Cavium eval
     * board. Our ngd Linux server do not have this issue.
     */

    ovrhd_factor = get_mem_overhead_factor();
    freememsz = getmemfree();
    ovrhd_sz = freememsz * ovrhd_factor;
    adjust_size = freememsz - ovrhd_sz;
    printf("Testing %dMB free memory\n", (unsigned int)(adjust_size/ONE_MEG));

    /* open vtop driver,
     * user should insmod & mknod the addr_vtop before test
     */

    return (adjust_size);
}

/*******************************************************************************
 * Function   :	pass_offset
 *
 * Description: Get the random number from /dev/urandom for offset and pass it.
 *
 * Inputs     :	None.
 *
 * Outputs    : urandomint.
 *
 * Notes      : /dev/random transforms the environmental noise as random number.
 *              If the noise is not enough, to read /dev/random will block
 *              until more noise is gathered.
 *              Thus, using /dev/urandom to avoid probability of block.
 *******************************************************************************/
static ull pass_offset (void) {

    ull urandom_ull;
    int fd_urand;

    if ((fd_urand = open("/dev/urandom", O_RDONLY)) < 0) {
        //        printf("%s(): Fail to open /dev/urandom", __FUNCTION__);
        assert(!"unable to open /dev/urandom");
        return (FAILED);
    }

    read(fd_urand, &urandom_ull, sizeof urandom_ull);

#ifdef MEMTESTER_DEBUG
    printf("urandom_number = %llu \n", urandom_ull);
#endif

    close(fd_urand);

    return (urandom_ull);
}

/*******************************************************************************
 * Function   :	test_patterns
 *
 * Description:	run the memtester for test
 *
 * Inputs     :	tst_type - select the test types
 *
 * Outputs    : PASSED/FAILED.
 *
 * Notes      : User should know how much memory you can safely allocate for
 *              testing.
 *              If the system has an out-of-memory process killer (like Linux)
 *              memtester or another process may be killed by the OOM killer.
 *
 *      If MSA is not multiple of pagesize, it will align to new address,
 *      than the test range is shown below.
 *
 *      Malloc Start Address: MSA             MSA
 *      Aligned Address: A_A                 / | \
 *      Test Size: T_S                   L_A(  |  \
 *      End address: E_A                     \ |   \
 *      Lost Address: L_A (a little)          A_A   \
 *      Real Test Size: RTS                  / |     ) T_S
 *                                          /  |    /
 *                                      RTS(   |   /
 *                                          \  |  /
 *                                           \ | /
 *                                            E_A
 *
 *      The size of L_A is (A_A - MSA), and L_A is less than pagesize.
 *
 *******************************************************************************
 */
int test_patterns (int tst_typ)  {
    ul loops, loop;
    ull pagesize, wantmb, wantbytes, wantbytes_orig,
      bufsize, halflen, count, mem_size, curr_size, offset;

    ptrdiff_t pagesizemask;
    void volatile *buf, *aligned, *end_addr;
    ulv *bufa, *bufb;
    int do_mlock = 1, done_mem = 0;
    int exit_code = 0;
    size_t maxbytes = -1; /* addressable memory, in bytes */
    size_t maxmb = (maxbytes >> 20) + 1; /* addressable memory, in MB */
    int mlock_return;

    testname("Linux memory tester");

    check_posix_system();
    pagesize = memtester_pagesize();
    pagesizemask = (ptrdiff_t) ~(pagesize - 1);

#if MEMTESTER_DEBUG
    printf("pagesizemask is 0x%tx\n", pagesizemask);
#endif

    /* Get the free memory size for testing. */
    curr_size = init_test_env();
    if (curr_size == FAILED) return (FAILED);


    /* if test automatically, we should not query to user */
    if (donot_query == TRUE) {
    	  mem_size = (curr_size/ONE_MEG);
    } else {
        /* unit of curr_size is KB, (curr_size/ONE_MEG) is for MB*/
        printf("\nToo large memory size may process oom killer in Linux");
        printf("\nCurrent available memeory size: %d MB",
			(unsigned int)(curr_size/ONE_MEG));
        mem_size = getdec_answer("\nEnter test memory size: (MB)"
            , 0, 0, (curr_size/ONE_MEG));
    }


    wantbytes_orig = wantbytes = (mem_size << MEGASHIFT);
    wantmb = (wantbytes_orig >> MEGASHIFT);

    if (wantmb > maxmb) {
        fprintf(stderr, "This system can only address %llu MB.\n", (ull) maxmb);
        exit(EXIT_FAIL_NONSTARTER);
    }
    if (wantbytes < pagesize) {
        fprintf(stderr, "bytes %llu < pagesize %llu -- memory argument too large?\n",
                wantbytes, pagesize);
        exit(EXIT_FAIL_NONSTARTER);
    }


    /* if test automatically */
    if (donot_query == TRUE) {
    	  /* default is run one time */
    	  loops = 1;
    } else {
        printf("\nEnter test times, zero for infinite loop.");
        loops = getdec_answer("\nTest times (default zero): ", 0, 0, 65535);
        /* 65535 for size of unsigned int. */
    }


    if (loops > 0) { errno = 0; }

    for(loop = 1; ((!loops) || loop <= loops); loop++) {

#if MEMTESTER_DEBUG
        printf("want %lluMB (%llu bytes)\n", (ull)wantmb, (ull)wantbytes);
#endif
        buf = NULL;

        while (!done_mem) {
            while (!buf && wantbytes) {
                buf = (void volatile *) malloc(wantbytes);
                if (!buf) {
            	      wantbytes -= pagesize;
                }
            }

            /* keep original size 12.01.16*/
            bufsize = wantbytes;
#if MEMTESTER_DEBUG
            printf("Malloc ");
#endif

            if ((donot_query == TRUE) && (!(diagflag_xram & D_MIN_TEST_TIME))) {
                /* the MIN_TEST_TIME ONLY for executing March_C directly,
    	           * reducing test size with DIV_SIZE
    	           */
    	          bufsize = wantbytes >> DIV_SIZE;
    	          printf("Min test time, test size is %lluMB (%llu bytes) \n",
                    (ull) bufsize >> MEGASHIFT, (ull) bufsize);
                printf("Malloc all ");

            } else {
                /* keep original size */
                bufsize = wantbytes;
                printf("Malloc ");
            }

#if MEMTESTER_DEBUG
            printf("free memory %lluMB (%llu bytes)",
                (ull) wantbytes >> MEGASHIFT, (ull) wantbytes);
            fflush(stdout);
#endif

            if (do_mlock) {

	        /* Check if address of malloc is the mutiple of pagesize. */
	        if ((size_t) buf % pagesize) {
		    /* printf("aligning to page -- was 0x%tx\n", buf); */
		    aligned = (void volatile *) ((size_t) buf & pagesizemask) + pagesize;
		    /* printf("  now 0x%tx -- lost %d bytes\n", aligned,
		     *      (size_t) aligned - (size_t) buf);
		     */
		    bufsize -= ((size_t) aligned - (size_t) buf);
                } else {
		    aligned = buf;
                }

                /* if directly March_C && MIN_TEST_TIME */
                if ((donot_query == TRUE) && (!(diagflag_xram & D_MIN_TEST_TIME))) {
		    offset = pass_offset();
		    if (offset == FAILED) {
		        assert(!"linux_memory_tester.c: invalid offset");
			return (FAILED);
		    }

#ifdef MEMTESTER_DEBUG
              printf("Before: aligned = %p \n", aligned);
#endif

              /*
               *  Malloc all the freememsize, and pick up 1/4 of
               *  freememsize as bufsize. After randomly offset the
               *  start address, the (start address + bufsize)
               *  should not over the range of freememesize.
               *
               *           3/4 of freemem           1/4 of freemem
               *    <------- range of S.A -------->
               *   |-------------------------------|---------------|
               *  S.A.                      limit of S.A.         E.A.
               *   |<--------------all the freememsize------------>|
               */
              aligned = aligned + (offset % (wantbytes - bufsize));

#ifdef MEMTESTER_DEBUG
              printf("After: aligned = %p, wantbytes = %llu, bufsize = %llu \n",
                  aligned, wantbytes, bufsize);
#endif
                }

            /* Try mlock */
            mlock_return = mlock((void *) aligned, bufsize);
            if (mlock_return < 0) {
                switch(errno) {
                    case ENOMEM:
                        printf("too many pages, reducing...\n");
                        free((void *) buf);
                        buf = NULL;
                        wantbytes -= pagesize;
                        break;
                    case EPERM:
                        printf("insufficient permission.\n");
                        printf("Trying again, unlocked:\n");
                        do_mlock = 0;
                        free((void *) buf);
                        buf = NULL;
                        wantbytes = wantbytes_orig;
                        break;
                    default:
                        printf("failed for unknown reason.\n");
                        do_mlock = 0;
                        done_mem = 1;
                    }
            } else {
#if MEMTESTER_DEBUG
                printf("Mlocked.\n");
#endif
                done_mem = 1;
            } /* if (mlock_return < 0) */

            } else {
                done_mem = 1;
                printf("\n");
            } /* if (do_mlock) */
        } /* while (!done_mem) */

        if (!do_mlock) fprintf(stderr, "Continuing with unlocked memory; "
                           "testing will be slower and less reliable.\n");

        halflen = bufsize / 2;
        count = halflen / sizeof(ul);
        bufa = (ulv *) aligned;
        bufb = (ulv *) ((size_t) aligned + (size_t) halflen);
        end_addr = (void volatile *)((size_t) aligned + (size_t) bufsize);

    if (donot_query == FALSE) {
        printf("Loop %lu", loop);

        if (loops) {
            printf("/%lu", loops);
        }
        printf(":\n");

        fflush(stdout);
    }


        if (donot_query == TRUE) {
            tst_typ = 0;
            exit_code = start_test(tst_typ, aligned, end_addr, bufa, bufb, count, bufsize);
#if 0
            while(1) {
                if((exit_code) || (!tests[tst_typ].name)){
                    break;
                } else {
                    exit_code = start_test(tst_typ, aligned, end_addr, bufa, bufb, count, bufsize);
                    tst_typ++;
                }
            }
#endif
       } else {
       	   exit_code = start_test(tst_typ, aligned, end_addr, bufa, bufb, count, bufsize);
       }

        if (do_mlock) munlock((void *) aligned, bufsize);

        free((void *) buf);

        if(exit_code)
           cterr('f',0,"Linux memtester failed, errcode %d\n", exit_code);

        /* reset these variables for next loop */
        do_mlock = 1;
        done_mem = 0;

    } /* for(loop = 1; ((!loops) || loop <= loops); loop++) */

    donot_query = FALSE; /* clean the flag*/
    fflush(stdout);

    return (PASSED);
}


int start_test(int tst_typ, void volatile *aligned, void volatile *end_addr,
               ulv *bufa, ulv *bufb, int count, ull bufsize) {

    int exit_code = 0;
    /*
     * The parameter for test_stuck_address and mem_march_test
     * are different with other tests,
     * so bring it out from others.
     */
    prpass(testpass,"%s:", tests[tst_typ].name);

    if (tst_typ == SELECT_STUCK_ADDR) {
        if (!test_stuck_address(aligned, bufsize / sizeof(ul))) {
            //printf("ok\n");
        } else {
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
    } else if (tst_typ == SELECT_MARCH_C) {
        if(!mem_march_test(LONG_UNCACHE, (ulong)aligned, (ulong)end_addr)) {
            //printf("ok\n");
        } else {
            exit_code |= EXIT_FAIL_MARCH_C;
        }
    } else {
         /* executes corresponding tests from test menu. */
         if (!tests[tst_typ].fp(bufa, bufb, count)) {
             //printf("ok\n");
         } else {
             exit_code |= EXIT_FAIL_OTHERTEST;
         }
    }
    if(!exit_code){
    	printf("passed");
        /*left spaces for cover redundant message which is from March-C. */
    	printf("                ");
    }

    if (donot_query == FALSE)
        printf("\n");

    fflush(stdout);

    return(exit_code);
}

/******** History ********
*---------------------------------------------------
$Log: linux_memory_tester.c,v $
Revision 1.14  2020/08/19 09:49:17  markzha
*** empty log message ***

Revision 1.13  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.12  2016/05/06 17:44:26  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.11.2.5  2017/08/11 07:50:34  leschen
Point out the mcelog file directory when it's not empty.

Revision 1.11.2.4  2017/07/18 06:25:13  leschen
Fix memory testing double free pointer problem.

Revision 1.11.2.3  2017/04/05 06:40:22  leschen
Sync with <ng_diag-tag-032917>

Revision 1.11.2.2  2016/11/29 07:27:15  leschen
Fix memory test continuously run fail problem.

Revision 1.11.2.1  2016/08/04 02:41:12  leschen
Create new memory tester with ECC checking.

Revision 1.12  2016/05/06 17:44:26  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.11  2016/04/20 07:03:32  benchen2
merge tachi_branch to maintrunk

Revision 1.10.60.2  2016/04/01 03:19:03  benchen2
fix define issue

Revision 1.10.60.1  2015/10/26 13:02:57  benchen2
memory test percentage set to 90% and remove overcommit_ratio

Revision 1.10  2012/06/08 21:33:28  ptong
Remove #if TAKE_ADDR_OFFSET

Revision 1.9  2012/06/06 10:30:15  aarwang
- Clean up compiler warning.

Revision 1.8  2012/06/05 09:33:44  aarwang
- Clean up compiler warnings.

Revision 1.7  2012/05/29 00:07:01  ptong
Correct the OVRHD_FACTOR value

Revision 1.6  2012/05/22 07:31:29  alpeng
Remove cterr from march C test, should put cterr outside of diag

Revision 1.5  2012/05/18 00:27:12  ptong
Increase the memory being tested and report it

Revision 1.4  2012/05/07 08:10:10  alpeng
fixed message, remove prcomplete

Revision 1.3  2012/03/30 21:42:49  ptong
Change OVRHD_FACTOR back to 0.2

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

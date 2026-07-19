/* $Id: linux_memory_tester.c,v 1.2 2015/05/25 03:59:16 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/linux_memory_tester.c,v $
 *------------------------------------------------------------------
 * 
 * linux_memory_tester.c - Porting from Linux API memtester ver 4.2.1.  
 * 
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *------------------------------------------------------------------
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
#include <setjmp.h>
#include <arch/chip.h>
#include <tmc/cpus.h>
#include <tmc/task.h>
#include "common.h" 
#include "types.h" 
#include "defs.h" 
#include "menu.h" 
#include "error.h" 
#include "linux_memory_tester.h" 
#include "linux_memory_tester_utils.h" 
#include "common_utils.h" 
#include "nvmonvars.h" 
#include "queryflags.h"

#ifdef SKYE_ENHANCED_ERR_MSG
#include "platform_fru.h"
#endif   /* SKYE_ENHANCED_ERR_MSG */

/******************************************************************************* 
 *                            Function prototypes 
 *******************************************************************************/ 
void check_posix_system(void); 
int memtester_pagesize(void); 
int test_patterns(int); 
int start_test(int , void volatile *, void volatile *, ulv *, ulv *, int, ull);
extern void skye_dump_env_prm(void);
extern int mem_march_test(MEM_CACH_TYPE, ulong, ulong); 
extern int mtest_all_cores(void);
extern jmp_buf *monjmpptr; 

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
 
#define RESERVE_FREE_MEM (512 * 1024 * 1024)
#define MEM_BLOCK_ALIGN (CHIP_L2_CACHE_SIZE() * 2)
 

#define SHRINKRAY_EVAL 1
/* define test method for memtester */ 
struct test_method tests[] = { 
    { "March C",              mem_march_test               },
    { "All Cores",            mtest_all_cores              }, 
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
    {"All Cores",               (PFT)mtest_all_cores,    1,  
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Random Value",            (PFT)test_patterns,      2,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare XOR",             (PFT)test_patterns,      3,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare SUB",             (PFT)test_patterns,      4,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare MUL",             (PFT)test_patterns,      5,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare DIV",             (PFT)test_patterns,      6,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare OR",              (PFT)test_patterns,      7,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Compare AND",             (PFT)test_patterns,      8,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Sequential Increment",    (PFT)test_patterns,      9,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Solid Bits",              (PFT)test_patterns,      0,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Block Sequential",        (PFT)test_patterns,      11,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Checkerboard",            (PFT)test_patterns,      12,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Bit Spread",              (PFT)test_patterns,      13,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Bit Flip",                (PFT)test_patterns,      14,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Walking Ones",            (PFT)test_patterns,      15,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Walking Zeroes",          (PFT)test_patterns,      16,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"Stuck Address",           (PFT)test_patterns,      17,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
#ifdef TEST_NARROW_WRITES 
    {"8-bit Writes",            (PFT)test_patterns,      18,  
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,  
     (type_t(*)())0, 0,    (type_t(*)())0,       0}, 
    {"16-bit Writes",           (PFT)test_patterns,      19,  
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
int
linux_memory_tester (int exe_march_c_test)
{ 
    build_primary_submenu(memtester_test_table, MEMTESTER_TEST_TABLE_SIZE, 
            "Memory Tester Menu", &memtester_testmenup);
    build_secondary_submenu(memtester_test_table, MEMTESTER_TEST_TABLE_SIZE, 
            memtester_tests_secondary_items);

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_DIMM;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "DDR3 DIMM");
	
    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check DIMM is installed properly.",
                    "Swap DIMM slot.",
                    "Replace DIMM with golden sample.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
	
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
    FILE* minfo;
    unsigned long long free_mem = 0;
    unsigned long long sequestered_mem = 0;
    char next_line[80];
    int count = 36; /* Tiler CPU has 36 cores */
    unsigned long long mem_size_to_test;

    /* CSCul62219:Fixed Memory test no readable memory Segmentation fault issue */
    minfo = fopen("/proc/meminfo", "r");
    if (minfo == NULL)
        tmc_task_die("Could not open /proc/meminfo.");
    
    /* Read one line of meminfo per loop iteration.
       End when hit end of file, or when the free memory line is found. */
    while (fgets(next_line, 80, minfo) != NULL)
    {
        int found = 0;
        if (sscanf(next_line, "MemFree: %lld kB", &free_mem) == 1)
            found++;
        if (sscanf(next_line, "Sequestered: %lld kB", &sequestered_mem) == 1)
            found++;
        if (found == 2)
            break;
    }
    /* If the free memory line was never found, free_mem will still be zero,
       and we'll fail out in the minimum free memory check below. */
    
    /* Convert free memory from kB to bytes. */
    free_mem <<= 10;
    
    /* Convert sequestered memory from kB to bytes. */
    sequestered_mem <<= 10;
    
    free_mem += sequestered_mem;
    
    /* We make sure to leave at least RESERVE_FREE_MEM, or one eighth of
       the available free memory, whichever is larger, unused. */
    if (RESERVE_FREE_MEM > free_mem / 8)
        free_mem -= RESERVE_FREE_MEM;
    else
        free_mem -= free_mem / 8;

    /* Minimum amount of free memory to run a test, based on block alignment. */
    unsigned long long min_free_mem = count * MEM_BLOCK_ALIGN;
  
    if (free_mem < min_free_mem)
        tmc_task_die("Minimum amount of memory to test with %d tiles is "
                   "%llu bytes.", count, min_free_mem);

    mem_size_to_test = (free_mem / count) & -MEM_BLOCK_ALIGN;
    free_mem = mem_size_to_test * count;

    printf("free_mem %dMB  memory\n", (unsigned int)(free_mem/ONE_MEG));
    return (free_mem); 
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
int
test_patterns (int tst_typ)  {
    ul loops, loop; 
    ull pagesize, wantmb, wantbytes, wantbytes_orig,  
      bufsize = 0, halflen, count, mem_size, curr_size, offset; 

    ptrdiff_t pagesizemask; 
    void volatile *buf, *aligned = 0, *end_addr; 
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

    /* Show Skye current enivronmental parameters */
    if (DIAGFLAG & D_VERBOSE) {
        skye_dump_env_prm();
    }

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
              printf("\nBefore: aligned = %llu \n",(long long unsigned int)aligned); 
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
              printf("\nAfter:  aligned = %llu, wantbytes = %llu, bufsize = %llu \n", 
                  (long long unsigned int)aligned, 
                  (long long unsigned int)wantbytes, 
                  (long long unsigned int)bufsize); 
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
        } else {
       	    exit_code = start_test(tst_typ, aligned, end_addr, bufa, bufb, count, bufsize);
        }

        if (do_mlock) {
             munlock((void *) aligned, bufsize); 
        }
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
 
/*******************************************************************************
 * Function   : start_test
 *
 * Description: star testing memory on linux
 *
 * Inputs     : tst_typ - test type
 *              aligned - test align
 *              end_addr - end address
 *              bufa     - buffer address a
 *              bufb     - buffer address b
 *              count    - counter
 *              bufsize  - buffer size
 *
 * Outputs    : exit_code
 *******************************************************************************
 */
int
start_test(int tst_typ, void volatile *aligned, void volatile *end_addr,
               ulv *bufa, ulv *bufb, int count, ull bufsize) {

    int exit_code = 0;
    /*  
     * The parameter for test_stuck_address and mem_march_test  
     * are different with other tests,  
     * so bring it out from others. 
     */
    prpass(testpass," %s: ", tests[tst_typ].name);

    if (tst_typ == SELECT_STUCK_ADDR) { 
        if (!test_stuck_address(aligned, bufsize / sizeof(ul))) {
        } else { 
            exit_code |= EXIT_FAIL_ADDRESSLINES; 
        } 
    } else if (tst_typ == SELECT_MARCH_C) { 
        if(!mem_march_test(LONG_UNCACHE, (ulong)aligned, (ulong)end_addr)) {
        } else { 
        	  exit_code |= EXIT_FAIL_MARCH_C; 
        } 
    } else {   
         /* executes corresponding tests from test menu. */ 
         if (!tests[tst_typ].fp(bufa, bufb, count)) { 
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


/*******************************************************************************
 *
 * Function    : memtest_do_all_wrapper
 * Description : Wrapper for memory test do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int memtest_do_all_wrapper (void)
{
    int rc = PASSED;
    /* avoid to ask user memory size in test_patterns.
     * Select memory size automatically,
     * and process infinite test.
     * For now is only march_C test.
     */
    donot_query = TRUE;
    /* March-C */
    if (test_patterns(0) == FAILED) {
        return (FAILED);
    }
    /* Test All Cores */
    if (mtest_all_cores() == FAILED) {
        return (FAILED);
    }

    return (rc);
}
 
/******** History ********/ 
/*
 *------------------------------------------------------------------
 * $Log: linux_memory_tester.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/04/30 08:33:53  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:33  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.6  2014/10/14 06:31:10  steja
 * Fix appropriate return failed for do all test.
 *
 * Revision 1.1.2.5  2014/09/18 07:22:25  palin2
 * Updated enhanced error message - debugging steps.
 *
 * Revision 1.1.2.4  2014/09/18 07:03:04  palin2
 * Added to show current enivronmental parameters in memory test with VERBOSE flag.
 *
 * Revision 1.1.2.3  2014/09/17 04:35:07  palin2
 * Updated Skye enhanced error message.
 *
 * Revision 1.1.2.2  2014/08/28 02:54:26  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.1  2014/07/21 01:56:53  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:44  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.6  2014/02/07 03:36:52  steja
 * code clean up
 *
 * Revision 1.1.4.5  2013/11/25 09:14:37  iachang
 * CSCul62219:Fixed Memory test no readable memory Segmentation fault issue
 *
 * Revision 1.1.4.4  2013/11/18 07:40:36  iachang
 * Fixed MARCH C memory test get free memory size issue
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:08  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2013/07/30 06:51:13  iachang
 * Support memory test on all cores.
 *
 * Revision 1.1.2.1  2013/04/29 08:25:08  iachang
 * Support memory test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


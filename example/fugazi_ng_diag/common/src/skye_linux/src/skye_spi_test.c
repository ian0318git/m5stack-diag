/* $Id: skye_spi_test.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_spi_test.c,v $
 *------------------------------------------------------------------
 *
 * skye_spi_test.c: SPI ROM Read / Write Test
 *
 * Jan 2014 - Ian Chang
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "queryflags.h" 

#ifdef SKYE_ENHANCED_ERR_MSG
#include "platform_fru.h"
#endif   /* SKYE_ENHANCED_ERR_MSG */

/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************/
int spirom_test(void);
int skye_spirom_util(void);

/*******************************************************************************
 *                                Externs
 *******************************************************************************/
extern int szalinski_spirom_read(unsigned int, unsigned int,unsigned int);
extern int szalinski_spirom_write(unsigned int, unsigned int, unsigned int,
                                  unsigned int);
extern int sr_spirom_read(unsigned int, unsigned int, unsigned int, 
                          unsigned char *);
extern int sr_spirom_write(unsigned int, unsigned int, unsigned int, 
                          unsigned char *);
extern long long timeval_diff(struct timeval *, struct timeval *,
                              struct timeval *);

/******************************************************************************
 *                                 Menus
 ******************************************************************************/ 
/*
 * SPI ROM utilities SubMenu Table
 */
static submenu_xtable_t spi_util_table[] = {
    {"SPI ROM R/W Test",  (PFT)spirom_test,                            TRUE,
     (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Skye SPIROM utility",   (PFT)skye_spirom_util,         TRUE,
      0,                            (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
};

#define spi_util_table_SZ \
        (sizeof(spi_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t spi_util_primary_items[spi_util_table_SZ + MAX_BASE_ITEMS];
static mitem_t spi_util_secondary_items[spi_util_table_SZ + MAX_BASE_ITEMS];

static menuinfo_t spi_util_menu = {
    "%s Utilities SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    spi_util_primary_items,
};

menuinfo_t *spi_util_submenup = &spi_util_menu;


/*******************************************************************************
 *
 * Function   :	access_spirom_test
 * Description:	main test for SPI device test
 * Inputs     :	file path to SPI device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
access_spirom_test (char *src)
{
    unsigned char buf[128], buf_bk[1024], buf_wr[1024], buf_rd[1024];
    uchar *p1 = (uchar *)buf_wr;
    uchar *p2 = (uchar *)buf_rd;
    int ib, ix, retval;
    int cnt = 0;
    unsigned int srom_no = 0, nbytes = 1024;
    static int spi_offset = 60 * 1024;  //no used space
  
    sprintf((char *)buf, "%s", (char *)src);

    memset((unsigned char *)buf_bk, 0, sizeof(buf_bk));
    memset((unsigned char *)buf_wr, 0, sizeof(buf_wr));
    memset((unsigned char *)buf_rd, 0, sizeof(buf_rd));

    if ((retval = sr_spirom_read(srom_no, spi_offset, nbytes, 
        (unsigned char *)&buf_bk)) == -1) {
        perror("Read");
        printf("Unable to read from drive.\n");
        return FAILED;
    }
    for (cnt=0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt]= PATTERN + cnt;
    }

    if ((retval = sr_spirom_write(srom_no, spi_offset, nbytes, buf_wr)) == -1) {
        perror("Write");
        printf("Write test pattern failed, can not write to drive.\n");
        return FAILED;
    }
    if ((retval = sr_spirom_read(srom_no, spi_offset, nbytes, (unsigned char *)
        &buf_rd)) == -1) {
        perror("Read");
        printf("Readback failed, can not read from drive.\n");
        return FAILED;
    }
    if (DIAGFLAG & D_VERBOSE) {
        for (ix = 0; ix < nbytes; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ",spi_offset + ix );
                printf("%2X ", buf_rd[ix]);
        }
    }

    for (ib =0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            if ((retval = sr_spirom_write(srom_no, spi_offset, nbytes, buf_bk))
                 == -1) {
                perror("Write");
                printf("Write test pattern failed, can not write to drive.\n");
                return FAILED;
            }
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x\n",
                  (ib+1), *p1, *p2);
            break;
        }
    }

    if ((retval = sr_spirom_write(srom_no, spi_offset, nbytes, buf_bk)) == -1) {
        perror("Write");
        printf("Write test pattern failed, can not write to drive.\n");
        return FAILED;
    }

    return PASSED;
}
/*******************************************************************************
 *
 * Function   :	spirom_test
 * Description:	entry point to SPI ROM device test
 * Inputs     : None
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int 
spirom_test (void)
{
    char src[32];
    int retval, fd = -1;
    unsigned int srom_no = 0;
    struct timeval t1, t2;
    struct timeval interval;

    testname("SPI ROM Access");
    prpass(testpass, "");
    
#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_SPIROM;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "SPI ROM");
	
    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check SPI ROM write protect.",
                    "Check SPI bus",
                    "Check boot code in which sector. Diag tests last sector.");
#endif   /* SKYE_ENHANCED_ERR_MSG */

    sprintf((char *)src, "/dev/srom/%d", srom_no);
    fd = open(src, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open %s.\n", __FUNCTION__, src);
        return (FAILED);
    }

    /* Start Timer */
    gettimeofday(&t1, NULL);

    retval = access_spirom_test(src);

    /* Stop Timer */
    gettimeofday(&t2, NULL);

    /* compute and print the elapsed time in millisec */
    printf("\nTest Time is %lld microseconds",
         timeval_diff(&interval, &t2, &t1)
    );

    printf(" (%ld seconds, %ld microseconds)\n",
            interval.tv_sec,
            interval.tv_usec
    );
    close(fd); /* Fixed CSCum81295:SPI Test Segmentation fault after 1020 times. */

    return retval;
}
/*******************************************************************************
 *
 * Function   : skye_spirom_util
 * Description: Uility to R/W Skye SPIROM.
 * Input      : None.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_spirom_util (void)
{
    int      choice = 0, retval;
    unsigned int srom_no = 0, nbytes, value;
    uint32_t spi_offset;   
    printf("\nSkye SPIROM utility List:\n");
    printf("1. Read SPIROM Data\n");
    printf("2. Write SPIROM Data\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 2);

    switch (choice) {
    case 1:
        spi_offset = gethex_answer("Please enter offset: ", 0, 1, 0xffffff);
        nbytes = getdec_answer("Please enter read bytes: ", 0, 1, 0xffff);
        retval = szalinski_spirom_read(srom_no, spi_offset, nbytes);
        break;
    case 2:
        spi_offset = getdec_answer("Please enter offset: ", 0, 1, 0xffffff);
        nbytes = getdec_answer("Please enter write bytes: ", 1, 1, 0xffff);
	    value = gethex_answer("\nEnter test pattern:", 0, 0, 0xff);
        retval = szalinski_spirom_write(srom_no, spi_offset, nbytes, value);
        retval = szalinski_spirom_read(srom_no, spi_offset, nbytes);
        break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }
    return (retval);
}
/*******************************************************************************
 *
 * Function   : build_spi_util_menu
 * Description: Function to build Skye SPI ROM utility submenu.
 * Inputs     : num - number of thermal
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int build_spi_util_menu (int num)
{
    char menu_title[32];

    memset(menu_title, 0 , sizeof(menu_title));

    build_primary_submenu(spi_util_table, spi_util_table_SZ,
                          menu_title, &spi_util_submenup);
    build_secondary_submenu(spi_util_table, spi_util_table_SZ,
                            spi_util_secondary_items);

    /* Display Utility Menu */
    menu(spi_util_submenup, spi_util_secondary_items, 0);

    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: skye_spi_test.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 13:45:46  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:36  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.4  2014/09/18 07:22:26  palin2
 * Updated enhanced error message - debugging steps.
 *
 * Revision 1.1.2.3  2014/09/17 04:35:08  palin2
 * Updated Skye enhanced error message.
 *
 * Revision 1.1.2.2  2014/08/31 23:01:52  palin2
 * Added enhanced error message.
 *
 * Revision 1.1.2.1  2014/07/21 01:56:56  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * skye_spi_test.c:
 * Revision 1.2.8.1  2014/05/08 03:43:04  iachang
 * CSCum81295 : Fixed SPI Test Segmentation fault after 1020 times
 *
 * Revision 1.2  2014/02/27 15:01:49  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2014/01/27 17:06:31  iachang
 * Add prpass with SPI ROM test
 *
 * Revision 1.1.2.1  2014/01/27 16:52:46  iachang
 * Support SPI ROM read/write Test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


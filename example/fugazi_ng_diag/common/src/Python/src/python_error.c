/* $Id: python_error.c,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/src/python_error.c,v $
 *------------------------------------------------------------------
 * Description: error reporting mechanism for C executable
 *
 * Oct 2013 - erwu2
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*-----------------------------------------------------------------------------
 * python-specific include
 *---------------------------------------------------------------------------*/
#include "Python.h"
#include "python_error.h"
#include "diag_flag_create_from_py.h"

/*-----------------------------------------------------------------------------
 * SRG environment include
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "common.h"

/*-----------------------------------------------------------------------------
 * python-specific define
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * SRG environment define
 *---------------------------------------------------------------------------*/
#define SCREEN_WIDTH   80
#define ERRLOG_INFO_BUF_SIZE 256

/* define line length in error_count.tmp */
#define ERR_TMP_LEN 50

/* needed to handle largest possible errmsg from lance test */
#define CBUFSIZ 2500

/* define bufsiz for enhanced error message */
#define EN_BUF_SIZE (CBUFSIZ*100)

/* global flag to prevent user from entering cterr again in cterr process */
static unsigned int cterr_entered = FALSE;
static char *warn_cnt_path = "../script/warncnt.tmp";
static char *err_accu_cnt_path = "../script/erraccu.tmp";
static char *stoponerr_path = "../script/stoponerr.tmp";
static int stoponerr(void);

typedef unsigned int uint32;

/* declare cterr_db_print() */
uint32 cterr_db_print(char *fmtptr, ...);

/* Pointer To The Start Of Cterr Reg Information */
cterr_reg_dump_t *first_reg_dump_ptr = NULL;

/* Pointer To The Start Of Cterr Env Information */
cterr_env_dump_t *first_env_dump_ptr = NULL;

/* Pointer To The Start Of Cterr Component Description */
cterr_component_t *first_comp_ptr = NULL;

/* Pointer To The Start Of Cterr Debug Information */
cterr_debug_t *first_debug_ptr = NULL;

extern fru_table_t platform_fru_table[];

/* declare cterr_bptr for global cterr msg */
char *cterr_bptr;

int logprintf (char* );

int dbprint (char *);

/*
 * FRU Table Offset Global
 * Initialize to 0xff, so that code will know if its
 * been set by diagnostic menu.  Anything but 0xff will be
 * valid.
 */
unsigned int fru_table_offset = FRU_INFO_INVALID;

/*
 * serialnumbuf can be filled in by the platform to allow
 * the display of the serial number of the failing board
 * (motherboard, NIM, DFC, WIC, etc., reference: atlantis)
 */
char serialnumbuf[20] = "HASN'T BEEN READ\0";
char testnamebuf[TESTNAMEBUFSIZ];
char test_progress_buf[CBUFSIZ/2];

/* Error counters. */
unsigned long testpass = 0;
unsigned long errcount = 0;
unsigned long err_accum = 0;
unsigned long warncount = 0;


/*-----------------------------------------------------------------------------
 * python-specific functions
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * SRG environment functions
 *---------------------------------------------------------------------------*/
 /*****************************************************************************
 *
 *  Function: ran_num_one_ten
 *
 *  Description: This function return random value from int 1 to 10 for test
 *
 *  Input: None
 *
 *  Returns: random value from 1 to 10
 *
 *****************************************************************************/
int
ran_num_one_ten()
{
    int ran_num = 0;
    srand(time(NULL));
    ran_num=(rand()%10)+1;
    return ran_num;
}

/******************************************************************************
 *
 *  Function: movbyte
 *
 *  Description: set buffer addr0 length and copy to addr1
 *
 *  Input: src addr, length, and dist addr
 *
 *  Returns: none
 *
 *****************************************************************************/
void
movbyte(unsigned char *addr0, unsigned char *addr1, int length)
{
    register unsigned char *end;

    end = (unsigned char *)((unsigned long)addr0 + length);
    while ((ulong)addr0 < (ulong)end) {
      *addr1++ = *addr0++;
    }
}

/******************************************************************************
 *
 *  Function: clrtestname
 *
 *  Description: clear testnamebuf with value 0
 *
 *  Input: none
 *
 *  Returns: none
 *
 *****************************************************************************/
void
clrtestname (void)
{
    memset(testnamebuf, 0, TESTNAMEBUFSIZ);
}

/******************************************************************************
 *
 *  Function: logprintf
 *
 *  Description: to save the error logs in a file(e.g. /errlog.txt)
 *
 *  Input: buffer to save information
 *
 *  Returns: TRUE/FLASE
 *
 *****************************************************************************/
int
logprintf (char *buf)
{
    FILE *fp;
    fp = fopen(ERRLOG, "a");
    if (fp == NULL) {
        printf("Failed to open /errlog.txt file.\n");
        return FALSE;
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);
    return TRUE;
}

/*
 * Function clrerrlog.
 *
 * This function is used to clear the error log.
 */
void
clrerrlog (void)
{
    int ret;
    ret = remove(ERRLOG);
    if (ret != 0) {
        printf("Failed to remove /errlog.txt file.\n");
    }
}

/******************************************************************************
 *
 *  Function: dbprint
 *
 *  Description: to save the dbprint logs in a file(e.g. /dblog.txt)
 *
 *  Input: buffer to save information
 *
 *  Returns: TRUE/FLASE
 *
 *****************************************************************************/
int
dbprint (char *buf)
{
    FILE *fp;
    fp = fopen(DBLOG, "a");
    if (fp == NULL) {
        printf("Failed to open /dblog.txt file.\n");
        return FALSE;
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);
    return TRUE;
}

/*
 * Function clrdblog.
 *
 * This function is used to clear the db buffer log.
 */
void
clrdblog (void)
{
    int ret;
    ret = remove(DBLOG);
    if (ret != 0) {
        printf("Failed to remove /dblog.txt file.\n");
    }
}

/*
 * Function dumperrlog.
 *
 * This function is used to display the error log.
 */
int
dumperrlog (void)
{
    FILE *errlog;
    char log_info[ERRLOG_INFO_BUF_SIZE];

    errlog = fopen(ERRLOG, "r");
    if (errlog == NULL) {
        printf("Failed to open /errlog.txt file.\n");
        return FAILED;
    }

    while (fgets(log_info, ERRLOG_INFO_BUF_SIZE, errlog) != NULL) {
        log_info[strlen(log_info)-1] = '\0';
        puts(log_info);
    }

    fclose(errlog);
    return PASSED;
}

/*
 * Function dumpdblog.
 *
 * This function is used to display the debug buffer log.
 */
int
dumpdblog (void)
{
    FILE *dblog;
    char log_info[ERRLOG_INFO_BUF_SIZE] = {0};

    dblog = fopen(DBLOG, "r");
    if (dblog == NULL) {
        printf("Failed to open /dblog.txt file.\n");
        return FAILED;
    }

    while (fgets(log_info, ERRLOG_INFO_BUF_SIZE, dblog) != NULL) {
        log_info[strlen(log_info)-1] = '\0';
        puts(log_info);
    }

    fclose(dblog);
    return PASSED;
}

/**********************************************************************
 *
 *  Function: reset_errmsg_var()
 *
 *  Description: reset error message variables
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************/
void reset_errmsg_var(void)
{
    /*segment 1*/
    /*
     * if developer use the same fru_table_offset has been assigned pid&loc,
     * but forget to update subtest's pid&loc, it will use previous values.
     * Clear here.
     */
    if (fru_table_offset != FRU_INFO_INVALID) {
        platform_fru_table[fru_table_offset].pid_string = (unsigned char *)"";
        platform_fru_table[fru_table_offset].location_string =
                                                          (unsigned char *)"";
    }
    fru_table_offset = FRU_INFO_INVALID;
    /*segment 4*/
    cterr_clear_component();
    /*segment 5*/
    cterr_clear_reg_dump();
    /*segment 6*/
    cterr_clear_env_dump();
    /*segment 7*/
    cterr_clear_debug();
    /* if send break then come back to prompt, */
    /* set cterr_entered to FALSE to keep cterr_db_print regular */
    cterr_entered = FALSE;
}

/******************************************************************************
 *
 *  Function: cterr_db_print
 *
 *  Description: if call cterr and cterr_entered flag on, print msg to buffer;
 *               otherwise, print as printf()
 *
 *  Input: parameters as printf
 *
 *  Returns: PASSED/FAILED
 *
 *****************************************************************************/
uint32
cterr_db_print (char *fmtptr, ...)
{
    va_list ap;
    va_start(ap, fmtptr);

    switch (cterr_entered) {
        case TRUE:
            /*add tab with msg in db buf*/
            cterr_bptr += vsprintf(cterr_bptr, "\t", ap);
            /* Call to print to log_buf */
            cterr_bptr += vsprintf(cterr_bptr, fmtptr, ap);
            break;
        default:
            /* function as normal printf */
            vprintf(fmtptr, ap);
            break;
        }
    va_end(ap);
    return PASSED;
}

/*
 * save_testprogress (char *buffer)
 * Save the info of the prpass as well as testname before the test failed
 * This info will be logged when cterr() is executed.
 */
void
save_testprogress (char *buffer)
{
    movbyte((unsigned char *)buffer, (unsigned char *)test_progress_buf,
            (CBUFSIZ/2));
}

/*
 * Function prcomplete.
 *
 * This function is used by diagnostics to indicate that the test has
 * completed.
 */
void
prcomplete(int pass, int errcount, char *msg, ...)
{
    if (msg) {
        printf("\n");
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
    }
    printf("\n errors = %d , warnings = %d\n",
              (int)err_accum,(int)warncount);
    fflush(stdout);
}

 /*****************************************************************************
 *
 *  Function: testname
 *
 *  Description: Each test which had been run in python menu should be
 *               given a test name.This function will save the name
 *               so that it can be displayed to the user if the test fail.
 *
 *  Input: argc and argv[] of each test executable
 *
 *  Out  : None
 *
 *****************************************************************************/
void
testname(char *string, ...)
{
    char buffer[TESTNAMEBUFSIZ];

    strcpy(testnamebuf, string);

    if (string) {
        va_list args;
        va_start(args, string);
        vsprintf(buffer, string, args);
        va_end(args);
    }

    if ((continuous == 1) && testpass) {
        sprintf(testnamebuf, "%s test passes %ld", buffer, testpass);
    } else {
        sprintf(testnamebuf, "%s test ", buffer);
    }

}


/******************************************************************************
 * Function : prpass
 *
 * Description:This function is used by diagnostics to indicate progress.
 *
 * Input: pass - testpass counter
 *        msg - message of progress
 *
 * Out: None
 *
 *****************************************************************************/
void prpass(int pass, char *msg, ...)
{
    char *bptr, buffer[SCREEN_WIDTH];
    char en_buf[CBUFSIZ/2]={0};
    va_list args;
    va_list prargs;
    va_start(args, msg);
    va_start(prargs, msg);

    /* Clear current line */
    fflush(stdout);
    printf(" ");
    printf("\r");
    memset(buffer, 0x20, sizeof(buffer)); /* ascii fow white space */
    buffer[SCREEN_WIDTH-1] = 0; /* NULL char */
    printf(buffer);
    printf("\r");

    if (fru_table_offset != FRU_INFO_INVALID) {

        /* enhanced error message */
        bptr = en_buf;
        if (pass) {
            bptr += sprintf(bptr, "pass %d, ", pass);
            if (!(external_customer == 1)) {
                printf("%s",en_buf);
            }
        }
        /* save test progress(prpass and testname) for cterr() segment 2 */
        if (msg) {
            bptr += vsprintf(bptr, msg, args);
        }
        sprintf(bptr, " %s", testnamebuf);
        save_testprogress(en_buf);

        /*
         * Determine if External Customer is set.
         * If it is, do nothing.
         */
        if (!(external_customer == 1)) {
            /* print prpass and testname to screen */
            if (msg) {
                vprintf(msg, prargs);
            }
            sprintf(buffer, "%s", testnamebuf);
            printf("%s",buffer);
        }

    } else {

        bptr = buffer;
        if (pass) {
            bptr += sprintf(bptr, "pass %d, ", pass);
            printf(buffer);
        }

        if (msg) {
            vprintf(msg, args);
        }
        sprintf(buffer, "%s", testnamebuf);
        printf(buffer);
    }
    va_end(args);
    va_end(prargs);
    fflush(stdout);
}

/******************************************************************************
 * Function : inthdlr
 *
 * Description : this function interrupt and exit c executable
 *               when receiving ctrl c cmd
 *
 * Input: system interrupt signal type
 *
 * Out: None
 *
 *****************************************************************************/
void
inthdlr(int sig)
{
     /* SIG_IGN : signal is ignored */
     signal(sig, SIG_IGN);
     exit(0);
}

/*
 * Function cterr.
 *
 * Failure messages for diagnostics should use this function.
 */
void cterr(char errtype, int errnum, char *errstr, ...)
{
    FILE *fp, *ferr_cnt,*fwarn_cnt, *ferr_accu , *fstp_err;
    char err_cnt_buf[EN_BUF_SIZE] = {0};
    char err_accu_buf[EN_BUF_SIZE] = {0};
    char warn_cnt_buf[EN_BUF_SIZE] = {0};
    time_t clk = time(NULL);

    if (fru_table_offset != FRU_INFO_INVALID) {
        char db_buffer[EN_BUF_SIZE] = {0};
        char log_buffer[EN_BUF_SIZE] = {0};
        /* prompt_buf is used for adding prompt message to errlog buffer
         * if reg dump function inside segment 5 */
        char prompt_buf[EN_BUF_SIZE] = {0};
        unsigned char *bptr_for_pnt_log_buf[3];
        /* bptr_for_pnt_log_buf[0] & [2] are used for logging segment 1~4
         * [1]is used for logging segment 6~7 */
        int cur_offset = 0;

        va_list ap;
        uint comp_name_index;
        cterr_component_t *comp_ptr;
        uint debug_info_index;
        cterr_debug_t *debug_ptr;
        uint reg_dump_index;
        cterr_reg_dump_t *reg_ptr;
        uint env_dump_index;
        cterr_env_dump_t *env_ptr;

        if (cterr_entered == TRUE) {
            /* cterr reentry */
            va_start(ap, errstr);
            /*add tab with msg in db buf*/
            cterr_bptr += vsprintf(cterr_bptr, "\t", ap);
            /* Call to print to log_buf */
            cterr_bptr += vsprintf(cterr_bptr, errstr, ap);
            va_end(ap);
            cterr_db_print("\n\t### cterr reentry! \n");
        } else {
            /*nornal calling cterr*/
            cterr_entered = TRUE;

            ferr_accu = fopen(err_accu_cnt_path,"r");
            if (ferr_accu) {
                fgets(err_accu_buf, ERR_TMP_LEN, ferr_accu);
                /* 10 for Decimal */
                err_accum = strtoul(err_accu_buf,NULL,10);
                fclose(ferr_accu);
            } else {
                printf("%s: Open '%s' Failed\n", __FUNCTION__,err_accu_cnt_path);
            }

            fwarn_cnt = fopen(warn_cnt_path,"r");
            if (fwarn_cnt) {
                fgets(warn_cnt_buf, ERR_TMP_LEN, fwarn_cnt);
                /* 10 for Decimal */
                warncount = strtoul(warn_cnt_buf,NULL,10);
                fclose(fwarn_cnt);
            } else {
                printf("%s: Open '%s' Failed\n", __FUNCTION__,warn_cnt_path);
            }

            cterr_bptr = (char *)db_buffer;
            /* bptr_for_pnt_log_buf[0] and [2] are used for
             * logging segment 1~4 */
            bptr_for_pnt_log_buf[0] = (unsigned char *)log_buffer;
            bptr_for_pnt_log_buf[2] = (unsigned char *)cterr_bptr;

            /* Print out time stamp for error message*/
            cterr_bptr += sprintf(cterr_bptr,"\n========%s\n",ctime(&clk));

            /* Print out the FRU info */
            /* Segment 1 */
            /* PID | Unique_string : slot_info */
            cterr_bptr += sprintf(cterr_bptr,
                        "\n1.PID | Unique_string : slot_info. \n\t%s:%s\n",
                        platform_fru_table[fru_table_offset].pid_string,
                        platform_fru_table[fru_table_offset].location_string);

            /* Segment 2 */
            /* Test Step Failed */
            cterr_bptr += sprintf(cterr_bptr,"\n\n2.Test Step Failed:\n*** ");

            switch (errtype) {

                case 'f':
                    errcount++;
                    err_accum++;
                    cterr_bptr += sprintf(cterr_bptr,"Fatal error #%lu",err_accum);
                    break;
                case 'w':
                    warncount++;
                    cterr_bptr += sprintf(cterr_bptr,"Warning: ");
                    break;
                case 'a':
                    errcount++;
                    err_accum++;
                    cterr_bptr += sprintf(cterr_bptr,"Test Abort: ");
                    break;
                default:
                    errcount++;
                    err_accum++;
                    cterr_bptr += sprintf(cterr_bptr,"Fatal error: ");
                    break;
            }

            cterr_bptr += sprintf(cterr_bptr," during %s",testnamebuf);

            *cterr_bptr++ = '\n';
            if (strcmp(serialnumbuf, "HASN'T BEEN READ")) {
                cterr_bptr += sprintf(cterr_bptr,"SN# %s", serialnumbuf);
                *cterr_bptr++ = '\n';
            }
            cterr_bptr += sprintf(cterr_bptr,
                                  "\nTestFailed:\n\t%s",test_progress_buf);

            /* Segment 3 */
            /* Failure Message: */
            cterr_bptr += sprintf(cterr_bptr, "\n\n\n3.Failure Message:\n\t");
            va_start(ap, errstr);
            cterr_bptr += vsprintf(cterr_bptr, errstr, ap);
            va_end(ap);

            /* Segment 4 */
            /* Components Used: */
            cterr_bptr += sprintf(cterr_bptr, "\n\n\n4.Components Used:");
            if (first_comp_ptr != NULL) {
                comp_ptr = first_comp_ptr;
                comp_name_index = 0;
                while (1) {
                    cterr_bptr += sprintf(cterr_bptr, "\n\t%c.  %s",
                                                      ('a' + comp_name_index),
                                                      comp_ptr->comp_name);
                    if (comp_ptr->next == NULL) {
                        break;
                    }
                    comp_ptr = comp_ptr->next;
                    comp_name_index++;
                }
            } else {
                cterr_bptr += sprintf(cterr_bptr, "\nN/A");
            }
            cterr_bptr += sprintf(cterr_bptr, "\n");

            /* Segment 5 */
            /* Register and Memory Dumps: */
            cterr_bptr += sprintf(cterr_bptr, "\n\n"
                                         "5.Register and Memory Dumps:\n");
            /* copy content of [2] ,segment 1~4, to [0] */
            strcpy((char *)bptr_for_pnt_log_buf[0],
                   (char *)bptr_for_pnt_log_buf[2]);

            if (first_reg_dump_ptr != NULL) {
                reg_ptr = first_reg_dump_ptr;
                reg_dump_index = 0;
                while (1) {
                    cterr_bptr += sprintf(cterr_bptr, "\n\t%c.\n",
                                            ('a' + reg_dump_index));
                    if (reg_ptr->reg_dump_func == NULL) {
                        break;
                    }

                    reg_ptr->reg_dump_func();

                    if (reg_ptr->next == NULL) {
                        break;
                    }
                    reg_ptr = reg_ptr->next;
                    reg_dump_index++;
                }
                /* prompt_buf is used for adding prompt to errlog buffer */
                strcpy(prompt_buf ,"\tUse showdebug CLI command"
                                   " for detailed dump");
                /* bptr_for_pnt_log_buf[1] is used for logging segment 6~7 */
                bptr_for_pnt_log_buf[1] = (unsigned char *)cterr_bptr;

            } else {
                /* bptr_for_pnt_log_buf[1] is used for logging segment 6~7 */
                bptr_for_pnt_log_buf[1] = (unsigned char *)cterr_bptr;
                cterr_bptr += sprintf(cterr_bptr, "N/A");
            }

            cterr_bptr += sprintf(cterr_bptr, "\n");

            /* Segment 6 */
            /* Platform Environment: */
            cterr_bptr += sprintf(cterr_bptr, "\n\n6.Platform Environment:\n");
            if (first_env_dump_ptr != NULL) {
                env_ptr = first_env_dump_ptr;
                env_dump_index = 0;
                while (1) {
                    cterr_bptr += sprintf(cterr_bptr, "\n\t%c.\n",
                                         ('a' + env_dump_index));
                    if (env_ptr->env_dump_func == NULL) {
                        break;
                    }

                    env_ptr->env_dump_func();

                    if (env_ptr->next == NULL) {
                        break;
                    }
                    env_ptr = env_ptr->next;
                    env_dump_index++;

                }
            } else {
                cterr_bptr += sprintf(cterr_bptr, "N/A");
            }

            /* Segment 7 */
            /* Top 3 Debugging Steps*/
            cterr_bptr += sprintf(cterr_bptr, "\n\n7.Debugging Hints:");
            if (first_debug_ptr != NULL) {
                debug_ptr = first_debug_ptr;
                debug_info_index = 0;
                while (1) {
                    cterr_bptr += sprintf(cterr_bptr, "\n\t%c.  %s",
                                                  ('a' + debug_info_index),
                                                  debug_ptr->debug_comment);
                    if (debug_ptr->next == NULL) {
                        break;
                    }
                    debug_ptr = debug_ptr->next;
                    debug_info_index++;
                }

            } else {
                cterr_bptr += sprintf(cterr_bptr, "\nN/A");
            }

            cterr_bptr += sprintf(cterr_bptr, "\n");

            cterr_bptr += sprintf(cterr_bptr, "\nEndOfErrorMessage\n");
            *cterr_bptr++ = '\n';
            *cterr_bptr++ = '\0';

            if (external_customer == 1) {
                /* Print out the FRU info */
                printf("\n%s:%s ...FAILED\n",
                platform_fru_table[fru_table_offset].pid_string,
                platform_fru_table[fru_table_offset].location_string);

            } else {
                /*
                 * if reg dump functions insided segment 5, using prompt_buf
                 * to log the prompt message and bptr_for_pnt_log_buf[1].
                 * in other case prompt_buf equals to bptr_for_pnt_log_buf[1]
                 */
                strcat((char *)prompt_buf,(char *)bptr_for_pnt_log_buf[1]);
                /* combine segment 1~4 and 6~7 for log_buffer */
                strcat((char *)bptr_for_pnt_log_buf[0],(char *)prompt_buf);
                /* display the message in log_buffer to screen */
                puts(log_buffer);
                /*put log_buffer to txt file*/
                if (FALSE == logprintf(log_buffer)) {
                    printf("%s: logprintf Failed\n", __FUNCTION__);
                }
            }

            /* Log into the Debug Buffer */
            if (FALSE == dbprint(db_buffer)) {
                printf("%s dbprint Failed\n",__FUNCTION__);
            }
            cterr_entered = FALSE;
        }

    } else {
        /* old error message format*/
        char log_buffer[ERRLOG_INFO_BUF_SIZE] = {0};
        char *bptr;
        bptr = (char *)log_buffer;

        ferr_accu = fopen(err_accu_cnt_path,"r");
        if (ferr_accu) {
            fgets(err_accu_buf, ERR_TMP_LEN, ferr_accu);
            /* 10 for Decimal */
            err_accum = strtoul(err_accu_buf,NULL,10);
            fclose(ferr_accu);
        } else {
            printf("%s: Open '%s' Failed\n", __FUNCTION__,err_accu_cnt_path);
        }


        fwarn_cnt = fopen(warn_cnt_path,"r");
        if (fwarn_cnt) {
            fgets(warn_cnt_buf, ERR_TMP_LEN, fwarn_cnt);
            /* 10 for Decimal */
            warncount = strtoul(warn_cnt_buf,NULL,10);
            fclose(fwarn_cnt);
        } else {
            printf("%s: Open '%s' Failed\n", __FUNCTION__,warn_cnt_path);
        }

        /* Print out time stamp for error message*/
        printf("\n========%s\n",ctime(&clk));
        printf("\n *** ");

        switch(errtype) {
        case 'f':
            errcount++;
            err_accum++;
            printf("Fatal error: ");
            break;
        case 'w':
            warncount++;
            printf("Warning: ");
            break;
        case 'a':
            errcount++;
            err_accum++;
            printf("Test Abort: ");
            break;
        default:
            errcount++;
            err_accum++;
            printf("Fatal error: ");
            break;
        }

        if (errstr) {
            va_list args;
            va_start(args, errstr);
            vprintf(errstr, args);
            va_end(args);

            /*For error log*/
            va_list args1;
            va_start(args1, errstr);
            bptr += sprintf(bptr,"\n========%s\n",ctime(&clk));
            bptr += vsprintf(bptr, errstr, args1);
            va_end(args1);

            logprintf(log_buffer);
        }
        printf(" ");
        printf(testnamebuf,errnum);
        printf("\n");

    }

    /* write err accu number to ../script/erraccu.tmp*/
    ferr_accu = fopen(err_accu_cnt_path, "w+");
    if (ferr_accu) {
        fprintf(ferr_accu, "%lu", err_accum);
        fclose(ferr_accu);
    } else {
        printf("%s: Open '%s' Failed\n", __FUNCTION__,err_accu_cnt_path);
    }


    /* write warn cnt number to ../script/warncnt.tmp*/
    fwarn_cnt = fopen(warn_cnt_path, "w+");
    if (fwarn_cnt) {
        fprintf(fwarn_cnt, "%lu", warncount);
        fclose(fwarn_cnt);
    } else {
        printf("%s: Open '%s' Failed\n", __FUNCTION__,warn_cnt_path);
    }


    /* If "stop on error" FLAG is set then stop test execution. */
    if (warning == 1) {
        switch (errtype) {
        case 'f':
            if (stop_on_error == 1) {
                /* exit the executable */
                fstp_err = fopen(stoponerr_path, "w+");
                if (fstp_err) {
                    fprintf(fstp_err, "%lu", warncount);
                    fclose(fstp_err);
                } else {
                    printf("%s: Open '%s' Failed\n", __FUNCTION__,stoponerr_path);
                }
                exit(0);
            }
            break;
        case 'w':
            warncount++;
            break;
        default:
            break;
        }
    } else {
        if (stop_on_error == 1) {
            /* exit the executable */
            fstp_err = fopen(stoponerr_path, "w+");
            if (fstp_err) {
                fclose(fstp_err);
            } else {
                printf("%s: Open '%s' Failed\n", __FUNCTION__,stoponerr_path);
            }
            exit(0);
        }
    }
}

/**************************************************************************
 *
 * Name: cterr_add_component_exec
 *
 * Description:  This routine is used to add a component description to
 *               a list for printing during a cterr
 *
 * Inputs: comp_str - Pointer to a component string.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_add_component_exec (char *fmtptr,...)
 {
    va_list ap;
    char *string_mem;
    int com_cnt = 0;
    cterr_component_t *cur_comp_ptr, *tmp_ptr;

    /* clear component linked list at first*/
    cterr_clear_component();

    va_start(ap, fmtptr);

    while(1){

        if (com_cnt == 0) {
            string_mem = fmtptr;
        } else {
            string_mem = va_arg(ap, char*);
        }
        if (!(strcmp(string_mem,END_LIST))) {
            break;
        }

        /* Allocate memory for the linked list element storage */
        cur_comp_ptr = malloc(sizeof(cterr_component_t));
        if (cur_comp_ptr == NULL) {
            cterr('f', 0,"Unable to allocate memory for"
                         " component linked list element");
            va_end(ap);
            return (FAILED);
        }

        /* Allocate memory for the string linked list storage */
        cur_comp_ptr->comp_name = malloc(strlen(string_mem)+1);
        if (cur_comp_ptr->comp_name == NULL) {
            cterr('f', 0,"Unable to allocate memory for component string");
            va_end(ap);
            if (cur_comp_ptr != NULL) {
                free(cur_comp_ptr);
                cur_comp_ptr = NULL;
            }
            return (FAILED);
        }

        /* Copy input string to memory pointed by string_mem */
        strcpy((char *)cur_comp_ptr->comp_name, string_mem);
        cur_comp_ptr->next = NULL;

        /*
         * Set first_comp_ptr to start of linked list
         * or add another link to the list
         */
        if (first_comp_ptr == NULL) {
            first_comp_ptr = cur_comp_ptr;
        } else {
            tmp_ptr = first_comp_ptr;
            while (tmp_ptr->next != NULL) {
                tmp_ptr = tmp_ptr->next;
            }
            tmp_ptr->next = cur_comp_ptr;
        }
        com_cnt++;
    }
    va_end(ap);
    return (PASSED);
}

/**************************************************************************
 *
 * Name: cterr_clear_component
 *
 * Description:  This routine is used clear the linked list of component
 *               names and free all allocated memory. After calling this
 *               function, first_comp_ptr will reinit to NULL.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_clear_component (void)
 {
    cterr_component_t *ll_ptr, *prev_ll_ptr;

    /* Exit, if list is uninitialized */
    if (first_comp_ptr == NULL) {
        return (PASSED);
    }

    /*
     * Find the last link of the list and free the memory
     * Do this until the whole list is freed.
     */
    while (first_comp_ptr != NULL) {
        ll_ptr = first_comp_ptr;
        prev_ll_ptr = NULL;

        /*
         * Find the end of the linked list of component names
         * and free up the memory for that link
         */
        while (1) {
            if (ll_ptr->next == NULL) {
                if (ll_ptr->comp_name != NULL) {
                    free(ll_ptr->comp_name);
                    ll_ptr->comp_name = NULL;
                }
                if (prev_ll_ptr == NULL) {
                    free(first_comp_ptr);
                    first_comp_ptr = NULL;
                } else {
                    free(prev_ll_ptr->next);
                    prev_ll_ptr->next = NULL;
                }
                break;
            } else {
                prev_ll_ptr = ll_ptr;
                ll_ptr = prev_ll_ptr->next;
            }
        }
    }

    return (PASSED);
}

/**************************************************************************
 *
 * Name: cterr_add_debug_exec
 *
 * Description:  This routine is used to add a debug information to
 *               a list for printing during a cterr
 *
 * Inputs: debug_info_str - Pointer to a debug info string.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_add_debug_exec (char *fmtptr,...)
 {
    va_list ap;
    char* string_mem;
    int dbg_cnt = 0;
    cterr_debug_t *cur_debug_ptr, *tmp_ptr;

    /* clear debug linked list at first*/
    cterr_clear_debug();

    va_start(ap, fmtptr);

    while(1)
    {
        if (dbg_cnt == 0) {
            string_mem = fmtptr;
        } else {
            string_mem = va_arg(ap, char*);
        }

        if (!(strcmp(string_mem,END_LIST))) {
            break;
        }

        /* Allocate memory for the linked list element storage */
        cur_debug_ptr = malloc(sizeof(cterr_debug_t));
        if (cur_debug_ptr == NULL) {
            cterr('f', 0,"Unable to allocate memory for"
                         " debug linked list element");
            va_end(ap);
            return (FAILED);
        }

        /* Allocate memory for the string linked list storage */
        cur_debug_ptr->debug_comment = malloc(strlen(string_mem)+1);
        if (cur_debug_ptr->debug_comment == NULL) {
            cterr('f', 0, "Unable to allocate memory for debug info string");
            va_end(ap);
            if (cur_debug_ptr != NULL) {
                free(cur_debug_ptr);
                cur_debug_ptr = NULL;
            }
            return (FAILED);
        }

        /* Copy input string to memory pointed by string_mem */
        strcpy((char *)cur_debug_ptr->debug_comment, string_mem);
        cur_debug_ptr->next = NULL;

        /*
         * Set first_comp_ptr to start of linked list
         * or add another link to the list
         */
        if (first_debug_ptr == NULL) {
            first_debug_ptr = cur_debug_ptr;
        } else {
            tmp_ptr = first_debug_ptr;
            while (tmp_ptr->next != NULL) {
                tmp_ptr = tmp_ptr->next;
            }
            tmp_ptr->next = cur_debug_ptr;
        }
        dbg_cnt++;
    }
    va_end(ap);
    return (PASSED);
}

/**************************************************************************
 *
 * Name: cterr_clear_debug
 *
 * Description:  This routine is used clear the linked list of component
 *               names and free all allocated memory. After calling this
 *               function, first_debug_ptr will reinit to NULL.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_clear_debug (void)
 {
    cterr_debug_t *ll_ptr, *prev_ll_ptr;

    /* Exit, if list is uninitialized */
    if (first_debug_ptr == NULL) {
        return (PASSED);
    }

    /*
     * Find the last link of the list and free the memory
     * Do this until the whole list is freed.
     */
    while (first_debug_ptr != NULL) {
        ll_ptr = first_debug_ptr;
        prev_ll_ptr = NULL;

        /*
         * Find the end of the linked list of component names
         * and free up the memory for that link
         */
        while (1) {
            if (ll_ptr->next == NULL) {
                if (ll_ptr->debug_comment != NULL) {
                    free(ll_ptr->debug_comment);
                    ll_ptr->debug_comment = NULL;
                }
                if (prev_ll_ptr == NULL) {
                    free(first_debug_ptr);
                    first_debug_ptr = NULL;
                } else {
                    free(prev_ll_ptr->next);
                    prev_ll_ptr->next = NULL;
                }
                break;
            } else {
                prev_ll_ptr = ll_ptr;
                ll_ptr = prev_ll_ptr->next;
            }
        }
    }

    return (PASSED);
}

/**************************************************************************
 *
 * Name: cterr_add_reg_dump_exec
 *
 * Description:  This routine is used to add some reg dump funcitons to
 *               a list for executing during cterr
 *
 * Inputs: func_ptr - Pointer to a register dump function.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_add_reg_dump_exec (void (*func_ptr)(), ...)
 {
    va_list ap;
    void (*func_mem)();
    int reg_cnt = 0;
    cterr_reg_dump_t *cur_reg_dump_ptr=NULL, *tmp_ptr;

    /* clear reg dump linked list at first*/
    cterr_clear_reg_dump();

    va_start(ap, func_ptr);

    while(1){
        if (reg_cnt == 0) {
            func_mem = func_ptr;
        } else {
            func_mem = va_arg(ap, void *);
        }
        if (!(strcmp((char *)(func_mem),END_LIST))) {
            break;
        }

        /* Allocate memory for the linked list element storage */
        cur_reg_dump_ptr = malloc(sizeof(cterr_reg_dump_t));
        if (cur_reg_dump_ptr == NULL) {
            cterr('f', 0, "Unable to allocate memory for"
                          " reg dump linked list element");
            va_end(ap);
            return (FAILED);
        }
        memset((char *)cur_reg_dump_ptr, 0, sizeof(cterr_reg_dump_t));

        /* Copy component function to linked list */
        cur_reg_dump_ptr->reg_dump_func = func_mem;
        cur_reg_dump_ptr->next = NULL;

        /*
         * Set first_reg_dump_ptr to start of linked list
         * or add another link to the list
         */
        if (first_reg_dump_ptr == NULL) {
            first_reg_dump_ptr = cur_reg_dump_ptr;
        } else {
            tmp_ptr = first_reg_dump_ptr;
            while (tmp_ptr->next != NULL) {
                tmp_ptr = tmp_ptr->next;
            }
            tmp_ptr->next = cur_reg_dump_ptr;
        }
        reg_cnt++;
    }
    va_end(ap);
    return (PASSED);
}
/**************************************************************************
 *
 * Name: cterr_clear_reg_dump
 *
 * Description:  This routine is used clear the linked list of reg dump
 *               function pointer names and free all allocated memory.
 *               After calling this function, first_reg_dump_ptr will
 *               reinit to NULL.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_clear_reg_dump (void)
 {
    cterr_reg_dump_t *ll_ptr, *prev_ll_ptr;

    /* Exit, if list is uninitialized */
    if (first_reg_dump_ptr == NULL) {
        return (PASSED);
    }

    /*
     * Find the last link of the list and free the memory
     * Do this until the whole list is freed.
     */
    while (first_reg_dump_ptr != NULL) {
        ll_ptr = first_reg_dump_ptr;
        prev_ll_ptr = NULL;

        /*
         * Find the end of the linked list of component names
         * and free up the memory for that link
         */
        while (1) {
            if (ll_ptr->next == NULL) {
                if (ll_ptr->reg_dump_func != NULL) {
                    ll_ptr->reg_dump_func = NULL;
                }
                if (prev_ll_ptr == NULL) {
                    free(first_reg_dump_ptr);
                    first_reg_dump_ptr = NULL;
                } else {
                    free(prev_ll_ptr->next);
                    prev_ll_ptr->next = NULL;
                }
                break;
            } else {
                prev_ll_ptr = ll_ptr;
                ll_ptr = prev_ll_ptr->next;
            }
        }
    }
    return (PASSED);
}

/**************************************************************************
 *
 * Name: cterr_add_env_dump_exec
 *
 * Description:  This routine is used to add some env dump funcitons to
 *                    a va list for executing during cterr
 *
 * Inputs: func_ptr - Function Pointer to a environment dump function.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_add_env_dump_exec (void(*func_ptr)(), ...)
 {
    va_list ap;
    void (*func_mem)();
    int env_cnt = 0;
    cterr_env_dump_t *cur_env_dump_ptr=NULL, *tmp_ptr;

    /* clear env dump linked list at first*/
    cterr_clear_env_dump();

    va_start(ap, func_ptr);

    while(1){
        if (env_cnt == 0) {
            func_mem = func_ptr;
        } else {
            func_mem = va_arg(ap, void *);
        }
        if (!(strcmp((char *)(func_mem),END_LIST))) {
            break;
        }

        /* Allocate memory for the linked list element storage */
        cur_env_dump_ptr = malloc(sizeof(cterr_env_dump_t));
        if (cur_env_dump_ptr == NULL) {
            cterr('f', 0, "Unable to allocate memory for"
                          " env dump linked list element");
            va_end(ap);
            return (FAILED);
        }
        memset((char *)cur_env_dump_ptr, 0, sizeof(cterr_env_dump_t));

        /* Copy component function to linked list */
        cur_env_dump_ptr->env_dump_func = func_mem;
        cur_env_dump_ptr->next = NULL;

        /*
         * Set first_env_dump_ptr to start of linked list
         * or add another link to the list
         */
        if (first_env_dump_ptr == NULL) {
            first_env_dump_ptr = cur_env_dump_ptr;
        } else {
            tmp_ptr = first_env_dump_ptr;
            while (tmp_ptr->next != NULL) {
                tmp_ptr = tmp_ptr->next;
            }
            tmp_ptr->next = cur_env_dump_ptr;
        }
        env_cnt++;
    }
    va_end(ap);
    return (PASSED);
}
/**************************************************************************
 *
 * Name: cterr_clear_env_dump
 *
 * Description:  This routine is used clear the linked list of env dump
 *               function pointer names and free all allocated memory.
 *               After calling this function, first_env_dump_ptr will
 *               reinit to NULL.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
 int cterr_clear_env_dump (void)
 {
    cterr_env_dump_t *ll_ptr, *prev_ll_ptr;

    /* Exit, if list is uninitialized */
    if (first_env_dump_ptr == NULL) {
        return (PASSED);
    }

    /*
     * Find the last link of the list and free the memory
     * Do this until the whole list is freed.
     */
    while (first_env_dump_ptr != NULL) {
        ll_ptr = first_env_dump_ptr;
        prev_ll_ptr = NULL;

        /*
         * Find the end of the linked list of component names
         * and free up the memory for that link
         */
        while (1) {
            if (ll_ptr->next == NULL) {
                if (ll_ptr->env_dump_func != NULL) {
                    ll_ptr->env_dump_func = NULL;
                }
                if (prev_ll_ptr == NULL) {
                    free(first_env_dump_ptr);
                    first_env_dump_ptr = NULL;
                } else {
                    free(prev_ll_ptr->next);
                    prev_ll_ptr->next = NULL;
                }
                break;
            } else {
                prev_ll_ptr = ll_ptr;
                ll_ptr = prev_ll_ptr->next;
            }
        }
    }
    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: python_error.c,v $
 * Revision 1.2  2014/06/03 10:53:28  erwu2
 * python menu collapsed to main trunk
 *
 * Revision 1.1.2.7  2014/04/29 11:40:36  erwu2
 * update python file structure
 *
 * Revision 1.1.2.6  2014/04/24 08:53:51  erwu2
 * merge makefile and add flag example to test
 *
 * Revision 1.1.2.5  2014/01/27 11:32:57  erwu2
 * improve print out message and add description to py files
 *
 * Revision 1.1.2.4  2014/01/21 10:45:19  erwu2
 * improve executable and submenu column definition
 *
 * Revision 1.1.2.3  2014/01/16 11:15:55  erwu2
 * update python files
 *
 * Revision 1.1.2.2  2013/12/19 10:25:19  erwu2
 * improve tftp dnld process
 *
 * Revision 1.1.2.1  2013/12/09 06:20:31  erwu2
 * python menu for o2 example
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


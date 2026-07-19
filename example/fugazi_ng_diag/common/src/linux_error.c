/* $Id: linux_error.c,v 1.19 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_error.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "menu.h"
#include "nvsysvars.h"
#include "term.h"
#include "common.h"
#include "dev_object.h"
#include "setjmps.h"
#include "error.h"
#include "proto.h"

extern void longjmp(jmp_buf env, int val);

void logprintf (char* );

void dbprint (char *);

#define ERRLOG_INFO_BUF_SIZE 256

/* Needed to handle largest possible errmsg from lance test */
#define CBUFSIZ 2500

/* define bufsiz for enhanced error message */
#define EN_BUF_SIZE (CBUFSIZ*100)

/* declare cterr_bptr for global cterr msg*/
char *cterr_bptr;

/* declare cterr_db_print()*/
uint32 cterr_db_print(char *fmtptr, ...);

/* global flag to prevent user from entering cterr again in cterr process */
static unsigned int cterr_entered = FALSE;

/* Pointer To The Start Of Cterr Reg Information*/
cterr_reg_dump_t *first_reg_dump_ptr = NULL;

/* Pointer To The Start Of Cterr Env Information*/
cterr_env_dump_t *first_env_dump_ptr = NULL;

/* Pointer To The Start Of Cterr Component Description */
cterr_component_t *first_comp_ptr = NULL;

/* Pointer To The Start Of Cterr Debug Information*/
cterr_debug_t *first_debug_ptr = NULL;

extern fru_table_t platform_fru_table[];

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

struct nvram nvram;  /* may need to map to actual NVRAM physical address */
unsigned long diagflag_xram = D_MIN_TEST_TIME;   /* ram global for additional diag flags */

extern jmp_buf *monjmpptr;

/* Buffer that will hold the name of the test been performed. */
char testnamebuf[TESTNAMEBUFSIZ];
char test_progress_buf[CBUFSIZ/2];

/* Error counters. */
unsigned long testpass = 0;
unsigned long errcount = 0;
unsigned long err_accum = 0;
unsigned long warncount = 0;
static int stoponerr(void);

const char *gettestname()
{
    return testnamebuf;
}

/*
 * Function testname.
 *
 * Each test been run in a diagnostics environment should be given a test name.
 * This function will save the name of the test so that it can be displayed to
 * the user if the test should fail.
 */
void
testname(char *string, ...)
{
    /* Save name of the test in testnamebuf */
    char buffer[TESTNAMEBUFSIZ / 2];
    
    strcpy(testnamebuf, string);

    if (string) {
        va_list args;
        va_start(args, string);
        vsprintf(buffer, string, args);
        va_end(args);
    }
    
    if(DIAGFLAG & D_QUIETMODE) return;  /* unless in quiet mode */
    
    if (DIAGFLAG & D_CONTINUOUS && testpass) {
        sprintf(testnamebuf, "%s test passes %ld", buffer, testpass);
    } else {
        sprintf(testnamebuf, "%s test ", buffer);
    }

    //clearline;
    //    moveleft(100);

}

void
flush_test_progress_buf(void)
{
    int i;

    for(i=0; i < CBUFSIZ/2; i++) {
        test_progress_buf[i] = 0x0;
    }
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
    if (fru_table_offset != FRU_INFO_INVALID){
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

/*
 * Function cterr.
 *
 * Failure messages for diagnostics should use this function.
 */
void
cterr(char errtype, int errnum, char *errstr, ...)
{
    if (fru_table_offset != FRU_INFO_INVALID) {
        
        char db_buffer[EN_BUF_SIZE] = {0};
        char log_buffer[EN_BUF_SIZE] = {0};        
        /* prompt_buf is used for adding prompt message to errlog buffer 
         * if reg dump function inside segment 5 */
        char prompt_buf[EN_BUF_SIZE] = {0};
        unsigned char *bptr_for_pnt_log_buf[3];
        /* bptr_for_pnt_log_buf[0] & [2] are used for logging segment 1~4
         * [1]is used for logging segment 6~7 */
        
        va_list ap;	
        uint comp_name_index;
        cterr_component_t *comp_ptr;
        uint debug_info_index;
        cterr_debug_t *debug_ptr;
        uint reg_dump_index;
        cterr_reg_dump_t *reg_ptr;
        uint env_dump_index;
        cterr_env_dump_t *env_ptr;
        	
        if (cterr_entered == TRUE){
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
        
#ifdef DEBUG
			printf("\nCONDEV @ %#.8x = %#x\n", &CONDEV, CONDEV);
#endif
            cterr_bptr = (char *)db_buffer;
            /* bptr_for_pnt_log_buf[0] and [2] are used for 
             * logging segment 1~4 */
            bptr_for_pnt_log_buf[0] = (unsigned char *)log_buffer;
            bptr_for_pnt_log_buf[2] = (unsigned char *)cterr_bptr;
            
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
            
            switch(errtype) {
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
        
            /* bptr_for_pnt_log_buf[1] is used for logging segment 6~7 */
            /* prepare pointer before reg_dump_func() 
             * which might use cterr_db_print to store info into 
             * cterr_bptr */
            bptr_for_pnt_log_buf[1] = (unsigned char *)cterr_bptr;

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
            } else {
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
        
#ifdef DEBUG
            /* if get mis-aligned load or SegV exception after printing
             * error, possible buffer overflow.  Use the following to
             * see if we are overflowing the buffer.
             */
            dismem((uchar *)&db_buffer[0],EN_BUF_SIZE,(uint)&db_buffer[0],2);
#endif
            if (diagflag_xram & D_EXT_CUSTOMER) {
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
                logprintf(log_buffer);
            }
            
            dbprint(db_buffer);  /* Log into the Debug Buffer */
            cterr_entered = FALSE;
        }
    } else {

        char log_buffer[ERRLOG_INFO_BUF_SIZE];
        
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
            vsprintf(log_buffer, errstr, args1);
            va_end(args1);
        
            logprintf(log_buffer);
        }        
        printf(" ");
        printf(testnamebuf,errnum);
        
        printf("\n");
        
    }

    /* If "stop on error" FLAG is set then stop test execution. */
    if (diagflag_xram & D_WARNING) {
	switch(errtype) {
	case 'f':
	    if(stoponerr())
                longjmp(*monjmpptr, 1);
	    break;
	case 'w':
	    warncount++;
	    break;
      case 'a':
         longjmp(*monjmpptr,1);
         
      default:
	    break;
	}
    } else {
	if(stoponerr())
            longjmp(*monjmpptr, 1);
    }
}


uint32
cterr_db_print (char *fmtptr, ...)
{
    va_list ap;
    va_start(ap, fmtptr);

    switch (cterr_entered){            
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
    return(0);               /* return number of characters */
}

/*
 * Function stoponerr.
 *
 * This function gets called when the if diagnostics enters cterr function
 * and the STOP ON ERROR flag is set.
 */
static int
stoponerr(void)
{
    if(DIAGFLAG & D_STOPONERR && monjmpptr) {
	printf("\n");
	puts(" test stopped on error \n");
	return(1);
    } else return(0);
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
 * Function prpass.
 *
 * This function is used by diagnostics to indicate progress. 
 */
void
prpass(int pass, char *msg, ...)
{
#define SCREEN_WIDTH   80

    char *bptr, buffer[SCREEN_WIDTH];
    char en_buf[CBUFSIZ/2]={0};
    va_list args;
    va_list prargs;
    va_start(args, msg); 
    va_start(prargs, msg); 

    /* Clear current line
     */
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
            if (!(diagflag_xram & D_EXT_CUSTOMER)) {
                printf(en_buf);
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
        if (!(diagflag_xram & D_EXT_CUSTOMER)) {
            /* print prpass and testname to screen */
            if (msg) {
                vprintf(msg, prargs);
            }
            printf("%s", testnamebuf);			
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
        printf("%s", testnamebuf);
    }
    va_end(args);
    va_end(prargs);
    fflush(stdout);

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
    printf("\n errors = %d  warnings = %d\n", errcount, (int)warncount);
}

/*
 * test_printing_to_console
 *
 * Test function which can be called from basic utilities tests printing to
 * the console on the cavium CPU.
 */
void 
test_printing_to_console(int i)
{
    int slot = 3;
    int wic_slot = 4;

    printf("\n ------ Test testname -------- \n");
    testname("Test printing to console");
    printf("\n");
    testname("Test Slot Number %d", slot);
    printf("\n");
    testname("Test Slot Number %d WIC Slot %d", slot, wic_slot);

    printf("\n");
    i = 1;
    prpass(testpass, "here %d ", i);
 
    printf("\n");
    i = 2;
    prpass(testpass, "here here %d ", i);

    printf("\n");
    cterr('f',0," Invalid value %d", i);

    printf("\n");
    cterr('f', 0, "Unexpected DMA complete interrupt during MIB DMA test");

    printf("\n");
    slot = 6;
    cterr('f', 0, "MIB DMA test did not generate DMA complete interrupt"
	  " for slot %d", slot);

    printf("\n");
    i = 4;
    cterr('w',0," Another almot Invalid value %d", i);

    /* Diags use the prcomplete function mot commonly in this way. */
    prcomplete(testpass, errcount, (char *)0);
}

/*
 * Function logprintf.
 *
 * To save the error logs in a file.
 */
void 
logprintf (char *buf)
{
    FILE *fp;

    fp = fopen("/errlog.txt", "a");
    if (fp == NULL) {
        printf("Failed to open /errlog.txt file.\n");
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);

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
    ret = remove("/errlog.txt");
    if (ret != 0) {
        printf("Failed to remove /errlog.txt file.\n");
    }
}

/*
 * Function dbprint.
 *
 * To save the dbprint logs in a file.
 */
void 
dbprint (char *buf)
{
    FILE *fp;
    fp = fopen("/dblog.txt", "a");
    if (fp == NULL) {
        printf("Failed to open /dblog.txt file.\n");
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);
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
    ret = remove("/dblog.txt");
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

    errlog = fopen("/errlog.txt", "r");
    if (errlog == NULL) {
        printf("Failed to open /errlog.txt file.\n");
        return (1);
    }

    while (fgets(log_info, ERRLOG_INFO_BUF_SIZE, errlog) != NULL) {
        log_info[strlen(log_info)-1] = '\0';
        puts(log_info);
    }

    fclose(errlog);

    return (0);
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

    dblog = fopen("/dblog.txt", "r");
    if (dblog == NULL) {
        printf("Failed to open /dblog.txt file.\n");
        return (1);
    }

    while (fgets(log_info, ERRLOG_INFO_BUF_SIZE, dblog) != NULL) {
        log_info[strlen(log_info)-1] = '\0';
        puts(log_info);
    }

    fclose(dblog);

    return (0);
}

void
print_spining_wheel (int pass)
{
    static int idx = 0;
    if (pass < 0) {
        pass = idx++;
    }
    printf("\b");
    switch (pass%8) {
    case 0:
        printf("|");
        break;
    case 1:
        printf("/");
        break;
    case 2:
        printf("-");
        break;
    case 3:
        printf("\\");
        break;
    case 4:
        printf("|");
        break;
    case 5:
        printf("/");
        break;
    case 6:
        printf("-");
        break;
    case 7:
        printf("\\");
        break;
    default:
        break;
    }
    fflush(stdout);
    printf("\r");
}

void
clrtestname (void)
{
    memset(testnamebuf, 0, TESTNAMEBUFSIZ);
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
        
        if (com_cnt == 0){            
            string_mem = fmtptr;
        } else {
            string_mem = va_arg(ap, char*);
        } 
        if (!(strcmp(string_mem,END_LIST))){
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
                if(ll_ptr->comp_name != NULL) {
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
        if (dbg_cnt == 0){            
            string_mem = fmtptr;
        } else {
            string_mem = va_arg(ap, char*);
        }
        
        if (!(strcmp(string_mem,END_LIST))){
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
                if(ll_ptr->debug_comment != NULL) {
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
        if (reg_cnt == 0){
            func_mem = func_ptr;           
        } else {
            func_mem = va_arg(ap, void *);        
        } 
        if (!(strcmp((char *)(func_mem),END_LIST))){
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
                if(ll_ptr->reg_dump_func != NULL) {
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
        if (env_cnt == 0){
            func_mem = func_ptr;            
        } else {
            func_mem = va_arg(ap, void *);        
        }
        if (!(strcmp((char *)(func_mem),END_LIST))){
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
                if(ll_ptr->env_dump_func != NULL) {
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

static int enhance_err_flag = 1;
int
get_enhance_err_flag (void)
{
    return(enhance_err_flag);
}

void
set_enhance_err_flag (int flag)
{
    enhance_err_flag = flag;
}

/* end of module */

/*-------------------- End of File ---------------------*/
/******** History ******** 
$Log: linux_error.c,v $
Revision 1.19  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.18.40.1  2016/10/28 08:30:52  alpeng
fixed enhance error msg bug, add more info on pcird/wr, update testcard plx scan test

Revision 1.18  2014/06/03 19:08:14  mcharon
support odd size pwer seq s record

Revision 1.17  2014/02/07 18:30:32  mcharon
by default turn on err enhance flag

Revision 1.16  2013/12/12 01:25:51  mcharon
default enhance flag to 0 for now

Revision 1.15  2013/12/12 01:23:23  mcharon
add set/get method to turn on/off advance err flag used for debugging

Revision 1.14  2013/11/27 09:22:20  erwu2
1. fix new line issue on prpass
2. remove redundant ext customer flag judgement on err msg
3. improve file process

Revision 1.13  2013/10/15 05:14:34  erwu2
cterr_add_xxx supports multi-lines string

Revision 1.12  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.11  2012/11/12 10:20:18  danchung
Fix the issue that the error log strings aren't be saved correctly.

Revision 1.10  2012/10/25 19:37:12  mcharon
support warning flag

Revision 1.9  2012/10/03 11:00:02  danchung
Support CLI cmd 'clrerr' and 'errlog' to save ,display and clear the error log.

Revision 1.8  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.7  2012/07/25 02:23:08  ptong
Init the D_MIN_TEST_TIME flag in diagflag_xram

Revision 1.6  2012/07/25 00:43:07  mcharon
add size and timeout argument for uart_rx

Revision 1.5  2012/06/06 15:00:17  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/05 09:33:44  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/05/11 17:18:35  ptong
Minor fix on a printf call

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

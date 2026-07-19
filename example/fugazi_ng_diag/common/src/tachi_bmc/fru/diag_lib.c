/* $Id: diag_lib.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_lib.c,v $
 *
 *      File:   diag_lib.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include "diag_main.h"
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static char result_all_test_name[32];
static char result_all_test_status[16];
static char result_all_run_hist[16];
extern const char diag_version_str[];
static char result_all_board_serial_no[20];
static char result_all_start_time[32];
static char result_all_end_time[32];
static uint32_t run_all_run_count = 0;
static uint32_t run_all_fail_count = 0;

#ifdef HOST_GOODING
static test_results_t presall[] =
{
        {"Test Name",   FORMAT_TYPE_STR, {(unsigned long)result_all_test_name}, 0},
        {"Test Status", FORMAT_TYPE_STR, {(unsigned long)result_all_test_status}, 0},
        {"Failed/Run History", FORMAT_TYPE_STR, {(unsigned long)result_all_run_hist}, 0},
        {"Start Time",  FORMAT_TYPE_STR, {(unsigned long)result_all_start_time}, 0},
        {"End Time",    FORMAT_TYPE_STR, {(unsigned long)result_all_end_time}, 0},
        {"Diag Version",FORMAT_TYPE_STR, {(unsigned long)diag_version_str}, 0},
        {"Board S/N",   FORMAT_TYPE_STR, {(unsigned long)result_all_board_serial_no}, 0},
        {"Err Code",    FORMAT_TYPE_DEC, {0x00}, 0},
        {"Err Info",    FORMAT_TYPE_STR, {0x00}, 0},
        {"Err LPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
        {"Err HPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
        {"Fail Addr",   FORMAT_TYPE_HEX, {0x00}, 0},
        {"Data Exp",    FORMAT_TYPE_HEX, {0x00}, 0},
        {"Data Read",   FORMAT_TYPE_HEX, {0x00}, 0},
        {NULL,         FORMAT_TYPE_UNKNOWN, {0x00}, 0},
};
#else
static test_results_t presall[] =
{
        {"Test Name",   FORMAT_TYPE_STR, {(uint32_t)result_all_test_name}, 0},
        {"Test Status", FORMAT_TYPE_STR, {(uint32_t)result_all_test_status}, 0}, 
        {"Failed/Run History", FORMAT_TYPE_STR, {(uint32_t)result_all_run_hist}, 0},
        {"Start Time",  FORMAT_TYPE_STR, {(uint32_t)result_all_start_time}, 0},
        {"End Time",    FORMAT_TYPE_STR, {(uint32_t)result_all_end_time}, 0},
        {"Diag Version",FORMAT_TYPE_STR, {(uint32_t)diag_version_str}, 0},
        {"Board S/N",   FORMAT_TYPE_STR, {(uint32_t)result_all_board_serial_no}, 0},
        {"Err Code",    FORMAT_TYPE_DEC, {0x00}, 0},
        {"Err Info",    FORMAT_TYPE_STR, {0x00}, 0},
        {"Err LPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
        {"Err HPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
        {"Fail Addr",   FORMAT_TYPE_HEX, {0x00}, 0},
        {"Data Exp",    FORMAT_TYPE_HEX, {0x00}, 0},
        {"Data Read",   FORMAT_TYPE_HEX, {0x00}, 0},
        {NULL,         FORMAT_TYPE_UNKNOWN, {0x00}, 0},
};
#endif

static int diag_update_sno()
{
	int rc = 0;

	// Update Serial Number
	if (!presall[6].flag) {
		presall[6].flag = 1;
		rc = diag_board_serial_no_get(result_all_board_serial_no, 20 );
		if (rc) {
			printf("  Warning: Failed to get Serial Number\n");
			snprintf(result_all_board_serial_no, 20, "Unknown");
		}
	}

	return (rc);
}
static int diag_get_time(char *pstr) 
{
	char buffer[30];
	struct timeval tv;
	time_t curtime;

	gettimeofday(&tv, NULL);
	curtime=tv.tv_sec;

	strftime(buffer, 30, "%m-%d-%Y %T.", localtime(&curtime));
	sprintf(pstr, "%s%ld", buffer, tv.tv_usec);

	return 0;
}

#ifndef HOST_GOODING
uint64_t glob_psumask = 0x0Full;
uint64_t glob_fanmask = 0xFFull;
uint64_t glob_blademask = 0xFFull;

char glob_psuptr[32] = {'0', '-', '3', '/', 'N', 'o', 'n', 'e', '\0'}; 
char glob_fanptr[32] = {'0', '-', '7', '/', 'N', 'o', 'n', 'e', '\0'};
char glob_bladeptr[32] = {'0', '-', '7', '/', 'N', 'o', 'n', 'e', '\0'}; 
#endif


test_parameters_t global_params[] =
{               
   {parm_run_cnt,	FORMAT_TYPE_DEC, {0x01},  {0x01}, {999999},     1},
   {parm_verbose,	FORMAT_TYPE_DEC, {0x00},  {0x00}, {16},         1},
   {parm_stop_on_fail,	FORMAT_TYPE_DEC, {0x01},  {0x00}, {1},          1},
   {parm_debug,		FORMAT_TYPE_DEC, {0x00},  {0x00}, {10},         1},
   {parm_err_cnt,	FORMAT_TYPE_HEX, {0x100}, {0x00}, {0xFFFFFFFF}, 1},
   {parm_nfs,		FORMAT_TYPE_DEC, {0x00},  {0x00},  {0x01},      1},
   {parm_extended,	FORMAT_TYPE_DEC, {0x00},  {0x00},  {0x01},      1},
   {parm_revision,	FORMAT_TYPE_DEC, {0x02},  {0x00},  {0x05},      1},
   {parm_rdwacc,	FORMAT_TYPE_DEC, {0x00},  {0x00},  {0x05},      1},
#ifdef CAMPBELL
   {parm_laneswap,	FORMAT_TYPE_DEC, {0x1},   {0x00},  {0x05},      1},
   {parm_flip_pol,	FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x5},       1},
   {parm_tx_inv_pol,	FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x5},       1},
   {parm_rx_inv_pol,	FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x5},       1},
#else
   {parm_laneswap,	FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x05},      1},
   {parm_flip_pol,	FORMAT_TYPE_DEC, {0x1},   {0x00},  {0x5},       1},
   {parm_tx_inv_pol,	FORMAT_TYPE_DEC, {0x1},   {0x00},  {0x5},       1},
   {parm_rx_inv_pol,	FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x5},       1},
#endif

   {"preemp",		FORMAT_TYPE_DEC, {0x0},   {0x00},  {0x7},       1},
   {"bcmemp",		FORMAT_TYPE_DEC, {0x0},   {0x00},  {0xF},       1},

#ifndef HOST_GOODING
   {"PsuMask",  FORMAT_TYPE_MASK, {(uint32_t)glob_psuptr  },
	{(uint32_t)"0-3/None"}, {(uint32_t)&glob_psumask}, 1},
   {"FanMask",  FORMAT_TYPE_MASK, {(uint32_t)glob_fanptr  },
	{(uint32_t)"0-7/None"}, {(uint32_t)&glob_fanmask}, 1},
   {"BladeMask",  FORMAT_TYPE_MASK, {(uint32_t)glob_bladeptr}, 
	{(uint32_t)"0-7/None"}, {(uint32_t)&glob_blademask}, 1},
   {"Retry",  FORMAT_TYPE_DEC, {0x03}, {0x01}, {0x100}, 1},
   {"Sysinit",   FORMAT_TYPE_DEC, {0x01},  {0x00},  {0x05},      1},
#else
   {parm_subslot, FORMAT_TYPE_DEC, {0x0}, {0x00}, {10}, 1 }, 
#endif

   {"krphyrev",		FORMAT_TYPE_HEX, {0x109},   {0x0},  {0x1000},       1},
   {"cliloop",		FORMAT_TYPE_DEC, {0x01},  {0x01},   {999999},     1},
   {"chstest",		FORMAT_TYPE_DEC, {0x00},  {0x00},   {5},     1},
   {"Detection",	FORMAT_TYPE_DEC, {0x00},  {0x00},   {2},     1}, //0=off, 1=auto, 2=semi
   { NULL, FORMAT_TYPE_UNKNOWN, {(uint32_t)NULL}, {(uint32_t)NULL},
      {(uint32_t)NULL}, (uint32_t)NULL},
};

void diag_set_flip_flag()
{
	uint32_t *pflipPol  = diag_get_global_param_ptr(parm_flip_pol);
	uint32_t *pTxInvPol = diag_get_global_param_ptr(parm_tx_inv_pol);

	if (pflipPol)  *pflipPol = 1;
	if (pTxInvPol) *pTxInvPol = 1;
}

void *diag_get_global_param_ptr (uint8_t *parameter)
{
	test_parameters_t *pparam = global_params;

	while (pparam && pparam->parameter) {
		if (!strncasecmp(pparam->parameter, parameter, strlen(parameter))) {
			if ((pparam->format == FORMAT_TYPE_DEC) ||
			    (pparam->format == FORMAT_TYPE_HEX)) {
				return (&(pparam->discrete.value));
			} else {
				return (pparam->discrete.ptr);
			}
		}
		pparam++;
	}	

	return (NULL);
}

void *diag_get_global_max_ptr (uint8_t *parameter)
{
	test_parameters_t *pparam = global_params;

	while (pparam && pparam->parameter) {
		if (!strncasecmp(pparam->parameter, parameter, strlen(parameter))) {
			if ((pparam->format == FORMAT_TYPE_DEC) ||
			    (pparam->format == FORMAT_TYPE_HEX)) {
				return (&(pparam->max.value));
			} else {
				return (pparam->max.ptr);
			}
		}
		pparam++;
	}	

	return (NULL);
}

static uint32_t diag_get_glob_param (uint8_t *parm_str)
{
	test_parameters_t *pparam = global_params;

	while (pparam && pparam->parameter) {
		if (!strcasecmp(pparam->parameter, parm_str))
			return (pparam->discrete.value);
		pparam++;
	}
	return (0);
}

uint32_t diag_get_psumask()
{
	uint64_t *pdata = diag_get_global_max_ptr("PsuMask");
	return ((uint8_t) *pdata);
}

uint32_t diag_get_fanmask()
{
	uint64_t *pdata = diag_get_global_max_ptr("FanMask");
	return ((uint8_t) *pdata);
}

uint32_t diag_get_blademask()
{
	uint64_t *pdata = diag_get_global_max_ptr("BladeMask");
	return ((uint8_t) *pdata);
}

uint32_t diag_get_retry()
{
	return (diag_get_glob_param("Retry"));
}

uint32_t diag_get_subslot()
{
	return (diag_get_glob_param("subslot"));
}

uint32_t diag_get_sysinit()
{
	return (diag_get_glob_param("Sysinit"));
}

uint32_t diag_get_revision()
{
	return (diag_get_glob_param(parm_revision));
}

uint32_t diag_get_krphyrev()
{
	return (diag_get_glob_param("krphyrev"));
}

uint32_t diag_get_cliloop()
{
	return (diag_get_glob_param("cliloop"));
}

uint32_t diag_get_chstest()
{
	return (diag_get_glob_param("chstest"));
}

uint32_t diag_get_debug()
{
	return (diag_get_glob_param(parm_debug));
}

uint32_t diag_get_extended()
{
	return (diag_get_glob_param(parm_extended));
}

uint32_t diag_get_bcmemp()
{
	return (diag_get_glob_param("bcmemp"));
}

uint32_t diag_get_preemp()
{
	return (diag_get_glob_param("preemp"));
}

uint32_t diag_get_detection()
{
	return (diag_get_glob_param("Detection"));
}
uint32_t diag_get_nfsmode()
{
	return (diag_get_glob_param(parm_nfs));
}

uint32_t diag_get_runcount()
{
	return (diag_get_glob_param(parm_run_cnt));
}

uint32_t diag_get_verbose()
{
	return (diag_get_glob_param(parm_verbose));
}

uint32_t diag_get_rdwacc()
{
	return (diag_get_glob_param(parm_rdwacc));
}

uint32_t diag_get_laneswap()
{
	return (diag_get_glob_param(parm_laneswap));
}

uint32_t diag_get_stop_on_fail()
{
	return (diag_get_glob_param(parm_stop_on_fail));
}

#ifdef HOST_GOODING
static void diag_test_parm_print(test_parameters_t *p_param)
{
        if (!p_param || !p_param->parameter) return;

        if (p_param->format == FORMAT_TYPE_HEX) {
                printf("   %-10s : 0x%lX [0x%lX - 0x%lX]\n",
                        p_param->parameter, p_param->discrete.value,
                        p_param->min.value, p_param->max.value);
        } else if (p_param->format == FORMAT_TYPE_DEC) {
                printf("   %-10s : %ld [%ld - %ld]\n",
                        p_param->parameter, p_param->discrete.value,
                        p_param->min.value, p_param->max.value);
        } else if (p_param->format == FORMAT_TYPE_STR) {
                printf("   %-10s : %s [%s]\n",
                        p_param->parameter, p_param->discrete.ptr,
                        p_param->min.ptr);
        } else if (p_param->format == FORMAT_TYPE_MASK) {
                printf("   %-10s : %s (0x%lX) [%s]\n",
                        p_param->parameter, p_param->discrete.ptr,
                        *(uint64_t*)p_param->max.ptr,  p_param->min.ptr);
        } else {
                printf(" ERROR: Unrecognized format\n");
        }
}
#else
static void diag_test_parm_print(test_parameters_t *p_param)
{
	if (!p_param || !p_param->parameter) return;

	if (p_param->format == FORMAT_TYPE_HEX) {
		printf("   %-10s : 0x%X [0x%X - 0x%X]\n",
			p_param->parameter, p_param->discrete.value,
			p_param->min.value, p_param->max.value);
	} else if (p_param->format == FORMAT_TYPE_DEC) {
		printf("   %-10s : %d [%d - %d]\n",
			p_param->parameter, p_param->discrete.value,
			p_param->min.value, p_param->max.value);
	} else if (p_param->format == FORMAT_TYPE_STR) {
		printf("   %-10s : %s [%s]\n",
			p_param->parameter, p_param->discrete.ptr,
			p_param->min.ptr);
	} else if (p_param->format == FORMAT_TYPE_MASK) {
		printf("   %-10s : %s (0x%llX) [%s]\n",
			p_param->parameter, p_param->discrete.ptr,
			*(uint64_t*)p_param->max.ptr,  p_param->min.ptr);
	} else {
		printf(" ERROR: Unrecognized format\n");
	}
}
#endif

void diag_dump_glob_param()
{
	test_parameters_t *pparam = global_params;

	if (pparam) {
		printf ("Global Parameters:\n");
	}
	while (pparam && pparam->parameter) {
		diag_test_parm_print(pparam++);
	}
	printf("\n");
}


static void diag_test_toc_print(test_toc_t *p_test_toc)
{
	if (p_test_toc && p_test_toc->name && p_test_toc->description) {
		printf("   %c%-8s : %s\n", 
			(p_test_toc->control & TEST_TYPE_SKIP) ? '>' :' ', 
			p_test_toc->name, p_test_toc->description);
	}
}

static void diag_section_toc_print(section_toc_t *p_sect_toc)
{
	if (p_sect_toc && p_sect_toc->name && p_sect_toc->description) {
		printf("%s : %s\n", p_sect_toc->name, p_sect_toc->description);
	}
}

void diag_test_toc_dump(test_toc_t *p_test_toc)
{
	if (!p_test_toc) return;

	while (p_test_toc->name != NULL) {
		diag_test_toc_print(p_test_toc);
		p_test_toc++;
	}
}

void diag_dump_test(test_toc_t *ptest)
{
	test_parameters_t *pparam;
	if (!ptest)
		return; 
	if (!ptest->name || !ptest->description)
		return; 

	printf("%-8s : %s\n", ptest->name, ptest->description);
	pparam = ptest->test_params;	

	while (pparam && pparam->parameter) {
		diag_test_parm_print(pparam);
		pparam++;
	}
}

test_toc_t* diag_test_get(section_toc_t *psect, uint8_t *name)
{
	test_toc_t	*ptest;

	if (!psect || !name) return NULL;

	ptest = psect->test_toc;
	while (ptest && ptest->name) {
		
		if (!strcasecmp(ptest->name, name)) {
			return ptest;
		}
		ptest++;
	}

	return NULL;
}

void diag_dump_section(section_toc_t *psect)
{
	test_toc_t *ptest = NULL;

	if (!psect) return;

	diag_section_toc_print(psect);
	ptest = psect->test_toc;

	while (ptest && ptest->name) {
		diag_test_toc_print(ptest);
		ptest++;
	}
	printf("\n");
}

section_toc_t* diag_section_get(uint8_t *name)
{
	diag_dev_t *pdev = diag_board_dev_get();

	if (!name || !pdev) return NULL;

	while (pdev) {
		if (pdev->psection != NULL) {
			if (!strcasecmp(pdev->psection->name, name)) {
				return pdev->psection;
			}
		}

		pdev = pdev->p_next;
	}

	return (NULL);
}

void diag_dump_all()
{
	diag_dev_t *pdev = diag_board_dev_get();

	while (pdev) {
		diag_dump_section(pdev->psection);
		pdev = pdev->p_next;
	}
}


uint32_t diag_run_test(test_toc_t *ptest, uint32_t cnt)
{
	uint32_t	rc = 0, test_fail_count = 0, test_run_count = 0;
	test_results_t	*results;

	if (!ptest) return (DIAG_SOFTWARE_ERROR);
	if (ptest->control & TEST_TYPE_SKIP) {
		printf("Skip %s Test [%s]\n", ptest->description, ptest->short_name);
		return (rc);
	}

	diag_results_update (ptest->test_params, ptest->test_results);
	rc = diag_results_enable(ptest->test_results, 1);
	if (rc) return (rc);
	
	if (ptest->test_exec) {
		if ((results = ptest->test_results) == NULL) {
			printf("ERROR: Result Parameters are not available\n");
			return (DIAG_SOFTWARE_ERROR);
		}
	
		results += (diag_param_size_get(ptest->test_params) -1);

		// Update Run Count;
		results->flag = 1;
		results->discrete.value++;
		test_run_count = results->discrete.value;
		printf("Running %s Test [%s] -> %d\n", 
			ptest->description, 
			ptest->short_name, cnt);
		results++;


		// Update Start Time;
		results->flag = 1;
		diag_get_time(results->discrete.ptr);
		results++;

		rc = (*ptest->test_exec)(ptest->test_params, 
					ptest->test_results);

		// Update End Time;
		results->flag = 1;
		diag_get_time(results->discrete.ptr);
		results++;

		// Update Test Name
		results->flag = 1;
		snprintf(results->discrete.ptr, 20, "%s", ptest->name);
		results++;

		// Update Test Status
		results->flag = 1;
		memcpy(results->discrete.ptr, rc? "Failed":"Passed",
			strlen("Passed")+1);
		results++;
	
		// Update Fail Count
		results->flag = 0;
		results->discrete.value += (rc? 1 : 0); 
		test_fail_count = results->discrete.value;
		results++;

		// Update Failed/Run History 
		results->flag = 1;
		snprintf(results->discrete.ptr, 16, "%d/%d",
			test_fail_count, test_run_count);
		results++;

		// Update Diag Version 
		results->flag = 1;
		snprintf(results->discrete.ptr, 64, "%s", 
			diag_version_str);
		results++;

		// Update Serial Number 
		diag_update_sno();
		results->flag = 1;
		strncpy(results->discrete.ptr, 
				result_all_board_serial_no, 20);
		results++;

		// Update Fail Parameters
		if (rc) {
			ptest->control |= TEST_TYPE_FAIL;
			ptest->control &= ~TEST_TYPE_PASS;
			if (!strcasecmp(results->parameter, 
				"Err Code")) {
				if (!results->discrete.value) {
					results->flag = 1;
					results->discrete.value = rc; 
					results++;
				}
			}
		} else {
			ptest->control |= TEST_TYPE_PASS;
			ptest->control &= ~TEST_TYPE_FAIL;
		} 
	}

	if (rc) {
		printf("Test Failed with error code ");
		diag_err_display(rc);
	} else {
		printf("Test Completed Successfully\n\n");
	}
	return (rc);
}

uint32_t diag_run_section(section_toc_t *psect, uint32_t cnt)
{
	test_toc_t *ptest = NULL;
	int	rc = 0, err_cnt=0;
	uint32_t stop_on_fail = diag_get_stop_on_fail();

	if (!psect) {
		printf("  Invalid Section. Use (show all) to get the list\n");
		return (rc);
	}
	printf("Running %s [%s] -> %d\n", 
		psect->description, psect->name, cnt);
	ptest =  psect? psect->test_toc : NULL;
	while (ptest && ptest->name) {
		rc = diag_run_test(ptest, cnt);
		if (rc) err_cnt++;
		if (stop_on_fail && rc) 
			return (err_cnt);
		ptest++;
	}
	return (err_cnt);
}

uint32_t diag_run_all(uint32_t runcount)
{
	int   rc = 0, err=0;
	uint32_t cnt;
	diag_dev_t *pdev = NULL;
	uint32_t stop_on_fail = diag_get_stop_on_fail();

	run_all_run_count = 0;
	run_all_fail_count = 0;
	// Test name
	snprintf(presall[0].discrete.ptr, 16, "All");
	presall[0].flag = 1;

	// Start Time
	diag_get_time(presall[3].discrete.ptr);
	presall[3].flag = 1;
	for (cnt = 0; cnt < runcount; cnt++) {
		run_all_run_count++;
		printf("Running All -> %d\n", cnt+1);
		pdev = diag_board_dev_get();
		while (pdev) {
			rc = diag_run_section(pdev->psection, cnt+1);
			if (rc) {
				err = rc;
				run_all_fail_count+=rc;
			}
			if (stop_on_fail && rc) {
				// End Time
				diag_get_time(presall[4].discrete.ptr);
				presall[4].flag = 1;
				return (rc);
			}
			pdev = pdev->p_next;
		}
	}

	// End Time
	diag_get_time(presall[4].discrete.ptr);
	presall[4].flag = 1;
	return (rc);
}
int diag_run (int argc, char *argv[])
{
	int cnt, rc = 0, tcount = diag_get_runcount();
	int stop_on_fail = diag_get_stop_on_fail();
 

#ifdef HOST_GOODING
       presall[6].flag = 0;
#endif

	if (argc == 1) {
		return (DIAG_SYNTAX_ERROR);		
	} 
	if (argc == 2) {
		if (!strcasecmp(argv[1], "all")) {
			diag_run_all(diag_get_runcount());
		} else {
			for (cnt = 0; cnt < tcount; cnt++) {
				rc = diag_run_section(diag_section_get(argv[1]), cnt+1); 
				if (stop_on_fail && rc)
					return (rc);
			}
		}
	} else if (argc == 3) {
		for (cnt = 0; cnt < tcount; cnt++) {
			rc = diag_run_test(diag_test_get(diag_section_get(argv[1]),
				argv[2]), cnt+1); 
			if (stop_on_fail && rc)
				return (rc);
		}
	}
	return (rc);
}

uint32_t diag_skip_test(test_toc_t *ptest, uint8_t flag)
{
	if (!ptest) return(0);
	if (flag) {
		ptest->control |= TEST_TYPE_SKIP;
	} else {
		ptest->control &= ~TEST_TYPE_SKIP;
	}
	return (0);
}

uint32_t diag_skip_section(section_toc_t *psect, uint8_t flag)
{
	test_toc_t *ptest = NULL;
	int	rc = 0;

	ptest =  psect? psect->test_toc : NULL;
	while (ptest && ptest->name) {
		rc = diag_skip_test(ptest, flag);
		if (rc) return (rc);
		ptest++;
	}
	return (rc);
}

uint32_t diag_skip_all(uint8_t flag)
{
        uint32_t   rc = 0;
        diag_dev_t *pdev = diag_board_dev_get();

        while (pdev) {
                rc = diag_skip_section(pdev->psection, flag);
                if (rc) return (rc);
                pdev = pdev->p_next;
        }
        return (rc);
}

#ifdef HOST_GOODING
static void diag_results_print(test_results_t *presults)
{
        while (presults && presults->parameter != NULL)
        {
                if (presults->flag)
                {
                        printf ("  %-18s : ", presults->parameter);
                        switch (presults->format) {
                                case    FORMAT_TYPE_STR:
                                        printf ("%-10s\n", (uint8_t *) presults->discrete.ptr);
                                        break;

                                case    FORMAT_TYPE_HEX:
                                        printf ("0x%08lX\n", presults->discrete.value);
                                        break;

                                case    FORMAT_TYPE_MASK:
                                        printf ("%s\n", presults->discrete.ptr);
                                        break; 

                                case    FORMAT_TYPE_DEC:
                                        printf ("%-10ld\n", presults->discrete.value);
                                        break;

                                default:
                                        break;
                        }
                }
                presults++;
        }
        printf("\n");
}
#else
static void diag_results_print(test_results_t *presults)
{
	while (presults && presults->parameter != NULL)
	{
		if (presults->flag)
		{
			printf ("  %-18s : ", presults->parameter);
			switch (presults->format) {
				case	FORMAT_TYPE_STR:
					printf ("%-10s\n", (uint8_t *) presults->discrete.ptr);
					break;

				case	FORMAT_TYPE_HEX:
					printf ("0x%08X\n", presults->discrete.value);
					break;

				case	FORMAT_TYPE_MASK:
					printf ("%s\n", presults->discrete.ptr);
					break;

				case	FORMAT_TYPE_DEC:
					printf ("%-10d\n", presults->discrete.value);
					break;

				default:
					break;
			}
		}
		presults++;
	}
	printf("\n");
}
#endif

uint32_t diag_results_test(test_toc_t *ptest, uint32_t flag)
{
	test_results_t	*presults = ptest? ptest->test_results : NULL;

	if (!ptest) return (0);

	if (flag) {
		if (!(ptest->control & flag)) {
			return (0);
		}
	} 

	if (!(ptest->control & (TEST_TYPE_PASS | TEST_TYPE_FAIL))) {
		// The test has never been run.
		return (0);
	} 

	// Display the results.
	printf("%s [%s]\n", ptest->description, ptest->name);
	diag_results_print(presults);
	return (0);
}


uint32_t diag_results_section(section_toc_t* psect, uint32_t flag)
{
	test_toc_t *ptest = NULL;
	uint32_t    rc = 0;

	if (!psect) return (0);
	ptest =  psect? psect->test_toc : NULL;
	while (ptest && ptest->name) {
		rc = diag_results_test(ptest, flag);
		if (rc) return (rc);
		ptest++;
	}
	return (rc);
}

test_toc_t *diag_results_failed(section_toc_t* psect, uint32_t flag)
{
	test_toc_t *ptest = NULL;
	if (!psect) return (0);
	ptest =  psect? psect->test_toc : NULL;
	while (ptest && ptest->name) {
        	if (ptest->control & TEST_TYPE_FAIL) {
			return(ptest);
		}
		ptest++;
	}
	return (NULL);
}

uint32_t diag_results_all(uint32_t flag)
{
        uint32_t         rc = 0;
        diag_dev_t      *pdev = diag_board_dev_get();
	test_toc_t      *ptest = NULL;
	test_results_t	*presults = NULL;

	if (!presall[0].flag) return(0);

        while (pdev) {
                ptest = diag_results_failed(pdev->psection, flag);
		if (ptest) break;
                pdev = pdev->p_next;
        }
	snprintf(presall[1].discrete.ptr, 16, "Passed");
	presall[1].flag = presall[0].flag;

	// Run Count
	snprintf(presall[2].discrete.ptr, 16, "%d / %d", 
		run_all_fail_count, run_all_run_count);
	presall[2].flag = 1;

	// Start/End Time
//	presall[3].discrete.ptr
//	presall[4].discrete.ptr

	// Diag Version
	presall[5].flag = 1;

	// Serial Number
	diag_update_sno();

	if (ptest && ptest->name) {
		// get the name
		snprintf(presall[0].discrete.ptr, 32, "%s - %s", 
			pdev->psection->name,  ptest->short_name);

		presults = ptest->test_results;
		while (presults && presults->parameter) {
			if (!strncasecmp(presults->parameter,
				"Run Count", strlen("Run Count")))
				break;
			presults++;
		}
		if (!presults || !presults->parameter) {
			printf(" Failed Parameters not found\n");
			return(0);
		}

		// Test Status			
		strncpy(presall[1].discrete.ptr, presults[4].discrete.ptr, 16);
		presall[1].flag = presults[4].flag;


		// Error code
		presall[7].discrete.value  = presults[9].discrete.value;
		presall[7].flag = presults[9].flag;

		// Error Device
		presall[8].discrete.ptr  = presults[10].discrete.ptr;
		presall[8].flag  = presults[10].flag;

		// Error Ports
		presall[9].discrete.value  = presults[11].discrete.value;
		presall[9].flag  = presults[11].flag;

		presall[10].discrete.value  = presults[12].discrete.value;
		presall[10].flag  = presults[12].flag;

		presall[11].discrete.value = presults[13].discrete.value;
		presall[11].flag = presults[13].flag;

		presall[12].discrete.value = presults[14].discrete.value;
		presall[12].flag = presults[14].flag;

		presall[13].discrete.value = presults[15].discrete.value;
		presall[13].flag = presults[15].flag;
	}

	diag_results_print(presall);
	return (rc);
}

static uint32_t diag_results_pass_fail(uint32_t flag)
{
        uint32_t   rc = 0;
        diag_dev_t *pdev = diag_board_dev_get();

        while (pdev) {
                rc = diag_results_section(pdev->psection, flag);
                if (rc) return (rc);
                pdev = pdev->p_next;
        }
        return (rc);
}

uint32_t diag_results_fail()
{
	return (diag_results_pass_fail(TEST_TYPE_FAIL));
}

uint32_t diag_results_pass()
{
	return (diag_results_pass_fail(TEST_TYPE_PASS));
}

uint32_t diag_convert_str_to_mask(char *pstr, uint64_t *data)
{
        unsigned char *ptr;
        int sbit, ebit, mbit;

	if (!strncasecmp(pstr, "None", strlen("None"))) {
		*data = 0x00ull;
		return (0);
	}
        // Check if the string is valid.
        ptr = pstr;
        while (*ptr != '\0') {
                if ((*ptr == ' ') ||
                    (*ptr == '\t') ||
                    (*ptr == ',') ||
                    (*ptr == '-') ||
                    (isdigit(*ptr)))
                        ptr++;
                else {
                        return (DIAG_SYNTAX_ERROR);
                }
        }

	// Now parse the string.
	ptr = pstr;
        while (*ptr != '\0') {
                sbit = atoi(ptr);

                while((*ptr != ',') &&  (*ptr != '-')) {
                        if (*ptr == '\0')
                                break;
                        ptr++;
                }

                if (*ptr == '-') {
                        ptr++;
                        ebit = atoi(ptr);

                        if (ebit > sbit) {
                                for (mbit = sbit; mbit <= ebit; mbit++) {
                                        *data |= BIT64(mbit);
                                }
                        } else {
                                return (-1);
                        }
                        while (*ptr != '\0' && *ptr++ != ',');

                } else if (*ptr == ',') {
                        ptr++;
                        *data |= BIT64(sbit);
                } else if (*ptr == '\0') {
                        *data |= BIT64(sbit);
                }
        }

        return 0;
}

uint32_t diag_set_param(int argc, char *argv[])
{
	uint64_t   data64 = 0;
	uint32_t   cnt = 0, glob=0, strsize, data, rc = 0;
	test_toc_t *ptest = NULL;

	if (argc < 2) return (DIAG_SOFTWARE_ERROR);
	if (!strcasecmp(argv[1], "glob")) {
		glob = 1;
	} else {
		ptest = diag_test_get(diag_section_get(argv[1]), argv[2]);	
		if (!ptest) return (DIAG_SYNTAX_ERROR);
	}

	for (cnt = glob ? 2 : 3; cnt < (uint32_t)argc; cnt++) {
		test_parameters_t *tparam = glob ? global_params : ptest->test_params;
		char *pstr, *tempstr = NULL;

		while (tparam && tparam->parameter) {
			if (!strncasecmp(tparam->parameter, argv[cnt], strlen(tparam->parameter))) {
				pstr = argv[cnt] + strlen(tparam->parameter);
				// Skip all spaces here
				while (*pstr == ' ')
					pstr++;

				if (*pstr == '\0') {
					cnt++;
					pstr = argv[cnt];
				}

				// Check for =
				if (*pstr != '=')
					return (DIAG_SYNTAX_ERROR);	
				else 
					pstr++;

				// Skip all spaces again.
				while (*pstr == ' ')
					pstr++;

				if (*pstr == '\0') {
					cnt++;
					pstr = argv[cnt];
				}
				switch (tparam->format) {
					case	FORMAT_TYPE_DEC:
						data =  strtoul(pstr, NULL, 10);
						if ((data >= tparam->min.value) &&
						    (data <= tparam->max.value)) 
							tparam->discrete.value = data;
						break;

					case	FORMAT_TYPE_HEX:
						data =  strtoul(pstr, NULL, 16);
						if ((data >= tparam->min.value) &&
						    (data <= tparam->max.value)) 
							tparam->discrete.value = data;
						break;

					case	FORMAT_TYPE_STR:
						// validate if the input string is an allowed choice.
						tempstr = strstr(tparam->min.ptr, pstr);
						if (!tempstr) break;
						strsize = strlen(pstr);
						while((*(tempstr + strsize) != '/') &&
						      (*(tempstr + strsize) != '\0') &&
						      (*(tempstr + strsize) != ' ')) {
							strsize++;
							if (strsize >= 80) return (DIAG_SOFTWARE_ERROR);
						}
						memcpy(tparam->discrete.ptr, tempstr, strsize);

						// Null Terminate the String.
						tparam->discrete.ptr[strsize] = '\0';
						break;

					case	FORMAT_TYPE_MASK:
						rc = diag_convert_str_to_mask(pstr, &data64);
						if (!rc) {
							memcpy(tparam->discrete.ptr, pstr, strlen(pstr)+1);
							*(uint64_t*)tparam->max.ptr = data64;
						}
						data64 = 0x00ull;
						break;

					case	FORMAT_TYPE_UNKNOWN:
						printf("ERROR : Invalid Format Type %d\n", 
							tparam->format);
						break;

					default:
						break;
				}
			}
			tparam++;
		}
	}

	return (0);
}

void diag_err_code_set(test_results_t *presults, uint32_t errcode)
{
	while (presults->parameter != NULL) {
		if (!strcasecmp(presults->parameter, "Err Code")) {
			presults->flag = 1;
			presults->discrete.value = errcode;
			return;
		}
		presults++;
	}
}

void diag_err_device_set(test_results_t *presults, uint8_t *dev)
{
	while (presults->parameter != NULL) {
		if (!strcasecmp(presults->parameter, "Err Device")) {
			presults->flag = 1;
			memcpy(presults->discrete.ptr, dev, strlen(dev)+1);
			return;
		}
		presults++;
	}
}

void diag_err_port_set(test_results_t *presults, uint64_t pmask)
{
	while (presults->parameter != NULL) {
		if (!strcasecmp(presults->parameter, "Err LPorts")) {
			presults->flag = 1;
			presults->discrete.value = (uint32_t)(pmask & 0xFFFFFFFF);
			presults++;
			if (!strcasecmp(presults->parameter, "Err HPorts")) {
				presults->flag = 1;
				presults->discrete.value = (uint32_t)(pmask >> 32); 
			}
			return;
		}
		presults++;
	}
}

void diag_err_addr_data_set(test_results_t *presults, uint32_t addr, uint32_t edata, uint32_t rdata)
{
	while (presults->parameter != NULL) {
		if (!strcasecmp(presults->parameter, "Fail Addr")) {
			presults->flag = 1;
			presults->discrete.value = addr;
		}
		if (!strcasecmp(presults->parameter, "Data Exp")) {
			presults->flag = 1;
			presults->discrete.value = edata;
		}
		if (!strcasecmp(presults->parameter, "Data Read")) {
			presults->flag = 1;
			presults->discrete.value = rdata;
		}
		presults++;
	}
}


void diag_test_param_dump(test_parameters_t *pparam)
{
        while(pparam && pparam->parameter) {
                diag_test_parm_print(pparam);
                pparam++;
        }
}

void diag_test_param_set(const char *sect_name, const char *test_name, 
			const char *param_name, uint32_t value, uint32_t min, 
			uint32_t max)
{
	test_toc_t *ptest = diag_test_get(diag_section_get((uint8_t*)sect_name), (uint8_t*)test_name);	
	test_parameters_t *pparam;

	if (!ptest) return; 

	pparam = ptest->test_params;
        while(pparam && pparam->parameter) {
                if (!strncasecmp(pparam->parameter, param_name, strlen(pparam->parameter))) { 
			pparam->discrete.value = value;
			pparam->min.value = min;
			pparam->max.value = max;
			return;
		}
                pparam++;
        }
}

void diag_test_str_param_set(const char *sect_name, const char *test_name, 
			const char *param_name, char *pstr)
{
	test_toc_t *ptest = diag_test_get(diag_section_get((uint8_t*)sect_name), (uint8_t*)test_name);	
	test_parameters_t *pparam;
	uint64_t data64=0x00ull;

	if (!ptest) return; 

	pparam = ptest->test_params;
        while(pparam && pparam->parameter) {
                if (!strncasecmp(pparam->parameter, param_name, strlen(pparam->parameter))) { 
			memcpy(pparam->discrete.ptr, pstr, strlen(pstr)+1);
			diag_convert_str_to_mask(pstr, &data64);
			*(uint64_t*)pparam->max.ptr = data64;
			return;
		}
                pparam++;
        }
}


#ifdef HOST_GOODING
void diag_test_results_dump(test_results_t *presults)
{                                       
        while (presults && presults->parameter != NULL)
        {
                printf ("  %-18s : ", presults->parameter);
                switch (presults->format) {
                        case    FORMAT_TYPE_STR:
                                printf ("[%-10s]\n", (uint8_t *) presults->discrete.ptr);
                                break;

                        case    FORMAT_TYPE_HEX:
                                printf ("[0x%08lX]\n", presults->discrete.value);
                                break;

                        case    FORMAT_TYPE_MASK:
                                printf ("[%-10ld]\n", presults->discrete.value);
                                break;
                
                        case    FORMAT_TYPE_DEC:
                                printf ("[%-10ld]\n", presults->discrete.value);
                                break;

                        default:
                                break;
                }
                presults++;
        }
}
#else
void diag_test_results_dump(test_results_t *presults)
{
        while (presults && presults->parameter != NULL)
        {
                printf ("  %-18s : ", presults->parameter);
                switch (presults->format) {
                        case    FORMAT_TYPE_STR:
                                printf ("[%-10s]\n", (uint8_t *) presults->discrete.ptr);
                                break;  

                        case    FORMAT_TYPE_HEX:
                                printf ("[0x%08X]\n", presults->discrete.value);
                                break;  

                        case    FORMAT_TYPE_MASK:
                                printf ("[%-10d]\n", presults->discrete.value);
                                break;  

                        case    FORMAT_TYPE_DEC:
                                printf ("[%-10d]\n", presults->discrete.value);
                                break;  

                        default:
                                break;  
                }       
                presults++;
        }
}
#endif

void diag_ttoc_dump(test_toc_t *ptest)
{
        while(ptest && ptest->name) {
                printf("%s : %s (%p %p %d)\n", ptest->name, ptest->description,
                        ptest->test_params, ptest->test_results, ptest->control);

		diag_test_param_dump(ptest->test_params);
		diag_test_results_dump(ptest->test_results);
                ptest++;
        }
}

void diag_section_toc_dump(section_toc_t *psect)
{
        if (psect && psect->name) {
                printf("%s : %s (%p %d)\n", psect->name, psect->description,
                        psect->test_toc, psect->control);
		diag_ttoc_dump(psect->test_toc);
        }
}

int is_diag_boot()
{
        FILE *fd;
        char my_str[256];

        fd = fopen("/proc/cmdline", "r");
        if (fd <= 0) {
                printf("Error: Failed to open file\n");
                return (0);
        }

        fgets(my_str, sizeof(my_str)-1, fd);
        fclose(fd);
		
        return((strstr(my_str, "diag")) ? 1 : 0); 
}


/* $Id: diag_cmd.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_cmd.c,v $
 *
 *      File:   diag_cmd.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#ifdef DIAG_TCL
#include <tcl.h>
#endif

#include "diag_main.h"



typedef struct _diag_cli_help_ diag_cli_help_t; 
struct _diag_cli_help_ {
	char *name;
	char *syntax;
	diag_cli_help_t *next;
};

static diag_cli_help_t	*diag_cli_help = NULL;

////////////////////////////////////////////////////////////////////
// help
////////////////////////////////////////////////////////////////////
static char diag_desc_help[] = "\nHelp:\n\
        Syntax: help\n\
        Prints the list of commands supported\n";

static int
dsh_cmd_help (TCL_ARGS int argc, char *argv[])
{
	int cnt = 0;
	diag_cli_help_t *phelp = diag_cli_help;

	if (argc == 1 ) {
		while (phelp) {
			printf ("%s", phelp->syntax);
			phelp = phelp->next;
		}
	} else {
		for (cnt = 1; cnt < argc; cnt++) {
			phelp = diag_cli_help;
			while (phelp) {
				if (!strncasecmp(phelp->name, argv[cnt], 
						strlen(argv[cnt]))) {
					printf("%s", phelp->syntax);
				}
				phelp = phelp->next;	
			}
		}
	}
        return 0;
}


////////////////////////////////////////////////////////////////////
// run
////////////////////////////////////////////////////////////////////
static char diag_desc_run[] = "\nrun:\n\
        Syntax: run <all/sectionid> <testid>\n\
        Runs the specified test if it is present.\n";

extern int diag_run (int argc, char *argv[]);
static int
dsh_cmd_run (TCL_ARGS int argc, char *argv[])
{
	if ((diag_run(argc, argv)) == DIAG_SYNTAX_ERROR)
		printf("%s\n", diag_desc_run);
        return 0;
}

////////////////////////////////////////////////////////////////////
// show
////////////////////////////////////////////////////////////////////
static char diag_desc_show[] = "\nshow:\n\
        Syntax: show <sectionid> <testid>\n\
        show the specified test if it is present.\n";

static int
dsh_cmd_show (TCL_ARGS int argc, char *argv[])
{
	if (argc == 1) {
		diag_dump_glob_param();
	} else if (argc == 2) {
		
		if (!strcasecmp(argv[1], "dev")) {
			diag_dump_dev();
		} else if (!strcasecmp(argv[1], "all")) {
			diag_dump_all();
		} else {
			diag_dump_section(diag_section_get(argv[1]));
		}
	} else if (argc == 3) {
		diag_dump_test(diag_test_get(diag_section_get(argv[1]), argv[2]));
	}
        return 0;
}

////////////////////////////////////////////////////////////////////
// skip/unskip
////////////////////////////////////////////////////////////////////
static char diag_desc_skip[] = "\nskip:\n\
        Syntax: skip <all/sectionid> <testid>\n\
        Skip the specified section/test if it is present.\n";

static char diag_desc_unskip[] = "\nUnskip:\n\
        Syntax: unskip <all/sectionid> <testid>\n\
        Unskip the specified section/test if it is present.\n";

static int
dsh_test_skip (int argc, char *argv[], unsigned char flag)
{
	if (argc == 1) {
		printf("Syntax Error:\n");
		printf("%s", diag_desc_skip);	
	} else if (argc == 2) {
		if (!strncasecmp(argv[1], "all", strlen("all"))) {
			diag_skip_all(flag);
		} else {
			diag_skip_section(diag_section_get(argv[1]), flag);
		}
	} else if (argc == 3) {
		diag_skip_test(diag_test_get(diag_section_get(argv[1]), argv[2]), flag);
	}
        return 0;
}

static int dsh_cmd_skip (TCL_ARGS int argc, char *argv[])
{
	return (dsh_test_skip(argc, argv, 1));
}

static int dsh_cmd_unskip (TCL_ARGS int argc, char *argv[])
{
	return (dsh_test_skip(argc, argv, 0));
}

////////////////////////////////////////////////////////////////////
// results
////////////////////////////////////////////////////////////////////
static char diag_desc_results[] = "\nresults:\n\
        Syntax: results <all/pass/fail/sectionid> <testid>\n\
        Display results of the all or passed or failed tests.\n\
	OR the specified section/test if it is present.\n";

static int
dsh_cmd_results (TCL_ARGS int argc, char *argv[])
{
	if (argc == 1) {
		
	} else if (argc == 2) {
		
		if (!strcasecmp(argv[1], "all")) {
			diag_results_all(0);
		} else if (!strcasecmp(argv[1], "pass")) {
			diag_results_pass();
		} else if (!strcasecmp(argv[1], "fail")) {
			diag_results_fail();
		} else {
			diag_results_section(diag_section_get(argv[1]), 0);
		}
	} else if (argc == 3) {
		diag_results_test(diag_test_get(diag_section_get(argv[1]), argv[2]), 0);
	}
        return 0;
}

////////////////////////////////////////////////////////////////////
// sp
////////////////////////////////////////////////////////////////////
static char diag_desc_sp[] = "\nsp:\n\
        Syntax: sp dev/glob/<sectionid> <testid> <parameter>=<value>\n\
        Runs the specified test if it is present.\n";

static int
dsh_cmd_sp (TCL_ARGS int argc, char *argv[])
{
	if( argc < 2) {
		printf("%s\n", diag_desc_sp);
		return(0);
	}
	if (!strcasecmp(argv[1], "dev")) {
		if (argc != 4) {
			printf("Usage: sp dev <devname> <devaddr>\n");
			return(0);
		}
		diag_fix_dev_addr(argv[2], strtoul(argv[3], NULL, 0));
	} else { 
		return (diag_set_param(argc, argv));
	}
	return (0);
}

////////////////////////////////////////////////////////////////////
// reg
////////////////////////////////////////////////////////////////////
static char diag_desc_reg[] = "\nreg:\n\
        Syntax: reg <device> <r/w/m/s/c/l/n/u/>\n\
	r-read, w-write, m-modify, s-set, c-clear\n\
	l-list, n-name, u-unit\n\
        Runs the specified test if it is present.\n";

static int
dsh_cmd_reg (TCL_ARGS int argc, char *argv[])
{
	return (diag_reg_access(argc, argv, 1));
}

////////////////////////////////////////////////////////////////////
// version
////////////////////////////////////////////////////////////////////
#ifdef DIAG_VERSION
const char diag_version_str[] = DIAG_VERSION;
#else
const char diag_version_str[] = "Diag Version Not Set\n";
#endif
static char diag_desc_version[] = "\nversion:\n\
        Syntax: version\n";


static int
dsh_cmd_version (TCL_ARGS int argc, char *argv[])
{
	printf("%s\n", diag_version_str);
	return (0);
}

////////////////////////////////////////////////////////////////////
// err
////////////////////////////////////////////////////////////////////
static char diag_desc_err[] = "\nerr:\n\
        Syntax: err <err num1> <err num 2> ...\n";

static int
dsh_cmd_err (TCL_ARGS int argc, char *argv[])
{
	uint32_t cnt = 0;

	if (argc == 1) {
		diag_err_display_all();
	} else {
		for (cnt = 1; cnt < (uint32_t)argc; cnt++) {
			diag_err_display(strtoul(argv[cnt], NULL, 0));
		}	
	}
	return (0);
}

static cli_cmds_t diag_cli[] =
{
        {"help",	diag_desc_help,		dsh_cmd_help},
	{"show",	diag_desc_show,		dsh_cmd_show},
	{"run",		diag_desc_run,		dsh_cmd_run},
	{"skip",	diag_desc_skip,		dsh_cmd_skip},
	{"unskip",	diag_desc_unskip,	dsh_cmd_unskip},
	{"results",	diag_desc_results,	dsh_cmd_results},
	{"sp",		diag_desc_sp,		dsh_cmd_sp},
	{"reg",		diag_desc_reg,		dsh_cmd_reg},
	{"version",	diag_desc_version,	dsh_cmd_version},
	{"err",		diag_desc_err,		dsh_cmd_err},
        {NULL,		NULL,			NULL},
};

static int diag_cli_help_db_check (char *syntax)
{
	diag_cli_help_t *phelp = diag_cli_help;
	while (phelp) {
		if (phelp->syntax == syntax) {
			return 1;
		}
		phelp = phelp->next;
	}
	return (0);
}

static int diag_cli_help_add (char *name, char *syntax)
{
	diag_cli_help_t *phelp = diag_cli_help;

	if (phelp) {
		// If the description is same for several commands,
		// avoid adding it again to the help structure.
		if (diag_cli_help_db_check(syntax))
			return (0);
		while (phelp->next) {
			phelp=phelp->next;
		}

		phelp->next = MALLOC(sizeof(diag_cli_help_t));	
		if (!phelp->next) return (DIAG_MALLOC_ERROR);

		phelp->next->name = name;
		phelp->next->syntax = syntax;
		phelp->next->next = NULL;
	} else  {
		phelp = MALLOC(sizeof(diag_cli_help_t));	
		if (!phelp) return (DIAG_MALLOC_ERROR);
		phelp->name = name;
		phelp->syntax = syntax;
		phelp->next = NULL;
		diag_cli_help = phelp;
	}
	return (0);
}

int diag_cli_cmd_add (cli_cmds_t *pcmd)
{
	int rc = 0;

        while ((pcmd != NULL) && (pcmd->name != NULL)) {
		if ((rc = diag_cli_help_add(pcmd->name, pcmd->desc))) {
                        printf("Failed to add command to help %s\n", pcmd->name);
                        return (rc);
		}
                if ((rc  = dsh_cmd_add(pcmd->name, pcmd->desc, pcmd->fptr))) {
                        printf("Failed to add command %s\n", pcmd->name);
                        return (rc);
                }
		pcmd++;
	}
	return (0);
}

int dsh_add_internal_commands()
{
	diag_ioctl_all_dev_types(IOCTL_REGISTER_COMMANDS);
	return (diag_cli_cmd_add(diag_cli));
}


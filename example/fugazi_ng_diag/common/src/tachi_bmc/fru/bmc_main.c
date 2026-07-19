/* $Id: bmc_main.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/bmc_main.c,v $
 *------------------------------------------------------------------
 *                                                                                                                                             
 *
 *      File:   diag_main.c
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
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "diag_main.h"
#include "diag_sock.h"

extern void phylpbk_restore_iface();

static	uint8_t init_flag = 0;
static	uint8_t quit_flag = 0;

#ifdef DIAG_TCL
#include <tcl.h>
typedef struct dsh_global_ {

    struct Tcl_Interp*         pInterp;  /** TCL interpreter */

} dsh_global_t;
dsh_global_t   global;

static  pid_t   pid_intf1 = -1;
static  pid_t   pid_intf0 = -1;

#else
static cli_cmds_t *pcli_cmds = NULL;
#endif


/** @todo move to shell cmd DB */
int dsh_cmd_add (char * name, char * desc,
               int  (*cmd_exec)(TCL_ARGS int argc, char *argv[]) )
{
#ifdef DIAG_TCL
     assert(cmd_exec != NULL);
     Tcl_CreateCommand(global.pInterp,
                       name,
                       (Tcl_CmdProc*) cmd_exec,
                       (ClientData) NULL,
                       (Tcl_CmdDeleteProc *) NULL);
	return (TCL_OK);
#else
        cli_cmds_t *pcli = pcli_cmds;

        printf("%s: Adding %s %p\n", __func__, name, cmd_exec);
        if (pcli) {
                while (pcli->next) {
                        pcli=pcli->next;
                }

                pcli->next = MALLOC(sizeof(cli_cmds_t));
                if (!pcli->next) return (DIAG_MALLOC_ERROR);

                pcli->next->name = name;
                pcli->next->desc = desc;
                pcli->next->fptr = cmd_exec;
                pcli->next->next = NULL;
        } else  {
                pcli = MALLOC(sizeof(cli_cmds_t));
                if (!pcli) return (DIAG_MALLOC_ERROR);
                pcli->name = name;
                pcli->desc = desc;
                pcli->next = NULL;
                pcli->fptr = cmd_exec;
                pcli_cmds = pcli;
        }
        return (0);
#endif
}

#define SIGHDLR(x)				\
{						\
    if (sigaction(SIGBUS, &signal_action, NULL) == -1) {			\
        fprintf( stderr, "sigaction(SIGINT, &signal_action, NULL) failed.");	\
        return(-1);								\
    }								\
}

/**
  @callgraph
 */
#ifdef DIAG_TCL 
static void
dsh_sig_handler (int sig)
{
}

static void
dsh_sigterm_handler (int arg)
{
    printf( "Received Signal %d", arg);
    exit(0);
}

static void
dsh_exit_handler( ClientData clientData)
{

    printf("shutting down\n");
    if (pid_intf0 != -1)
    	kill(pid_intf0, SIGKILL);

    if (pid_intf1 != -1)
    	kill(pid_intf1, SIGKILL);
}

extern void brd_prompt_get(char *pbuf, int buf_sz);
static void brd_prompt_set()
{
	int fd = 0;
	char prompt[24];

	memset(prompt, '\0', sizeof(prompt));
	brd_prompt_get(prompt, sizeof(prompt));
	fd = open ("./dsh_prompt", O_WRONLY); 
	if (fd < 0) {
                printf("Unable to open ./dsh_prompt\n");
                return;
	}
	write(fd, prompt, strlen(prompt));
	close(fd);
}

static int
Dsh_Init (Tcl_Interp *interp)
{
    struct sigaction signal_action;
    int rc = 0;

    printf("Shell Initialization\n");
 
    global.pInterp = interp;
    
    /* Set the shell src script file */
    // Tcl_SetVar(interp, "tcl_rcFileName", "./dshrc", TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "tcl_rcFileName", "/etc/dshrc", TCL_GLOBAL_ONLY);


    /* Setup internals (commands not routed to the engine) */

 
    Tcl_CreateExitHandler( dsh_exit_handler, NULL);

    /* Set signal handlers */
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = 0;

    // ctrl-C signal handler
    signal_action.sa_handler = dsh_sig_handler;
    if (sigaction(SIGINT, &signal_action, NULL) == -1) {
        fprintf( stderr, "sigaction(SIGINT, &signal_action, NULL) failed.");
        return(-1);
    }

    // SIGCHLD signal handler
    signal_action.sa_handler = dsh_sig_handler;
    if (sigaction(SIGCHLD, &signal_action, NULL) == -1) {
        fprintf( stderr, "sigaction(SIGCHLD, &signal_action, NULL) failed.");
        return(-1);
    }

    // TERM signal handler
    signal_action.sa_handler = dsh_sigterm_handler;
    if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
        fprintf( stderr, "sigaction(SIGTERM, &signal_action, NULL) failed.");
        return(-1);
    }

    if ((rc = diag_main())) {
	printf("diag_main call failed\n");
	return (rc);
    }

    dsh_add_internal_commands();

    if (init_flag) {
	rc = diag_boot_init();
	if (rc) {
		printf("ERROR: SYSTEM INIT FAILED (rc=%d)\n", rc);
	}
    }

    brd_prompt_set();
    return TCL_OK; 
}

int Tcl_AppInit (Tcl_Interp *interp)
{
	if (Tcl_Init(interp) != TCL_OK)
	{
		return TCL_ERROR;
	}

    if (Dsh_Init(interp) != TCL_OK)
    {
        return TCL_ERROR;
    }

    if (quit_flag) {
	Tcl_Exit(0);
    }
    return TCL_OK;
}
#else


static uint32_t  parse_cli_command(int cli_argc, char *cli_argv[])
{
   uint32_t rc = 0;
   int  loop = diag_get_cliloop();
   cli_cmds_t *mycmd = (cli_cmds_t *)pcli_cmds;

   if (!cli_argc)
      return(0);

   if (!loop) loop = 1;
   while (mycmd && mycmd->name != NULL)
   {
      if (!(strncasecmp(mycmd->name, cli_argv[0], strlen(mycmd->name))))
      {
	 do { 
            rc = (*mycmd->fptr)(cli_argc, cli_argv);

            if (rc == DIAG_SYNTAX_ERROR)
               printf("%s", mycmd->desc);
	 } while (--loop > 0);
         return(rc);
      }
      mycmd = mycmd->next;
   }

   printf ("Did not find a match in cli commands\n");
   printf ("Please use help to identify the command list\n");

   return(0);
}

#define CLI_DELIMIT         " \t\n"

/*************************************************************** 
 *                                                             * 
 * is_alpha (char c)                                           *
 * Unable to link this function. So created one.               *
 *                                                             * 
 ***************************************************************/
int is_alpha (char c)
{
   if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
      return(1);
   else
      return(0);
}


/***************************************************************
 *                                                             *
 * is_alnum (char c)                                           *
 * Unable to link this function. So created one.               *
 *                                                             *
 ***************************************************************/
int is_alnum (char c)
{
   if (((c >= 'a') && (c <= 'z')) ||
       ((c >= 'A') && (c <= 'Z')) ||
       ((c >= '0') && (c <= '9')))
      return(1);
   else
      return(0);
}

#define MAX_CLI_ARGS 16
static uint32_t buf_args(char *pbuf)
{
   char *buf, *ptr, *cli_argv[MAX_CLI_ARGS];
   int   cli_argc, rc = 0, cnt;

   for (buf = pbuf, cnt = 0; !is_alnum(*buf) && cnt < strlen(pbuf);
       cnt++, buf++);

   if (!is_alnum(*buf)) return (DIAG_SYNTAX_ERROR);
   if (*buf == (uint8_t)NULL) return(DIAG_SYNTAX_ERROR);

   if (strtok(buf, CLI_DELIMIT) == NULL)   /* an argv[0] is required */
      return(-1);

   cli_argv[cli_argc = 0] = buf;

   while ((ptr = strtok(NULL, CLI_DELIMIT)) != NULL)
   {
      if (++cli_argc >= MAX_CLI_ARGS-1)
         return(-1);
      cli_argv[cli_argc] = ptr;
   }

   cli_argv[++cli_argc] = NULL;

   if (cli_argc) {
	if ((!strncasecmp(cli_argv[0], "exit", strlen(cli_argv[0]))) ||
	    (!strncasecmp(cli_argv[0], "quit", strlen(cli_argv[0]))))
		return (DIAG_EXIT_ERROR);
   }
   rc = parse_cli_command (cli_argc, cli_argv);

   return(rc);
}

int cli()
{
   char inpstr[256];
   int rc;
   while (1)
   {
      printf("DIAG > ");
      fgets (inpstr, sizeof(inpstr)-1, stdin);

      rc = buf_args(inpstr);
      if (rc == DIAG_EXIT_ERROR) break;
   }
#if 0
   phylpbk_restore_iface();
#endif
   return (0);
}
#endif // DIAG_TCL

extern int verbose;
int diag_sprom_rw_flag = 0;

int fru_main (int argc, char **argv)
{
	int rc = 0, cnt=0;
	int pid = -1;

	for (cnt = 1; cnt < argc; cnt++) {	
		if(!strncasecmp(argv[cnt], "cmc", strlen("cmc"))) {
			if ((pid = fork()) < 0) {
				perror("fork");
			} else if (pid == 0) {  // Child
				diag_server_proc(3);
				printf(" Exit: Server Process \n");
				return (0);
			}
                } else if(!strncasecmp(argv[cnt], "-rw", strlen("-rw"))) {
                        diag_sprom_rw_flag = 1;
		} else if(!strncasecmp(argv[cnt], "init", strlen("init"))) {
			init_flag = 1;
		} else if(!strncasecmp(argv[cnt], "quit", strlen("quit"))) {
			quit_flag = 1;
		} else {
			printf("Usage: %s init/quit/cmc\n", argv[0]);
			return (0);
		}
	}
#ifdef DIAG_TCL
	Tcl_Main(1, argv, Tcl_AppInit);
#else
	if ((rc = diag_main())) {
		printf("diag_main call failed\n");
		return (rc);
	}

	dsh_add_internal_commands();

	if (init_flag) {
		rc = diag_boot_init();
		if (rc) {
			printf("ERROR: SYSTEM INIT FAILED (rc=%d)\n", rc);
		}
	}

	if (quit_flag) {
		return (0);
	}
	cli();
#endif
	return rc;
}


int diag_clicmd_exec (char *clicmd)
{
#ifdef DIAG_TCL
	printf(" Not Implemented %s\n", __func__);
#else 
	printf("  Exec %s\n", clicmd);
	return (buf_args(clicmd));
#endif
}

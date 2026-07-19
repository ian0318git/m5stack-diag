/* $Id: jbiexprt.h,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/jbiexprt.h,v $
 */
/*******************************************************************************
 * Author: Kody Ko Ported from Altera
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 ******************************************************************************/
/****************************************************************************/
/*																			*/
/*	Module:			jbiexprt.h												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1998-2001				*/
/*																			*/
/*	Description:	Jam STAPL ByteCode Player Export Header File			*/
/*																			*/
/*	Revisions:																*/
/*																			*/
/****************************************************************************/

#ifndef INC_JBIEXPRT_H
#define INC_JBIEXPRT_H

/****************************************************************************/
/*																			*/
/*	Return codes from most JBI functions									*/
/*																			*/
/****************************************************************************/

#define JBI_RETURN_TYPE int

#define JBIC_SUCCESS            0
#define JBIC_OUT_OF_MEMORY      1
#define JBIC_IO_ERROR           2
/* #define JAMC_SYNTAX_ERROR       3 */
#define JBIC_UNEXPECTED_END     4
#define JBIC_UNDEFINED_SYMBOL   5
/* #define JAMC_REDEFINED_SYMBOL   6 */
#define JBIC_INTEGER_OVERFLOW   7
#define JBIC_DIVIDE_BY_ZERO     8
#define JBIC_CRC_ERROR          9
#define JBIC_INTERNAL_ERROR    10
#define JBIC_BOUNDS_ERROR      11
/* #define JAMC_TYPE_MISMATCH     12 */
/* #define JAMC_ASSIGN_TO_CONST   13 */
/* #define JAMC_NEXT_UNEXPECTED   14 */
/* #define JAMC_POP_UNEXPECTED    15 */
/* #define JAMC_RETURN_UNEXPECTED 16 */
/* #define JAMC_ILLEGAL_SYMBOL    17 */
#define JBIC_VECTOR_MAP_FAILED 18
#define JBIC_USER_ABORT        19
#define JBIC_STACK_OVERFLOW    20
#define JBIC_ILLEGAL_OPCODE    21
/* #define JAMC_PHASE_ERROR       22 */
/* #define JAMC_SCOPE_ERROR       23 */
#define JBIC_ACTION_NOT_FOUND  24

/****************************************************************************/
/*																			*/
/*	Macro Definitions														*/
/*																			*/
/****************************************************************************/

/*
*	For DOS port, program data is stored in a set of 16K pages, accessed
*	through a pointer table.  For 32-bit version, the buffer is continuous.
*	The macro GET_BYTE gets a single byte for either case.
*/
#define PROGRAM_PTR unsigned char *

#define GET_BYTE(x) (program[x])

#define GET_WORD(x) \
	(((((unsigned short) GET_BYTE(x)) << 8) & 0xFF00) | \
	(((unsigned short) GET_BYTE((x)+1)) & 0x00FF))

#define GET_DWORD(x) \
    (((((unsigned int) GET_BYTE(x)) << 24) & 0xFF000000) | \
    ((((unsigned int) GET_BYTE((x)+1)) << 16) & 0x00FF0000) | \
    ((((unsigned int) GET_BYTE((x)+2)) << 8) & 0x0000FF00) | \
    (((unsigned int) GET_BYTE((x)+3)) & 0x000000FF))

/****************************************************************************/
/*																			*/
/*	Structured Types														*/
/*																			*/
/****************************************************************************/

typedef struct JBI_PROCINFO_STRUCT
{
	char *name;
	unsigned char attributes;
	struct JBI_PROCINFO_STRUCT *next;
}
JBI_PROCINFO;

/****************************************************************************/
/*																			*/
/*	Global Data Prototypes													*/
/*																			*/
/****************************************************************************/

#if PORT==DOS
extern unsigned char jbi_aca_out_buffer[8192 + 1024];
#endif

extern PROGRAM_PTR jbi_program;

extern char *jbi_workspace;

extern int jbi_workspace_size;

/****************************************************************************/
/*																			*/
/*	Function Prototypes														*/
/*																			*/
/****************************************************************************/

JBI_RETURN_TYPE jbi_execute
(
	PROGRAM_PTR program,
	int program_size,
	char *workspace,
	int workspace_size,
	char *action,
	char **init_list,
	int reset_jtag,
	int *error_address,
	int *exit_code,
	int *format_version
);

JBI_RETURN_TYPE jbi_get_note
(
	PROGRAM_PTR program,
	int program_size,
	int *offset,
	char *key,
	char *value,
	int length
);

JBI_RETURN_TYPE jbi_check_crc
(
	PROGRAM_PTR program,
	int program_size,
	unsigned short *expected_crc,
	unsigned short *actual_crc
);

JBI_RETURN_TYPE jbi_get_file_info
(
	PROGRAM_PTR program,
	int program_size,
	int *format_version,
	int *action_count,
	int *procedure_count
);

JBI_RETURN_TYPE jbi_get_action_info
(
	PROGRAM_PTR program,
	int program_size,
	int index,
	char **name,
	char **description,
	JBI_PROCINFO **procedure_list
);

int jbi_jtag_io
(
	int tms,
	int tdi,
	int read_tdo
);

void jbi_message
(
	char *message_text
);

void jbi_export_integer
(
	char *key,
	int value
);

void jbi_export_boolean_array
(
	char *key,
	unsigned char *data,
	int count
);

void jbi_delay
(
	int microseconds
);

int jbi_vector_map
(
	int signal_count,
	char **signals
);

int jbi_vector_io
(
	int signal_count,
	int *dir_vect,
	int *data_vect,
	int *capture_vect
);

void *jbi_malloc
(
	unsigned int size
);

void jbi_free
(
	void *ptr
);

#endif /* INC_JBIEXPRT_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: jbiexprt.h,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.2  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

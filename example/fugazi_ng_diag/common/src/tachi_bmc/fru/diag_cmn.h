/* $Id: diag_cmn.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_cmn.h,v $
 *
 *      File:   diag_cmn.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

/////////////////////////////////////////////////////////////////
//                                                             //
// File Name   : diag_cmn.h                                    //
//                                                             //
// Description : This file includes all the common defines     //
//               used by  diagnostics.                         //
//                                                             //
/////////////////////////////////////////////////////////////////


#ifndef _DIAG_CMN_H_
#define _DIAG_CMN_H_

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include "diag_err.h"
#include "diag_reg.h"

#ifdef DIAG_TCL
#include <tcl.h>
#define TCL_ARGS	ClientData clientData, Tcl_Interp *pInterp,
#else
#define TCL_ARGS
#endif

#define MALLOC			malloc
#define FREE			free

#define BIT32(i)		(0x01 << (i))
#define BIT64(i)		(0x01ull << (i))

typedef enum _format_type_
{
	FORMAT_TYPE_DEC,
	FORMAT_TYPE_HEX,
	FORMAT_TYPE_STR,
	FORMAT_TYPE_MASK,
	FORMAT_TYPE_UNKNOWN,
} format_type_t;

typedef enum _ioctl_type_
{
	IOCTL_REGISTER_COMMANDS,
	IOCTL_SPECIAL_INIT,
	IOCTL_DEVICE_INIT,
	IOCTL_REG_RD,
	IOCTL_REG_WR,
	IOCTL_LOOPBACK,
	IOCTL_SOFT_RESET,
	IOCTL_HARD_RESET,
	IOCTL_CLEAR_RMON,
	IOCTL_BOOT_CONFIG,
	IOCTL_LINK_STATUS,
	IOCTL_EXT_LOOPBACK,
	IOCTL_VERSION,
	IOCTL_SW_ALLOC,
	IOCTL_SW_DEALLOC,
	IOCTL_UCODE_DOWNLOAD,
	IOCTL_MODE_INIT,
	IOCTL_SET_FEATURE,
	IOCTL_POLL_ERR_COUNTERS,
} ioctl_type_t;


// Add all structures here

typedef struct _cli_cmds_ cli_cmds_t;
struct _cli_cmds_
{
        char *name;
        char *desc;
	int (*fptr)(TCL_ARGS int argc, char *argv[]);
#ifndef DIAG_TCL
	cli_cmds_t *next;
#endif
};

typedef struct _test_parameters_s
{
   char *parameter;
   format_type_t format;
#ifdef HOST_GOODING
   union {
      unsigned long value;
      uint8_t  *ptr;
   } discrete;

   union {
      unsigned long value;
      uint8_t  *ptr;
   } min;

   union {
      unsigned long value;
      uint8_t  *ptr;
   } max;
#else
   union {
      uint32_t value;
      uint8_t  *ptr;
   } discrete;

   union {
      uint32_t value;
      uint8_t  *ptr;
   } min;

   union {
      uint32_t value;
      uint8_t  *ptr;
   } max;
#endif 

   uint8_t flag;

} test_parameters_t;

typedef struct _rslt_parameters_s
{
   char *parameter;
   format_type_t format;
#ifdef HOST_GOODING
   union {
      unsigned long value;
      uint8_t  *ptr;
   } discrete;
#else
   union {
      uint32_t value;
      uint8_t  *ptr;
   } discrete;
#endif

   uint8_t flag;
} test_results_t;

#define TEST_TYPE_SKIP	BIT32(0)
#define TEST_TYPE_LOCAL	BIT32(1)
#define TEST_TYPE_PASS	BIT32(2)
#define TEST_TYPE_FAIL	BIT32(3)

typedef uint32_t (*DIAG_FPTR)(test_parameters_t*, test_results_t*);
typedef struct _test_toc_s
{
  uint8_t                *name;
  uint8_t                *description; 
  uint8_t                *short_name;
  uint32_t              (*test_exec)(test_parameters_t*, test_results_t*);
  test_parameters_t       *test_params;
  test_results_t          *test_results;
  uint32_t                control;
} test_toc_t;


typedef struct _section_toc_s
{
  uint8_t            *name;
  uint8_t            *description;
  test_toc_t   	     *test_toc;
  uint32_t            control;
} section_toc_t;

typedef struct _mem_desc_s_
{
	uint8_t		*mem_name;
	uint32_t	mem_saddr;
	uint32_t	mem_size;
	uint32_t	mem_width;
	uint32_t	mem_access;
	uint32_t	mem_type;
	uint8_t		*mem_mask;
	uint32_t	(*fields)(uint8_t *data, uint32_t flag);
} mem_desc_t;

typedef struct _dev_info_s diag_dev_t;
struct _dev_info_s 
{
	char		*name;
	uint32_t	type;
	uint32_t	instance;
	uint32_t	dev_addr;
	uint16_t	dev_bus;
	uint16_t	flags;
	uint32_t	port_cnt;

	section_toc_t	*psection;
	reg_desc_t	*preg;
	reg_desc_t	**portreg;
	mem_desc_t	*pmem;
	uint32_t   	(*ioctl)(diag_dev_t *pdev, uint32_t opcode, va_list arglist);

	diag_dev_t*	p_parent;
	diag_dev_t*	p_next;
	diag_dev_t*	p_prev;
	diag_dev_t*	p_next_inst;
	diag_dev_t*	p_prev_inst;
};

typedef struct _board_info_s
{
	char		*name;
	uint32_t	dev_type;
	uint32_t	dev_cnt;
	uint32_t*	dev_addr;
	uint16_t*	dev_bus;
	section_toc_t	*psection;
	reg_desc_t	*preg;
	reg_desc_t	**portreg;
	mem_desc_t	*pmem;
	uint32_t   	(*ioctl)(diag_dev_t *pdev, uint32_t opcode, va_list arglist);
	uint32_t	port_cnt;
} board_info_t;

#endif  // _DIAG_CMN_H_

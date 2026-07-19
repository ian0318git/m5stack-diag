/* $Id: diag_main.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_main.h,v $
 *
 *      File:   diag_main.h
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
// File Name   : diag_main.h                                   //
//                                                             //
// Description : This file includes all the defines, structures//
//               enumerations and the external prototypes      //
//               required by functions defined in diag.c       // 
//                                                             //
/////////////////////////////////////////////////////////////////


#ifndef _DIAG_MAIN_H_
#define _DIAG_MAIN_H_

// Add all include files here

// system header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// user define header files
#include "diag_cmn.h"
#include "diag_dev.h"
#include "diag_sys.h"
#include "diag_ops.h"
#include "diag_glob.h"

#define DIAG_PRINT(verbose, fmt, ...) \
        if (diag_get_verbose() > (verbose)) {\
            printf(fmt, ##__VA_ARGS__); \
        }

#define DIAG_PRINT_TEST_PROGRESS 2
#define DIAG_PRINT_CONFIG 3
#define DIAG_PRINT_TEST_DEBUG 4

extern int diag_debug;
#endif  // _DIAG_MAIN_H_

/* $Id: python_util.h,v 1.2 2014/06/03 10:53:27 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/include/python_util.h,v $
 *------------------------------------------------------------------
 * Filename:    python_util.h
 *
 * Description: python common utilities header file
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PYTHON_UTIL_H__
#define __PYTHON_UTIL_H__

#include <stdbool.h>
#include <signal.h>

/* pass arguments came from python side */
extern int py_parse(int argc,char *argv[]);
extern int parse_diag_flag_value(int);

/* define essential argument numbers for test item executables*/
/* first arg       : executable */
/* second-last arg : testpass */
/* last arg        : diag flag value */
/* last and second-last which came from python script. */
#define ARGS_3 3


#endif /*__PYTHON_UTIL_H__*/
/******** History ********
$Log: python_util.h,v $
Revision 1.2  2014/06/03 10:53:27  erwu2
python menu collapsed to main trunk

Revision 1.1.2.1  2014/04/29 11:40:36  erwu2
update python file structure


$Endlog$
*/

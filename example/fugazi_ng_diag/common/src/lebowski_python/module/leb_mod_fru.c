/* $Id: leb_mod_fru.c,v 1.2 2014/06/03 10:53:29 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_fru.c,v $
 *------------------------------------------------------------------
 * Description: Enhanced error message for FRU PID and
 *              Location Strings define.
 *
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "types.h"
#include "python_error.h"
#include "leb_mod_fru.h"

/* FRU PID and Location Strings */
uchar io_pid[] = "IO-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar pvdm_pid[] = "PVDM-PID";

uchar io_loc[] = "IO";
uchar dimm_loc[] = "IO/DIMM";
uchar pvdm_loc[] = "IO/PVDM";

fru_table_t platform_fru_table[] = {
    { io_pid,        io_loc },
    { dimm_pid,      dimm_loc },
    { pvdm_pid,      pvdm_loc },
};

/******** History ******** 
$Log: leb_mod_fru.c,v $
Revision 1.2  2014/06/03 10:53:29  erwu2
python menu collapsed to main trunk

Revision 1.1.2.2  2014/04/29 11:40:38  erwu2
update python file structure

Revision 1.1.2.1  2014/04/24 08:53:49  erwu2
merge makefile and add flag example to test


$Endlog$
*/

/* $Id: linux_ntwk.h,v 1.2 2013/11/26 08:40:35 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/linux_ntwk.h,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/12
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int tftp_get (char *dir, char *file, char * port,
                     char *dest, int);
/*-------------------------------------------------
$Log: linux_ntwk.h,v $
Revision 1.2  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.2  2012/05/17 22:50:28  mcharon
support dynamic server ip

Revision 1.1  2012/05/02 22:00:04  mcharon
add api to support tftp file from remote to local


$Endlog$
*/

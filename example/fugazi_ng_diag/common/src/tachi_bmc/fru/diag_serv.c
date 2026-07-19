/* $Id: diag_serv.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_serv.c,v $
 *
 *      File:   diag_serv.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Routines to access the Redwood RTL simulation 
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include "diag_sock.h"

int diag_server_socket = -1;

int diag_server_proc (int blade_slot)
{
	int rc = 0;
	
	diag_server_socket = 
		create_server_connection(diag_get_peer_port(blade_slot), NULL);

	while (1) {
		rc = diag_recv_send(diag_server_socket);
		if (rc) break;
	}
	return (0);
}

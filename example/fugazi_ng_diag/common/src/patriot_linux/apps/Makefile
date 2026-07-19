# $Id: Makefile,v 1.1 2014/03/25 02:12:32 huanngo Exp $
# $Source:  
#------------------------------------------------------------------------------
# File Name: Makefile
#
# Description: Makefile for Patriot module
#
#      
#
# Copyright (c)2011 - 2014 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------------------- 
#



PATRIOT_FW_PATH = /auto/sp-engops/diags/pld/patriot
vpath %.c $(PATRIOT_FW_PATH) 

CC = /auto/modlnx/tools/montavista/tools/powerpc-gnu/bin/powerpc-montavista-linux-gnu-gcc

CFLAGS = -I. 
DEPS = endians.h defs.h common_utils.h platform_main.h p1021_etsec.h \
	p1021_immap.h router_if.h sgmii_defs.h qe.h patriot_intr.h

OBJ = getnum.o common_utils.o patriot_espi.o ds3170.o p1021_espi.o \
	p1021_hdlc.o p1021_i2c.o p1021_ethernet.o patriot_util.o \
	patriot_test.o patriot_main.o p1021_utils.o \
	mem_utils.o qe_ucode.o patriot_fpga_prom.o patriot_fpga_hmac.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

patriot: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

depend: dependancies

dependancies:
	rm -f dependancies
	rm -f patriot.symbols
	touch dependancies
	touch patriot.symbols

clean:
	rm -f *.o

#------------------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.1  2014/03/25 02:12:32  huanngo
# Adding patriot_linux directory to ng_diag code tree
#
# Revision 1.4  2013/05/23 20:08:05  huanngo
# Adding 'make depend' to create dependancies and patriot.symbols files
#
# Revision 1.3  2012/09/14 23:39:13  huanngo
# New FPGA files: patriot_fpga_prom.o patriot_fpga_hmac.o
#
# Revision 1.2  2012/05/08 23:52:54  huanngo
# Support SM Patriot on ngd main code tree
#
# Revision 1.1.4.5  2012/02/06 22:29:03  huanngo
# Update to not compile code using bitbake, use make with local kernel
#
# Revision 1.1.4.4  2011/12/21 23:46:32  huanngo
# Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
#
# Revision 1.1.4.3  2011/11/24 00:40:00  huanngo
# Update code for Patriot to fix bugs and support new tests
#
# Revision 1.1.4.2  2011/08/18 19:43:20  huanngo
# Update code to patriot2-branch
#
#
#------------------------------------------------------------------------------
# $Endlog$
#
#------------------------------------------------------------------------------
#



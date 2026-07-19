# $Id: Makefile,v 1.1 2014/03/25 02:12:43 huanngo Exp $
# $Source: 
#----------------------------------------------------------------------------
# File Name: Makefile
#
# Description: Makefile for Patriot KLM
#
#      
#
# Copyright (c)2011 - 2014 by Cisco Systems, Inc.
# All rights reserved.
#
#-----------------------------------------------------------------------------
#
EXTRA_CFLAGS+=  -Wno-declaration-after-statement
CC = /auto/modlnx/tools/montavista/tools/powerpc-gnu/bin/powerpc-montavista-linux-gnu-gcc
MODULES = addr_vtop klm_p1021 mem_mgr patriot_fpga patriot_framer
ARCH=powerpc
KERNEL_SOURCE=/ws/huanngo-sjc/patriot_codesign/patriots/linux-2.6.32
APPS = ../apps
SRC = .
LD = /auto/modlnx/tools/montavista/tools/powerpc-gnu/bin/../lib/gcc/powerpc-montavista-linux-gnu/4.4.1/../../../../powerpc-montavista-linux-gnu/bin/ld
default: all

obj-m += addr_vtop.o
obj-m += klm_p1021.o
obj-m += mem_mgr.o
obj-m += patriot_fpga.o
obj-m += patriot_framer.o
patriot_framer-objs := patriot_framer_spi.o patriot_framer_main.o

BUILD = $(MODULES:%=%.ko)


clean::
	rm -f $(BUILD) *.o *.ko *.mod.c *.mod.o *~ .*.cmd Module.symvers
	rm -rf .tmp_versions

all:
	$(MAKE) -C $(KERNEL_SOURCE) ARCH=$(ARCH) SUBDIRS=`pwd` CC=${CC}  modules

#%.ko:  $(SRC)/addr_vtop.c $(SRC)/klm_p1021.c $(SRC)/mem_mgr.c \
	$(SRC)/patriot_fpga.c \
	$(SRC)/mem_mgr.h $(SRC)/klm_defs.h $(SRC)/patriot_fpga.h \
	$(APPS)/patriot_main.h $(APPS)/patriot_intr.h \
	$(APPS)/p1021_immap.h $(APPS)/qe.h $(APPS)/p1021_etsec.h \
	$(APPS)/router_if.h $(APPS)/sgmii_defs.h 


#------------------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.1  2014/03/25 02:12:43  huanngo
# Adding patriot_linux directory to ng_diag code tree
#
# Revision 1.2  2012/05/08 23:52:56  huanngo
# Support SM Patriot on ngd main code tree
#
# Revision 1.1.4.6  2012/03/16 23:38:17  huanngo
# Compile with the kernel which supports code signing
#
# Revision 1.1.4.5  2012/03/13 13:27:50  steja
# Support Framer Interrupt
#
# Revision 1.1.4.4  2012/02/06 22:29:04  huanngo
# Update to not compile code using bitbake, use make with local kernel
#
# Revision 1.1.4.3  2011/12/21 23:46:32  huanngo
# Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
#
# Revision 1.1.4.2  2011/08/18 19:43:27  huanngo
# Update code to patriot2-branch
#
# Revision 1.1.2.6  2011/07/21 20:06:27  huanngo
# Add support for mem_mgr.ko
#
# Revision 1.1.2.5  2011/07/19 06:11:35  huanngo
# Update code per code review comments
#
# Revision 1.1.2.4  2011/06/28 06:27:56  huanngo
# Update code to support Patriot SM
#
# Revision 1.1.2.3  2011/05/21 01:01:49  huanngo
# Support memory test, I2C interface and Montavista Linux
#
# Revision 1.1.2.2  2011/05/03 00:25:47  huanngo
# Fix the header comments
#
# Revision 1.1.2.1  2011/05/02 23:33:35  huanngo
# Update code to support Patriot module side
#
#------------------------------------------------------------------------------
# $Endlog$
#
#------------------------------------------------------------------------------
#







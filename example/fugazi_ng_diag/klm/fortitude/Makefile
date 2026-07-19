# $Id: Makefile,v 1.4 2012/07/11 23:33:07 ywen Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/fortitude/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Fortitude KLM.
#
# Christine Wen -- Oct. 2011
#
# Copyright (c)2011 - 2012 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************


export MVISTA_LICENSE_FILE=@blr-lic-lnx1
export CROSS_COMPILE=mips-montavista-linux-gnu-

LINUX_TOOL_CHAIN_PATH := /auto/modlnx/tools/montavista_mvl6/tools/mips-gnu/bin
LINUX_SET_PATH = PATH=$(LINUX_TOOL_CHAIN_PATH):$(PATH);

MAKE = /router/bin/make

kernel_source := $(GNUSTEP_APPS)/fortitude/linux-2.6.28/

EXTRA_CFLAGS += -Wdeclaration-after-statement -DLINUX_KLM 

SRCDIR = ..

default: all
obj-m      :=  $(SRCDIR)/addr_vtop.o klm_wp3.o pmc-adapter.o max4422.o

all:
	$(LINUX_SET_PATH) $(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;
	mv $(SRCDIR)/*.ko .
	rm -rf $(SRCDIR)/*.mod* $(SRCDIR)/*.o

install: all
	cp addr_vtop.ko $(GNUSTEP_APPS)/fortitude/userspace/fortitude
	cp klm_wp3.ko $(GNUSTEP_APPS)/fortitude/userspace/fortitude
	cp pmc-adapter.ko $(GNUSTEP_APPS)/fortitude/userspace/fortitude
	cp max4422.ko $(GNUSTEP_APPS)/fortitude/userspace/fortitude

clean:
	rm -f *.mod.c *.o *.ko 

#/******** History ********
#$Log: Makefile,v $
#Revision 1.4  2012/07/11 23:33:07  ywen
#Add I2C driver.
#
#Revision 1.3  2012/05/31 00:09:59  ywen
#Improve Makefile to provide kernel directory dynamically.
#
#Revision 1.2  2012/05/22 23:18:18  ywen
#Improve Makefile to be able to build KLM from any Linux server.
#
#Revision 1.1.1.1  2012/03/23 23:02:11  ptong
#Initial archive of ng_diag module
#
#Revision 1.1.2.1  2012/03/05 17:17:20  ywen
#Initial checkin to support Fortitude.
#
#
#
#$Endlog$
#*/

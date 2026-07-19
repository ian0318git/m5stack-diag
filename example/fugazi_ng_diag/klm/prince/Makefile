# $Id: Makefile,v 1.1 2013/11/13 06:02:04 xiaoyizh Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/prince/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Prince KLM.
#
# Xiaoying Zhang -- Dec. 2012
#
# Copyright (c)2012 - 2013 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************


export ARCH=arm
export MVISTA_LICENSE_FILE=@blr-lic-lnx1
export CROSS_COMPILE=arm-xilinx-linux-gnueabi-

LINUX_TOOL_CHAIN_PATH := /auto/modlnx/tools/xilinx/CodeSourcery/Sourcery_CodeBench_Lite_for_Xilinx_GNU_Linux/bin
LINUX_SET_PATH = PATH=$(LINUX_TOOL_CHAIN_PATH):$(PATH);

MAKE = /router/bin/make

kernel_source := $(GNUSTEP_APPS)/prince/linux-xlnx

EXTRA_CFLAGS += -Wdeclaration-after-statement -DLINUX_KLM -DARM

SRCDIR = ..
 
default: all
obj-m      :=  $(SRCDIR)/addr_vtop.o klm_prc.o

all:
	$(LINUX_SET_PATH) $(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;
	mv $(SRCDIR)/*.ko .
	rm -rf $(SRCDIR)/*.mod* $(SRCDIR)/*.o

install: all
	cp addr_vtop.ko $(GNUSTEP_APPS)/prince/userspace/src
	cp klm_prc.ko $(GNUSTEP_APPS)/prince/userspace/src

clean:
	rm -f *.mod.c *.o *.ko 

#/******** History ********
#$Log: Makefile,v $
#Revision 1.1  2013/11/13 06:02:04  xiaoyizh
#Initial check in for Prince klm.
#
#
#
#$Endlog$
#*/

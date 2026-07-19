# $Id: Makefile,v 1.3 2018/01/17 11:48:09 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/reva/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Reva KLM.
#
# Copyright (c)2015-2018 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************


export ARCH=arm
export MVISTA_LICENSE_FILE=@blr-lic-lnx1
export CROSS_COMPILE=arm-xilinx-linux-gnueabi-

LINUX_TOOL_CHAIN_PATH := /auto/modlnx/tools/xilinx/CodeSourcery/Sourcery_CodeBench_Lite_for_Xilinx_GNU_Linux/bin
LINUX_SET_PATH = PATH=$(LINUX_TOOL_CHAIN_PATH):$(PATH);

MAKE = /router/bin/make

kernel_source := $(GNUSTEP_APPS)/reva/linux-xlnx

EXTRA_CFLAGS += -Wdeclaration-after-statement -DLINUX_KLM -DARM -DREVA

SRCDIR = ..
 
default: all
obj-m      :=  $(SRCDIR)/addr_vtop.o klm_prc.o

all:
	$(LINUX_SET_PATH) $(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;
	mv $(SRCDIR)/*.ko .
	rm -rf $(SRCDIR)/*.mod* $(SRCDIR)/*.o

install: all
	cp addr_vtop.ko $(GNUSTEP_APPS)/reva/userspace/src
	cp klm_prc.ko $(GNUSTEP_APPS)/reva/userspace/src

clean:
	rm -f *.mod.c *.o *.ko 

#/******** History ********
#$Log: Makefile,v $
#Revision 1.3  2018/01/17 11:48:09  iachang
#CSCvh19145 : Fixed Reva firmware compiler issue. Reva didn't support new FPGA.
#
#Revision 1.2  2016/05/06 05:20:45  umlin
#Reva: Commit Reva module side KLM to main trunk
#

#$Endlog$
#*/

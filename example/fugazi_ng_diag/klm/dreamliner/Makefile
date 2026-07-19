# $Id: Makefile,v 1.2 2018/05/18 09:25:02 alpeng Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/dreamliner/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Dreamliner KLM.
#
# Christine Wen -- Jan. 2014
#
# Copyright (c)2014 - 2018 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************

MAKE = /router/bin/make

#ifndef $(KSRC)
KSRC := /auto/sword/victory-kernel.daily
#KSRC := /ws/mcharon-sjc/ads-linux-stable-git
#KSRC := /ws/ywen-sjc/victory-kernel
#KSRC := /ws/ptong-sjc/git/linux-stable-git/
#endif

EXTRA_CFLAGS += -Wdeclaration-after-statement -DCONFIG_CISCO_LINUX -DLINUX_KLM -DX86 

KLMTARGET = nim_dm.ko

default: all
obj-m      := nim_dm.o 

$(KLMTARGET): 
	$(MAKE) -C $(KSRC) SUBDIRS=`pwd`  modules;

all: nep $(KLMTARGET) 


NEP_KLMTARGET = nim_dm_nep.ko
NEP_KSRC := /auto/neptune/nepfs/nep/linux-4.9.10
#expired, left for backup
#NEP_KSRC := /auto/neptune/nepfs/nep/linux-3.19

nep: 
	$(MAKE) -C $(NEP_KSRC) SUBDIRS=`pwd`  modules;
	mv $(KLMTARGET) $(NEP_KLMTARGET)

install: $(KLMTARGET) install.sh
	sh install.sh
	touch install.sh

clean:
	rm -f *.mod.c *.o *.ko .*.cmd Module.symvers modules.order

.PHONY: nep

#/******** History ********
#$Log: Makefile,v $
#Revision 1.2  2018/05/18 09:25:02  alpeng
# Neptune merge to trunk with tag <neptune-branch-0518>
#
#Revision 1.1.32.3  2017/06/14 06:43:57  alpeng
#support 4.9 kernel
#
#Revision 1.1.32.2  2017/01/13 02:22:59  alpeng
#add nep to default(all), it will generate dreamliner ko for USD and nep
#
#Revision 1.1.32.1  2016/12/29 03:24:36  alpeng
#add 'nep' for rename nim_dm.ko to nim_dm_nep.ko automatically
#
#Revision 1.1  2014/09/23 20:37:49  ywen
#Add support for Dreamliner.
#
#
#$Endlog$
#*/

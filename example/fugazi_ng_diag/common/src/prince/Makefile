# $Id: Makefile,v 1.6 2013/11/26 08:40:38 hroni Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Prince NIM application.
#
# Xiaoying Zhang -- Nov. 2012
#
# Copyright (c)2012 - 2013 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************

default: all

#export MVISTA_LICENSE_FILE=@blr-lic-lnx1
export MVISTA_LICENSE_FILE=27200@sjc-lds-lic3:/opt/MontaVista/bin/mvista.lic,/auto/xe-distro/mvlicenses/sjc/mvista_sjc.lic

LINUX_PATH = /auto/modlnx/tools/montavista_xilinx/tools/arm-gnueabi/
LINUX_TOOL_CHAIN_PATH := $(LINUX_PATH)/bin
LINUX_SET_PATH = PATH=$(LINUX_TOOL_CHAIN_PATH);
CROSS_COMPILE = arm-montavista-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
OBJDUMP = $(CROSS_COMPILE)objdump

IROOT           = ../../..
SRCDIR          = .

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
INCDIR          = $(IROOT)/common/include
KLM_DIR 	= $(IROOT)/klm

#
# Path to fpga images
#

PLDDIR		= /auto/sp-engops/diags/pld/prince

#
# Include paths for compiler
#

INC	= \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(DEV_DRV) \
	-I$(KLM_DIR)

#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

COM_OBJS = \
	menu.o \
	common_utils.o \
	shcmd.o \
	alias.o \
	monitor.o \
	parsetoken.o \
	builtin.o \
	query.o \
	memops.o \
	testmem.o \
	linux_stub.o \
	linux_mem.o\
	linux_memory_tester.o \
	linux_memory_tester_utils.o \
	linux_error.o \
	linux_api.o\
	bit_swaps.o\
	print_hex.o

COM_BASE = \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/shcmd.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/monitor.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_memory_tester.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/bit_swaps.c

SRC_OBJS = \
	prince_main.o \
	stubs.o \
	diag.o \
	prince_scc.o \
	prince_scc_diag.o \
	prince_ge_mac.o \
	prince_ge_dma.o \
	prince_ge_phy.o \
	prince_eth_pkt.o \
	zynq_i2c.o \
	i2c_util.o \
	zynq_qspi.o \
	qspi_util.o \
        prince_xp_ugd_fw.o

SRC_BASE = \
	$(SRCDIR)/prince_main.c \
	$(SRCDIR)/stubs.c \
	$(SRCDIR)/diag.c\
	$(SRCDIR)/prince_scc.c \
	$(SRCDIR)/prince_scc_diag.c \
	$(SRCDIR)/prince_ge_mac.c \
	$(SRCDIR)/prince_ge_dma.c \
	$(SRCDIR)/prince_ge_phy.c \
	$(SRCDIR)/prince_eth_pkt.c \
        $(SRCDIR)/zynq_i2c.c \
	$(SRCDIR)/i2c_util.c \
	$(SRCDIR)/zynq_qspi.c \
	$(SRCDIR)/qspi_util.c \
        $(SRCDIR)/prince_xp_ugd_fw.c

NET_SRC = \
	$(SRC_BASE) \
	$(COM_BASE) \
	$(DEV_CORE_BASE)

NET_OBJS = \
	$(SRC_OBJS) \
	$(COM_OBJS) \
	$(DEV_CORE_OBJS)

vpath %.c \
	$(SRCDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(PLDDIR): \

vpath %.h \
	$(SRCDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(WDDI_API_INC):

DEFS = -D__NO_STRING_INLINES -Dlinux -DLINUX_APP -DPRINCE -D_WP_MODULE_EFM_BONDING_OVER_ENET_G999_ -DPMC4358
LDFLAGS_GLOBAL += -g -static -msoft-float -nostdinc -L$(LINUX_PATH)/lib/gcc/arm-montavista-linux-gnueabi/4.4.1
CFLAGS= -c -O0 -fsigned-char -msoft-float -g -march=armv7-a  -mfpu=neon -fomit-frame-pointer -fno-builtin -pipe -Wa, $(INC) $(DEFS)
COMPILE = $(CC) $(CFLAGS) -o $@ $<

%.o : %.c
	$(LINUX_SET_PATH) $(COMPILE)

TARGET = prince

all: $(TARGET)

$(TARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh PRINCE 
	$(LINUX_SET_PATH) $(CC) $(CFLAGS) -c banner.c
	$(LINUX_SET_PATH) $(CC) $(LDFLAGS_GLOBAL) -v -o $@ $(NET_OBJS) banner.o -lpthread -lrt -v

install: $(TARGET)
	cp $(TARGET) $(GNUSTEP_APPS)/prince/userspace/prince

clean:
	rm -rf $(NET_OBJS) *.d $(TARGET)

#/******** History ********
#$Log: Makefile,v $
#Revision 1.6  2013/11/26 08:40:38  hroni
#fix compiler warning
#
#Revision 1.5  2013/07/31 09:26:40  xiaoyizh
#Add new source file and flag -lrt.
#
#Revision 1.4  2013/07/16 03:00:10  liwwang
#update to support prince_xp_ugd_fw.c
#
#Revision 1.3  2013/06/25 09:17:01  xiaoyizh
#Update Monta Vista license server.
#
#Revision 1.1  2013/04/19 07:17:52  xiaoyizh
#Initial check in for Prince NIM.
#
#$Endlog$
#*/

# $Id: Makefile,v 1.5 2018/08/02 09:35:01 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Pluggable Serial application.
#
# Copyright (c)2015-2018 by Cisco Systems, Inc.
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
SRCDIR          = ../prince
REVADIR         = ../reva
PLUGSERDIR      = ./

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
# *** Need to check with CRDC Echo Pluggable serial FPGA file location ***
PLDDIR		= /auto/sp-engops/diags/pld/reva

#
# Include paths for compiler
#

INC	= \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(DEV_DRV) \
	-I$(KLM_DIR) \
      -I$(REVADIR) \
      -I$(PLUGSERDIR) 
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
	plugser_main.o \
	stubs.o \
	diag.o \
	plugser_scc.o \
	plugser_scc_diag.o \
	prince_ge_mac.o \
	reva_ge_dma.o \
	reva_ge_phy.o \
	prince_eth_pkt.o \
	zynq_i2c.o \
	i2c_util.o \
	zynq_qspi.o \
	qspi_util.o \
	reva_sysinfo.o \
	plugser_sb_upgrade.o

SRC_BASE = \
	$(PLUGSERDIR)/plugser_main.c \
	$(SRCDIR)/stubs.c \
	$(PLUGSERDIR)/diag.c\
	$(PLUGSERDIR)/plugser_scc.c \
	$(PLUGSERDIR)/plugser_scc_diag.c \
	$(SRCDIR)/prince_ge_mac.c \
	$(REVADIR)/reva_ge_dma.c \
	$(REVADIR)/reva_ge_phy.c \
	$(SRCDIR)/prince_eth_pkt.c \
    $(SRCDIR)/zynq_i2c.c \
	$(REVADIR)/i2c_util.c \
	$(SRCDIR)/zynq_qspi.c \
	$(SRCDIR)/qspi_util.c \
	$(REVADIR)/reva_sysinfo.c \
	$(PLUGSERDIR)/plugser_sb_upgrade.c

NET_SRC = \
	$(SRC_BASE) \
	$(COM_BASE) \
	$(DEV_CORE_BASE)

NET_OBJS = \
	$(SRC_OBJS) \
	$(COM_OBJS) \
	$(DEV_CORE_OBJS)

vpath %.c \
	$(PLUGSERDIR): \
	$(REVADIR): \
	$(SRCDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(PLDDIR): \
	
vpath %.h \
	$(SRCDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(WDDI_API_INC):

DEFS = -D__NO_STRING_INLINES -Dlinux -DLINUX_APP -DPRINCE -D_WP_MODULE_EFM_BONDING_OVER_ENET_G999_ -DPMC4358 -DPLUG_SER
LDFLAGS_GLOBAL += -g -static -msoft-float -nostdinc -L$(LINUX_PATH)/lib/gcc/arm-montavista-linux-gnueabi/4.4.1
CFLAGS= -c -O0 -fsigned-char -msoft-float -g -march=armv7-a  -mfpu=neon -fomit-frame-pointer -fno-builtin -pipe -Wa, $(INC) $(DEFS)
COMPILE = $(CC) $(CFLAGS) -o $@ $<

%.o : %.c
	$(LINUX_SET_PATH) $(COMPILE)

TARGET = plugser

all: $(TARGET)

$(TARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh PLUGSER 
	$(LINUX_SET_PATH) $(CC) $(CFLAGS) -c banner.c
	$(LINUX_SET_PATH) $(CC) $(LDFLAGS_GLOBAL) -v -o $@ $(NET_OBJS) banner.o -lpthread -lrt -v

install: $(TARGET)
	cp $(TARGET) $(GNUSTEP_APPS)/prince/userspace/src/$(TARGET)

clean:
	rm -rf $(NET_OBJS) *.d $(TARGET)

#/******** History ********
#$Log: Makefile,v $
#Revision 1.5  2018/08/02 09:35:01  iachang
#Merge Pluggable Serial from branch star-branch-c9xx to main trunk
#
#Revision 1.4  2018/03/27 12:46:36  hondwang
#Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER
#
#Revision 1.3.2.2  2018/03/13 06:42:09  iachang
#Support golden image protection with Micron flash
#
#Revision 1.3.2.1  2018/03/02 03:13:35  iachang
#Support QSPI firmware and golden image protection
#Support QSPI lock test
#
#Revision 1.3  2018/02/09 09:17:33  hondwang
#Merge Star branch star-branch-c9xx to main trunk
#
#Revision 1.2.2.2  2018/01/20 06:54:52  hondwang
#prepare merge star-branch-c9xx to main trunk
#
#Revision 1.2  2018/01/20 04:58:56  hondwang
#prepare merge star-branch-c9xx to main trunk
#
#Revision 1.1.4.4  2017/10/02 03:18:31  iachang
#Modify the PlaggSer install location.
#
#Revision 1.1.4.3  2017/08/22 04:13:17  lucywang
#add firmware upgrade for 7007 FPGA
#
#Revision 1.1.4.2  2017/08/08 07:43:37  hondwang
#add pluggable serial for star-branch-c9xx
#
#Revision 1.1.2.1  2017/07/31 10:49:58  lucywang
#add pluggable serial code of host and module
#

#
#$Endlog$
#*/

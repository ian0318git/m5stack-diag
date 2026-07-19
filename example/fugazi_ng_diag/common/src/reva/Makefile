# $Id: Makefile,v 1.5 2018/07/23 06:45:00 easochen Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Reva NIM application.
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
REVADIR         = ./

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

PLDDIR		= /auto/sp-engops/diags/pld/reva

#
# Include paths for compiler
#

INC	= \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(DEV_DRV) \
	-I$(KLM_DIR) \
        -I$(REVADIR) 
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
	reva_main.o \
	stubs.o \
	diag.o \
	prince_ge_mac.o \
	reva_ge_dma.o \
	reva_ge_phy.o \
	prince_eth_pkt.o \
	zynq_i2c.o \
	i2c_util.o \
	zynq_qspi.o \
	qspi_util.o \
        reva_sboot_upgrade.o \
        reva_scc.o \
        reva_scc_diag.o \
        reva_sysinfo.o

SRC_BASE = \
	$(REVADIR)/reva_main.c \
	$(SRCDIR)/stubs.c \
	$(REVADIR)/diag.c\
	$(REVADIR)/reva_scc.c \
	$(REVADIR)/reva_scc_diag.c \
	$(SRCDIR)/prince_ge_mac.c \
	$(SRCDIR)/reva_ge_dma.c \
	$(SRCDIR)/reva_ge_phy.c \
	$(SRCDIR)/prince_eth_pkt.c \
        $(SRCDIR)/zynq_i2c.c \
	$(REVADIR)/i2c_util.c \
	$(SRCDIR)/zynq_qspi.c \
	$(SRCDIR)/qspi_util.c \
        $(PLDDIR)/reva_sboot_upgrade.c \
        $(REVADIR)/reva_sysinfo.c

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

DEFS = -D__NO_STRING_INLINES -Dlinux -DLINUX_APP -DPRINCE -D_WP_MODULE_EFM_BONDING_OVER_ENET_G999_ -DPMC4358 -DREVA 
LDFLAGS_GLOBAL += -g -static -msoft-float -nostdinc -L$(LINUX_PATH)/lib/gcc/arm-montavista-linux-gnueabi/4.4.1
CFLAGS= -c -O0 -fsigned-char -msoft-float -g -march=armv7-a  -mfpu=neon -fomit-frame-pointer -fno-builtin -pipe -Wa, $(INC) $(DEFS)
COMPILE = $(CC) $(CFLAGS) -o $@ $<

%.o : %.c
	$(LINUX_SET_PATH) $(COMPILE)

TARGET = reva

all: $(TARGET)

$(TARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh REVA 
	$(LINUX_SET_PATH) $(CC) $(CFLAGS) -c banner.c
	$(LINUX_SET_PATH) $(CC) $(LDFLAGS_GLOBAL) -v -o $@ $(NET_OBJS) banner.o -lpthread -lrt -v

install: $(TARGET)
	cp $(TARGET) $(GNUSTEP_APPS)/reva/userspace/src/reva

clean:
	rm -rf $(NET_OBJS) *.d $(TARGET)

#/******** History ********
#$Log: Makefile,v $
#Revision 1.5  2018/07/23 06:45:00  easochen
#Reva: Add new board_id and firmware for 4G DDR
#
#Revision 1.4  2018/01/17 11:48:07  iachang
#CSCvh19145 : Fixed Reva firmware compiler issue. Reva didn't support new FPGA.
#
#Revision 1.3  2016/05/09 05:51:57  umlin
#Reva:
#common/src/reva/diag.c        => Change wording in comment
#common/src/reva/reva_ge_dma.c => Change wording in comment
#common/src/reva/i2c_util.c    => Change wording for function name
#common/src/reva/i2c_util.h    => Change wording for function name
#common/src/reva/reva_ge_phy.c => Polling to check copper link status
#utils/banner.sh               => Add banner BOX_TYPE for Reva
#common/src/reva/Makefile      => Update FPGA bin file: reva_sb_mboot_rel
#
#Revision 1.2  2016/05/06 03:43:52  umlin
#Reva: Commit Reva module side diag codes to main trunk
#
#
#$Endlog$
#*/

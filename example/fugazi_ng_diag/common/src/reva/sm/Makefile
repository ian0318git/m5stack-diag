# $Id: Makefile,v 1.5 2018/01/17 11:48:08 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Reva-SM application.
#
# Copyright (c)2016-2018 by Cisco Systems, Inc.
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

IROOT           = ../../../..
SRCDIR          = ../../prince
REVADIR         = ../
REVASMDIR         = ./

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

PLDDIR		= /auto/sp-engops/diags/pld/revasm

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
        reva-64_015_SBOOT_upgrade_clone_2017_0327.o \
        reva-64_100T_clone_2017_0327.o \
        reva_scc.o \
        reva_scc_diag.o \
        reva_sysinfo.o \
        spi_util.o

SRC_BASE = \
	$(REVASMDIR)/reva_main.c \
	$(SRCDIR)/stubs.c \
	$(REVASMDIR)/diag.c\
	$(REVASMDIR)/reva_scc.c \
	$(REVASMDIR)/reva_scc_diag.c \
	$(SRCDIR)/prince_ge_mac.c \
	$(SRCDIR)/reva_ge_dma.c \
	$(SRCDIR)/reva_ge_phy.c \
	$(SRCDIR)/prince_eth_pkt.c \
        $(SRCDIR)/zynq_i2c.c \
	$(REVASMDIR)/i2c_util.c \
	$(SRCDIR)/zynq_qspi.c \
	$(SRCDIR)/qspi_util.c \
        $(PLDDIR)/reva-64_015_SBOOT_upgrade_clone_2017_0327.c \
        $(PLDDIR)/reva-64_100T_clone_2017_0327.c \
        $(REVADIR)/reva_sysinfo.c \
        $(REVASMDIR)/spi_util.c

NET_SRC = \
	$(SRC_BASE) \
	$(COM_BASE) \
	$(DEV_CORE_BASE)

NET_OBJS = \
	$(SRC_OBJS) \
	$(COM_OBJS) \
	$(DEV_CORE_OBJS)

vpath %.c \
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

DEFS = -D__NO_STRING_INLINES -Dlinux -DLINUX_APP -DPRINCE -D_WP_MODULE_EFM_BONDING_OVER_ENET_G999_ -DPMC4358 -DREVA
LDFLAGS_GLOBAL += -g -static -msoft-float -nostdinc -L$(LINUX_PATH)/lib/gcc/arm-montavista-linux-gnueabi/4.4.1
CFLAGS= -c -O0 -fsigned-char -msoft-float -g -march=armv7-a  -mfpu=neon -fomit-frame-pointer -fno-builtin -pipe -Wa, $(INC) $(DEFS)
COMPILE = $(CC) $(CFLAGS) -o $@ $<

%.o : %.c
	$(LINUX_SET_PATH) $(COMPILE)

TARGET = reva

all: $(TARGET)

$(TARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh REVA_SM 
	$(LINUX_SET_PATH) $(CC) $(CFLAGS) -c banner.c
	$(LINUX_SET_PATH) $(CC) $(LDFLAGS_GLOBAL) -v -o $@ $(NET_OBJS) banner.o -lpthread -lrt -v

install: $(TARGET)
	cp $(TARGET) $(GNUSTEP_APPS)/reva/userspace/src/reva

clean:
	rm -rf $(NET_OBJS) *.d $(TARGET)

#/******** History ********
#$Log: Makefile,v $
#Revision 1.5  2018/01/17 11:48:08  iachang
#CSCvh19145 : Fixed Reva firmware compiler issue. Reva didn't support new FPGA.
#
#Revision 1.4  2017/04/17 07:35:01  umlin
#Reva-SM: TFTP upgrade 7015 and A100T by using image in the firmware
#
#Revision 1.3  2017/03/20 09:44:02  umlin
#Reva-SM:
#Start from P1C-2nd:
#1. Change board id from 0x2 to 0x0 for Reva-SM-2G and 0x1 for Reva-SM-4G
#2. Including secure boot image upgrade
#
#Revision 1.2  2017/03/16 05:20:22  umlin
#Reva-SM: Commit Reva-SM module side diag codes to main trunk
#
#Revision 1.1.2.7  2017/02/17 10:28:04  umlin
#Reva-SM: New FPGA image for P1C build. Support A100T Done, A100T version and SPI_SEL pins and display in reva subsystem info.
#
#Revision 1.1.2.6  2016/12/16 07:01:09  umlin
#Reva-SM: Add MAC loopback test back, need to upgrade FPGA V1.1 using upgrade util.
#
#Revision 1.1.2.5  2016/12/08 07:31:51  umlin
#Reva-SM: Depend on FPGA, this version still need to skip MAC loopback and LED test.
#
#Revision 1.1.2.4  2016/12/05 07:38:27  umlin
#Reva-SM: 1. Support new LEDs and MAC loopback. 2.New 7z015 image upgrade.
#
#Revision 1.1.2.3  2016/12/02 09:00:01  umlin
#Reva-SM: 1. A100T flash upgrade and SPI flash util. 2. Skip MAC Loopback test, need to change Prince code in this moment. 3. Support SM3 & SM4
#
#Revision 1.1.2.2  2016/11/02 18:00:16  umlin
#Reva-SM: Add 1.2V and 1.0V voltage margin. Fine-tune voltage margin value for Reva-SM
#
#Revision 1.1.2.1  2016/10/18 22:05:18  umlin
#Reva-SM: SM-Module side diag, refer to FPGA 24~63 ports memory mapping to add those async loopback test. Removed PPP loopback function because of FPGA PPP logic is removed due to FPGA resource constraint.
#
#
#$Endlog$
#*/

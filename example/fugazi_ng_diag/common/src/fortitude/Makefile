# $Id: Makefile,v 1.14 2014/08/18 06:38:48 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/Makefile,v $
# **********************************************************************
# File Name: Makefle
#
# Description: Makefile for Fortitude NPU application.
#
# Christine Wen -- Oct. 2011
#
# Copyright (c)2011 - 2014 by Cisco Systems, Inc.
# All rights reserved.
#
# ***********************************************************************

default: all

export MVISTA_LICENSE_FILE=27200@sjc-lds-lic3:/opt/MontaVista/bin/mvista.lic,/auto/xe-distro/mvlicenses/sjc/mvista_sjc.lic

LINUX_PATH = /auto/modlnx/tools/montavista_mvl6/tools/mips-gnu/
LINUX_TOOL_CHAIN_PATH := $(LINUX_PATH)/bin
LINUX_SET_PATH = PATH=$(LINUX_TOOL_CHAIN_PATH);
CROSS_COMPILE = mips-montavista-linux-gnu-

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
PMC4359_SRC     = $(DEV_DRV)/dev_pmc_4359
KLM_DIR 	= $(IROOT)/klm

#
# Path to fpga images and WDDI libraries
#

PLDDIR		= /auto/sp-engops/diags/pld/fortitude

# defines for linking with WDDI
WDDS_DIR = $(GNUSTEP_APPS)/wintegra/wp3_spo/wdds_4_2
WPI_HOST = linux
WPI_TARGET = mips_linux
WPI_BOARD = wds_sl
WPI_HW_DEVICE = winpath3
WDDS_LIB_DIR = $(WDDS_DIR)/objects/$(WPI_HOST)/$(WPI_TARGET)_$(WPI_HW_DEVICE)/lib
WDDS_BOARD_LIB_DIR = $(WDDS_DIR)/objects/$(WPI_HOST)/$(WPI_TARGET)_$(WPI_HW_DEVICE)/$(WPI_BOARD)/lib

DPS_IMAGE_OBJ = $(WDDS_LIB_DIR)/wpi_dps_image_backhaul.o
PPS_IMAGE_OBJ = $(WDDS_LIB_DIR)/wpi_pps_image_iwgp_parser.o

WDDI_LIB = \
	$(WDDS_LIB_DIR)/wpi_wddi.a \
	$(WDDS_LIB_DIR)/wpi_hw_device.a \
	$(WDDS_BOARD_LIB_DIR)/wpx_phy.a \
	$(WDDS_BOARD_LIB_DIR)/wpx_board.a \
	$(WDDS_LIB_DIR)/winutil.a \
	$(WDDS_LIB_DIR)/platform.a 

WDDI_API_INC = $(WDDS_DIR)/wddi/sources/api
WDDI_PLAT_TARGET_DIR = $(WDDS_DIR)/platform/$(WPI_TARGET)_$(WPI_HW_DEVICE)/include
WDDI_PLAT_TARGET_DIR2 = $(WDDS_DIR)/platform/include
WDDI_PHY_INC = $(WDDS_DIR)/wddi/phy
WDDI_BOARD_INC = $(WDDS_DIR)/wddi/board
WDDI_UTILITY_INC = $(WDDS_DIR)/wddi/board/$(WPI_BOARD)_$(WPI_HW_DEVICE)
WDDI_WINUTIL_INC = $(WDDS_DIR)/wddi/sources/winutil/include

LINUXTRA=-muclibc -lpthread

#
# Include paths for compiler
#

INC	= \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(DEV_DRV) \
	-I$(KLM_DIR) \
	-I$(PMC4359_SRC) \
	-I$(WDDI_API_INC) \
	-I$(WDDI_PHY_INC) \
	-I$(WDDI_BOARD_INC) \
	-I$(WDDI_PLAT_TARGET_DIR) \
	-I$(WDDI_PLAT_TARGET_DIR2) \
	-I$(WDDS_DIR)/wddi/sources/veneer \
	-I$(WDDI_UTILITY_INC) \
	-I$(WDDS_DIR)/include \
	-I$(WDDS_DIR)/wddi/target/$(WPI_TARGET)_$(WPI_HW_DEVICE)/include \
	-I$(WDDI_WINUTIL_INC) \
	-I$(WDDS_DIR)/wddi/sources/include/core \
	-I$(WDDS_DIR)/wddi/sources

#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_OBJS = \
	dev_4359.o

DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

DEV_BASE = \
	$(PMC4359_SRC)/dev_4359.c

DEV_HFILES = \
	$(PMC4359_SRC)/dev_4359.h

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
	bit_swaps.o \
	linux_api.o \
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
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/linux_api.c

SRC_OBJS = \
	fortitude_main.o \
	diag.o \
	stubs.o \
	fortitude_framer.o \
	fortitude_fpga.o \
	fortitude_tests.o \
	fortitude_npu.o \
	bootflash_test.o \
	fortitude_fpga_image_lx25.o \
	fortitude_fpga_image_lx9.o \
	fortitude_fpga_image_lx4.o \
	fortitude_2p_fpga_ugd.o \
	fortitude_4p_fpga_ugd.o \
	fortitude_8p_fpga_ugd.o \
	fortitude_2p_fpga_gld.o \
	fortitude_4p_fpga_gld.o \
	fortitude_8p_fpga_gld.o \

SRC_BASE = \
	$(SRCDIR)/fortitude_main.c \
	$(SRCDIR)/diag.c \
	$(SRCDIR)/stubs.c \
	$(SRCDIR)/fortitude_framer.c \
	$(SRCDIR)/fortitude_fpga.c \
	$(SRCDIR)/fortitude_tests.c \
	$(SRCDIR)/fortitude_npu.c \
	$(SRCDIR)/bootflash_test.c \
	$(PLDDIR)/fortitude_fpga_image_lx25.c \
	$(PLDDIR)/fortitude_fpga_image_lx9.c \
	$(PLDDIR)/fortitude_fpga_image_lx4.c \
	$(PLDDIR)/fortitude_2p_fpga_ugd.c \
	$(PLDDIR)/fortitude_4p_fpga_ugd.c \
	$(PLDDIR)/fortitude_8p_fpga_ugd.c \
	$(PLDDIR)/fortitude_2p_fpga_gld.c \
	$(PLDDIR)/fortitude_4p_fpga_gld.c \
	$(PLDDIR)/fortitude_8p_fpga_gld.c \

NET_SRC = \
	$(SRC_BASE) \
	$(COM_BASE) \
	$(DEV_CORE_BASE) \
	$(DEV_BASE) 

NET_OBJS = \
	$(SRC_OBJS) \
	$(COM_OBJS) \
	$(DEV_CORE_OBJS) \
	$(DEV_OBJS)

vpath %.c \
	$(SRCDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(PMC4359_SRC): \
	$(PLDDIR): \

vpath %.h \
	$(SRCDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(PMC4359_SRC): \
	$(WDDI_API_INC):

DEFS = -D__NO_STRING_INLINES -Dlinux -DWP_HW_WINPATH3 -DLINUX_APP -DFORTITUDE -D_WP_MODULE_EFM_BONDING_OVER_ENET_G999_ -DPMC4358
LDFLAGS_GLOBAL += -G 0 -g -static -msoft-float -nostdinc -L$(LINUX_PATH)/lib/gcc/mips-montavista-linux-gnu/4.3.3/soft-float/
CFLAGS= -c -O0 -fsigned-char -msoft-float -g -G 0 -mips2 -mno-memcpy -fomit-frame-pointer -fno-builtin -pipe -Wa, $(INC) $(DEFS)
COMPILE = $(CC) $(CFLAGS) -o $@ $<

%.o : %.c
	$(LINUX_SET_PATH) $(COMPILE)

TARGET = fortitude

all: $(TARGET)

$(TARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh FORTITUDE 
	$(LINUX_SET_PATH) $(CC) $(CFLAGS) -c banner.c
	$(LINUX_SET_PATH) $(CC) $(LDFLAGS_GLOBAL) -v -o $@ $(NET_OBJS) banner.o $(DPS_IMAGE_OBJ) $(PPS_IMAGE_OBJ) $(WDDI_LIB) -lpthread -v

install: $(TARGET)
	cp $(TARGET) $(GNUSTEP_APPS)/fortitude/userspace/fortitude

clean:
	rm -rf *.o *.d $(TARGET) banner.c .compile

#/******** History ********
#$Log: Makefile,v $
#Revision 1.14  2014/08/18 06:38:48  iachang
#
#Supported boot flash R/W utility.
#
#Revision 1.13  2013/11/26 08:40:35  hroni
#fix compiler warning
#
#Revision 1.12  2013/09/06 22:14:25  ywen
#Add src/bit_swaps.c to solve compile error.
#
#Revision 1.11  2013/05/16 17:58:36  ywen
#Update Monta Vista license server we use.
#
#Revision 1.10  2013/01/29 17:36:09  ywen
#Remove .compile and banner.o in make clean.
#
#Revision 1.9  2012/11/13 17:22:01  ywen
#Add version control to Fortitude FW.
#
#Revision 1.8  2012/10/31 21:28:01  ywen
#Add utility to upgrade golden FPGA image.
#
#Revision 1.7  2012/10/04 21:38:29  ywen
#Add code to support different HW revision.
#
#Revision 1.6  2012/08/21 23:12:34  ywen
#Upgrade to wdds_4_2.
#
#Revision 1.5  2012/06/07 23:30:16  ywen
#Add support for 1/2/4/8 port SKUs.
#
#Revision 1.4  2012/05/31 00:10:00  ywen
#Improve Makefile to provide kernel directory dynamically.
#
#Revision 1.3  2012/05/22 23:19:38  ywen
#Improve Makefile to be able to build Fortitude image from any Linux server.
#
#Revision 1.2  2012/04/20 21:14:55  ywen
#Cleanup.
#
#Revision 1.1.1.1  2012/03/23 23:02:06  ptong
#Initial archive of ng_diag module
#
#Revision 1.1.2.1  2012/03/05 19:01:23  ywen
#Initial checkin to support Fortitude.
#

#$Endlog$
#*/

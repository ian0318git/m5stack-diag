# $Id: Makefile,v 1.27 2020/01/09 01:02:22 jiajliu Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/Makefile,v $
#------------------------------------------------------------------
# Makefile - Makefile to build Operation-Overlord cavium data plan diag image
#
# March 2011, Paul Tong
#
# Copyright (c) 2011-2014 by Cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------

# Crash us if /bin/make is used:
SERVER:=$(HOSTNAME)

ifeq ($(origin USER),undefined)
USER = Bozo
endif

default: all 

SHELL = /bin/sh
OPTIONS =
MAKE = make

# Tree root
#
IROOT = ../../../..

#The processor specific source directory
#
PROCSRCDIR = $(IROOT)/mips/src
PROCINCDIR = $(IROOT)/mips/include

# Top level common directories
#
INCDIR= $(IROOT)/common/include
COMSRCDIR = $(IROOT)/common/src
DEV_DRIVER = $(IROOT)/common/chips
UTIL_DIR = $(IROOT)/utils
KERNDIR= $(COMSRCDIR)/kernel
CHIPINC = $(DEV_DRIVER)/include
SRCDIR = .
OVERLORDDIR = ..

# Object device drivers
#
EEPROM_SRC = $(DEV_DRIVER)/dev_at24c0n_atmel
I2CMX_SRC = $(DEV_DRIVER)/dev_pca9545a_ti

KLMINC = $(IROOT)/klm/.

# Cavium Octeon SDK environment and lib paths
# OCTEON_ROOT is setup in the user Linux environment.
# OCTEON_ROOT = /auto/overlord/ovld-vendor-src/cvmx2.2
# The libcvmx.a is from $(OCTEON_ROOT)/examples/linux-filter/obj-linux_64/libcvmx.a
#
OCTEON_ROOT=/auto/overlord/ovld-vendor-src/cvmx2.2
CVMX_SDK_INC_PATH = $(OCTEON_ROOT)/executive
CVMX_LIB = /auto/sp-engops/diags/pld/overlord/libcvmx.a
OCTEON_MODEL = OCTEON_CN66XX_PASS1_0
OCTEON_TARGET = linux_64
OCTEON_REMOTE_PROTOCOL=pci
OCTEON_REMOTE_DEBUG=1
OCTEON_CPPFLAGS_GLOBAL_ADD= \
	-DUSE_RUNTIME_MODEL_CHECKS=1 \
	-DCVMX_ENABLE_PARAMETER_CHECKING=0 \
	-DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 \
	-DCVMX_ENABLE_POW_CHECKS=0

INC = 	-I. \
	-I$(INCDIR) \
	-I$(CVMX_SDK_INC_PATH) \
	-I$(SRCDIR) \
	-I$(CHIPINC) \
	-I$(PROCINCDIR) \
	-I$(KLMINC) \
	-I$(OVERLORDDIR) \
	-I$(OVERLORDDIR)/x86 \
	-I$(DEV_DRIVER) \
	-I$(EEPROM_SRC) \
	-I$(I2CMX_SRC) \

CURDIR=$(shell pwd)
OCTEON_SET=cd $(OCTEON_ROOT); source $(OCTEON_ROOT)/env-setup $(OCTEON_MODEL); cd $(CURDIR)

#  Octeon tools
#
CC = $(OCTEON_SET); mips64-octeon-linux-gnu-gcc
AR = $(OCTEON_SET); mips64-octeon-linux-gnu-ar
LD = $(OCTEON_SET); mips64-octeon-linux-gnu-ld
STRIP = $(OCTEON_SET); mips64-octeon-linux-gnu-strip
OBJDUMP = $(OCTEON_SET); mips64-octeon-linux-gnu-objdump
NM = $(OCTEON_SET); mips64-octeon-linux-gnu-nm


#  Octeon default flags
CPPFLAGS_GLOBAL = $(OCTEON_CPPFLAGS_GLOBAL_ADD)
CFLAGS_COMMON_CONFIG += -DOCTEON_MODEL

CFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL) $(CFLAGS_COMMON_CONFIG) \
	-DOCTEON_TARGET -mabi=64 -march=octeon -msoft-float 

ASFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL) -g $(CFLAGS_COMMON_CONFIG) \
	-DOCTEON_TARGET -mabi=64 -march=octeon -msoft-float

LDFLAGS_PATH = -L$(OCTEON_ROOT)/target/lib
LDFLAGS_GLOBAL = -mabi=64 -static -lpthread -lrt -Xlinker -T -Xlinker $(OCTEON_ROOT)/target/lib/cvmx-shared-linux.ld

CFLAGS_LOCAL += -g -Wno-unused-parameter \
	-Wall -Werror \
	-DLINUX_APP \
	-DLINUX_64BIT \
	-D_GNU_SOURCE $(INC) \
	-DOVERLORD \
	-DCVMX_DP

COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -o $@ $<
DEPCOMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) 
COMPILE_NOWARN = $(COMPILE)
ASSEMBLE = $(CC) $(ASFLAGS_GLOBAL) $(ASFLAGS_LOCAL) -MD -c -o $@ $<

.PHONY: all clean clobber depend

NETTARGET = o2cvm_lnx

# Source code
CBASE = \
    $(SRCDIR)/diag.c \
	$(SRCDIR)/fpga_sfp_intr.c \
	$(SRCDIR)/host2dp_mbox.c \
	$(COMSRCDIR)/linux_eth.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_i2c.c \
	$(SRCDIR)/main.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/menu.c \
	$(SRCDIR)/ovld_common_utils.c \
	$(COMSRCDIR)/linux_error.c \
	$(SRCDIR)/platform_eth.c \
	$(SRCDIR)/platform_ext_lpbk.c \
	$(SRCDIR)/platform_macsec.c \
	$(SRCDIR)/platform_sfp_ext_lpbk.c \
	$(SRCDIR)/platform_sgmii.c \
	$(SRCDIR)/platform_xaui.c \
	$(SRCDIR)/platform_dimm.c \
	$(SRCDIR)/platform_124mux.c \
	$(SRCDIR)/platform_sfp_cookie.c \
	$(COMSRCDIR)/query.c \
	$(SRCDIR)/stub.c \
	$(COMSRCDIR)/testmem.c \
	$(OVERLORDDIR)/dash_reg_test.c \
	$(OVERLORDDIR)/dash_get_addr.c \
	$(OVERLORDDIR)/eth_pkt_utils.c \
    $(SRCDIR)/eeprom_check.c \
	$(SRCDIR)/platform_i2c.c 


# The following .o files have compiled warnings suppressed (no -Wall option)
# NOTE: ONLY VENDOR FILES should be compiled without -Wall option
$(CVMX_OBJS):  %.o : %.c
	$(COMPILE_NOWARN)

HFILES=	\
	$(SRCDIR)/mon_plat_defs.h \
	$(SRCDIR)/nvmonvars.h \
	$(SRCDIR)/pcmap.h \
	$(INCDIR)/common.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/menu.h \
	$(INCDIR)/queryflags.h \
	$(INCDIR)/bitdef.h \
	$(INCDIR)/defs.h \
	$(INCDIR)/signals.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/error.h \
	$(INCDIR)/strings.h \
	$(INCDIR)/free.h \
	$(INCDIR)/ctype.h \
	$(INCDIR)/stdarg.h \
	$(INCDIR)/types.h \
	$(INCDIR)/cpu.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/linux_eth.h \
	$(SRCDIR)/platform_eth.h \
	$(SRCDIR)/platform_macsec.h \
	$(SRCDIR)/platform_fru.h \
	$(SRCDIR)/host2dp_mbox.h \
	$(OVERLORDDIR)/dash_fpga.h \
	

# For diaglinux.
# RAM Based Object files : linux_main.o must be first.
# This will ensure that _start is first code in .text section.
#
NET_OBJS = \
	main.o \
	fpga_sfp_intr.o \
	host2dp_mbox.o \
	linux_eth.o \
	linux_mem.o \
	linux_i2c.o \
	linux_error.o \
	memops.o \
	menu.o \
	platform_eth.o \
	platform_i2c.o \
	platform_ext_lpbk.o \
	platform_macsec.o \
	platform_sfp_ext_lpbk.o \
	platform_sgmii.o \
	platform_xaui.o \
	platform_dimm.o \
	platform_124mux.o \
	platform_sfp_cookie.o \
	query.o \
	testmem.o \
	ovld_common_utils.o \
	diag.o \
	stub.o \
	dash_reg_test.o \
	dash_get_addr.o \
	eth_pkt_utils.o \
        eeprom_check.o \

vpath %.c \
	$(PROCSRCDIR): \
	$(COMSRCDIR): \
	$(OVERLORDDIR): \
	$(SRCDIR): \
	$(KERNDIR): \

vpath %.h $(PROCINCDIR): \
	  $(CVMX_SDK_INC_PATH): \
	  $(OVERLORDDIR): \
	  $(SRCDIR): \
	  $(INCDIR): \
	  $(KERNDIR): \
	  $(GS_4WIRE): \
	  $(GS_NEW): \
	  $(DEV_DRIVER): \
	  $(DIMM_SRC): \
	  $(EEPROM_SRC): \

vpath %.S $(PROCSRCDIR):$(SRCDIR):$(COMSRCDIR)

# Override RCS stuff

.SUFFIXES:
.SUFFIXES: .o .lis .S .c .s .dis

 %.o : %.S
	$(ASSEMBLE)
# a very large image compiled without -g option to avoid a
# warning message generated by compiler
 %.o : %.c
	$(COMPILE)
%.lis : %.c
	lister3 CC=$(CC) AS=$(AS) $< $(CFLAGS) | grep -v '.stab' >$*.lis
#
# the following  makes a disassemble listing file from a .o file
# but to make the entire diag image (atlantisnet.dis) do:
# objdump.98r1-v0.mips64 -h -l -S -D caviumnet.elf > caviumnet.dis
#
%.dis :  %.o
	$(OBJDUMP) -l -S -D $< > $*.dis

all: $(NETTARGET)

$(NETTARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh OVLD_DP
	$(CC) $(CFLAGS_GLOBAL) -c banner.c 
	$(CC) $(NET_OBJS) $(LDFLAGS_PATH) $(CVMX_LIB) $(LDFLAGS_GLOBAL) banner.o -o $@
	$(OBJDUMP) -t $(NETTARGET) >  $(NETTARGET).symbols

clean:  
	rm -f $(NET_OBJS) $(LIBOBJS) \
	banner*.* temp.* *.dat syms.net .compile
	rm -rf *.[od]

clobber: clean
	rm -f $(NETTARGET)*

depend:
	@$(DEPCOMPILE) -M $(CBASE) > dependancies

dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.27  2020/01/09 01:02:22  jiajliu
# Merge Curie 2RU to main trunk
#
# Revision 1.26  2014/02/18 09:11:11  alpeng
# CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
#
# Revision 1.25  2013/12/18 06:32:58  hroni
# use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64
#
# Revision 1.24  2013/11/29 10:42:03  hroni
# set octeon toolchain env in Makefile
#
# Revision 1.23  2013/11/27 09:07:47  hroni
# check for valid build server
#
# Revision 1.22  2013/10/08 11:14:27  erwu2
# enhanced err msg first check-in
#
# Revision 1.21  2013/06/05 03:00:03  hroni
# Revert platform_i2c.c path for cavium
#
# Revision 1.20  2013/06/04 07:08:52  hroni
# move platform specific files to the corresponding directory (i.e. x86 and utah)
#
# Revision 1.19  2013/02/15 01:09:50  ptong
# Stop using common/src/linux_api.c in Cavium Linux
#
# Revision 1.18  2013/01/25 10:47:02  alpeng
# support macsec util
#
# Revision 1.17  2012/12/12 01:59:41  ptong
# Put libcvmx.a at /auto/sp-engops/diags/pld/overlord/libcvmx.a
#
# Revision 1.16  2012/11/14 01:47:33  ptong
# Move BCM and CVMX vendor src tree to /auto/overlord/ovld-vendor-src
#
# Revision 1.15  2012/09/21 09:32:29  danchung
# Add eeprom_check.c file.
#
# Revision 1.14  2012/08/11 00:00:18  ptong
# Remove complile flag RELEASE_CVMX_DIAG
#
# Revision 1.13  2012/08/10 22:51:38  ptong
# Add version banner
#
# Revision 1.12  2012/07/24 23:57:59  ptong
# Bump up release string to 2.0.0 and add -Werror in Makefile
#
# Revision 1.11  2012/07/18 22:59:29  ptong
# Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu
#
# Revision 1.10  2012/06/14 22:37:43  ptong
# Add FPGA SFP interrupt test
#
# Revision 1.9  2012/06/06 15:00:37  palin2
# Clean up compiler warnings.
#
# Revision 1.8  2012/05/31 22:01:14  palin2
# Turn on "-Wall" compiler flag.
#
# Revision 1.7  2012/05/30 09:36:54  alpeng
# suppoted i2c scan test on cavium side, removing useless definition on platform_i2c.h
#
# Revision 1.6  2012/05/23 09:20:08  alpeng
# support is_sfp_present function for SFP test
#
# Revision 1.5  2012/05/08 00:05:15  ptong
# Improve test printing
#
# Revision 1.4  2012/05/02 21:27:47  ptong
# Use o2cvm_lnx and o2cvm_lnx_lab as MAKECMDGOALS
#
# Revision 1.3  2012/05/02 20:11:51  ptong
# Add condition compile flag (release)
#
# Revision 1.2  2012/04/17 22:01:26  ptong
# Added more utility to run DP test from host.
#
# Revision 1.1.1.1  2012/03/23 23:02:07  ptong
# Initial archive of ng_diag module
#
# Revision 1.1.2.12  2012/02/28 02:43:02  ptong
# Add RELEASE_CVMX_DIAG flag to include sgmii-0 testing
#
# Revision 1.1.2.11  2012/02/04 00:25:18  ptong
# Config Cavium xaui port, add ping test, and other fixes
#
# Revision 1.1.2.10  2012/01/23 07:12:46  ptong
# Clean up file
#
# Revision 1.1.2.9  2011/12/23 02:45:13  palin2
# To fix Overlord Cavium side compile error.
#
# Revision 1.1.2.8  2011/12/21 23:33:49  ptong
# Add FPGA register test
#
# Revision 1.1.2.7  2011/12/05 01:23:12  ptong
# Remove appmain in main.c and Makefie
#
# Revision 1.1.2.6  2011/10/31 18:46:07  alpeng
# support internal/external loopback on Overlord.
#
# Revision 1.1.2.5  2011/10/11 18:09:34  ptong
# Set OCTEON_ROOT
#
# Revision 1.1.2.4  2011/09/09 22:19:47  ptong
# Add cvmx_lib
#
# Revision 1.1.2.3  2011/08/26 08:34:54  palin2
# Added SFP tests support to Overlord Cavium.
#
# Revision 1.1.2.2  2011/07/18 17:32:49  palin2
# To add DIMM utilities support in Cavium.
#
# Revision 1.1.2.1  2011/04/05 19:59:36  ptong
# Initial checkin
#
# $Endlog$
#########################

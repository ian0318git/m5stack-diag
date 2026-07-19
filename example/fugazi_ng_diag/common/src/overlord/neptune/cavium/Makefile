# $Id: Makefile,v 1.3 2020/01/09 01:02:24 jiajliu Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/Makefile,v $
#------------------------------------------------------------------
# Makefile - Makefile to build Neptune cavium data plan diag image
#
# Leveraged from overlord/cavium
#
# Copyright (c) 2016-2018 by Cisco Systems, Inc.
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
IROOT = ../../../../..

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
OVERLORDDIR = ../..
CVMDIR = ../../cavium
VENDOR_SRC=/auto/neptune/vendor-src

# Object device drivers
#
EEPROM_SRC = $(DEV_DRIVER)/dev_at24c0n_atmel
I2CMX_SRC = $(DEV_DRIVER)/dev_pca9545a_ti

KLMINC = $(IROOT)/klm/.

# Broadcom lib path
BCM_API_PATH = $(VENDOR_SRC)/broadcom
BCM_MIURA_LIB = $(BCM_API_PATH)/api-82757/BCM82757_SW/MIUR_1_6/build/libphymodepil.a
BCM_QUADRA28_LIB = $(BCM_API_PATH)/api-82752/Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/build/libphymodepil.a
BCM_MIURA_REFER_PATH = $(BCM_API_PATH)/api-82757/BCM82757_SW/MIUR_1_6/miura_reference_app
BCM_EPDM_PATH = $(BCM_API_PATH)/api-82757/BCM82757_SW/epdm_1_5_8
BCM_EPDM_LIB = $(BCM_EPDM_PATH)/libepdm.a

# Cavium Octeon SDK environment and lib paths
# OCTEON_ROOT is setup in the user Linux environment.
# OCTEON_ROOT = /auto/neptune/vendor-src/cavium/cvm-sdk-3.1.2/Cavium_Networks/OCTEON-SDK 
# The libcvmx.a is from $(OCTEON_ROOT)/executive/obj-linux_64-octeon3/libcvmx.a
#
OCTEON_ROOT=$(VENDOR_SRC)/cavium/cvm-sdk-3.1.2/Cavium_Networks/OCTEON-SDK
CVMX_SDK_INC_PATH = $(OCTEON_ROOT)/executive
CVMX_LIB = $(OCTEON_ROOT)/executive/obj-linux_64-octeon3/libcvmx.a
OCTEON_MODEL = OCTEON_CN72XX
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
	-I$(BCM_EPDM_PATH) \
	-I$(BCM_MIURA_REFER_PATH) \

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
	-DOCTEON_TARGET -mabi=64 -march=octeon3 

ASFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL) -g $(CFLAGS_COMMON_CONFIG) \
	-DOCTEON_TARGET -mabi=64 -march=octeon3

LDFLAGS_PATH = -L$(OCTEON_ROOT)/target/lib
LDFLAGS_GLOBAL = -mabi=64 -march=octeon3 -static -lpthread -lrt -Xlinker -T -Xlinker $(OCTEON_ROOT)/target/lib/cvmx-shared-linux.ld

CFLAGS_LOCAL += -g -Wno-unused-parameter -Wno-unused-but-set-variable \
	-Wall -Werror \
	-DLINUX_APP \
	-DLINUX_64BIT \
	-D_GNU_SOURCE $(INC) \
	-DOVERLORD \
	-DCVMX_DP \
	-DDUEL_DIE_DUPLEX_FP_40G \
	-DPLP_MIURA_SUPPORT \
	-DPLP_MACSEC_SUPPORT \
	-DSERDES_API_FLOATING_POINT

COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -o $@ $<
DEPCOMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) 
COMPILE_NOWARN = $(COMPILE)
ASSEMBLE = $(CC) $(ASFLAGS_GLOBAL) $(ASFLAGS_LOCAL) -MD -c -o $@ $<

.PHONY: all clean clobber depend

NETTARGET = nepcvm_diag

# Source code
CBASE = \
	$(SRCDIR)/main.c \
	$(SRCDIR)/diag.c \
	$(SRCDIR)/linux_mem.c \
	$(SRCDIR)/fpga_sfp_intr.c \
	$(CVMDIR)/host2dp_mbox.c \
	$(SRCDIR)/ovld_common_utils.c \
	$(SRCDIR)/eeprom_check.c \
	$(SRCDIR)/platform_eth.c \
	$(SRCDIR)/platform_dimm.c \
	$(SRCDIR)/platform_124mux.c \
	$(SRCDIR)/platform_sfp_cookie.c \
	$(SRCDIR)/platform_i2c.c \
	$(SRCDIR)/stub.c \
	$(SRCDIR)/sff_trans.c \
	$(SRCDIR)/phy_common.c \
	$(SRCDIR)/miura_macsec_10G_bypass_port.c \
	$(SRCDIR)/epdm.c \
	$(SRCDIR)/quadra28_common.c \
	$(SRCDIR)/bcm82752_api.c \
	$(SRCDIR)/bcm82752_reg_tbl.c \
	$(SRCDIR)/bcm82752_test.c \
	$(SRCDIR)/bcm82752_ucode.c \
	$(SRCDIR)/bcm54194_api.c \
	$(SRCDIR)/bcm54194_ptp1588_lib.c \
	$(SRCDIR)/bcm54194_test.c \
	$(COMSRCDIR)/linux_eth.c \
	$(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/testmem.c \
	$(OVERLORDDIR)/dash_reg_test.c \
	$(OVERLORDDIR)/dash_get_addr.c \
	$(OVERLORDDIR)/eth_pkt_utils.c \
	$(SRCDIR)/platform_ext_lpbk.c \
	$(SRCDIR)/platform_sgmii.c \
	$(SRCDIR)/platform_xfi.c \
	$(SRCDIR)/diag_common_lib.c \
	$(SRCDIR)/platform_sfp_ext_lpbk.c



# The following .o files have compiled warnings suppressed (no -Wall option)
# NOTE: ONLY VENDOR FILES should be compiled without -Wall option
$(CVMX_OBJS):  %.o : %.c
	$(COMPILE_NOWARN)

HFILES=	\
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
	$(INCDIR)/types.h \
	$(INCDIR)/cpu.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/linux_eth.h \
	$(SRCDIR)/platform_eth.h \
	platform_fru.h \
	$(SRCDIR)/platform_dimm.h \
	$(CVMDIR)/host2dp_mbox.h \
	$(OVERLORDDIR)/dash_fpga.h \
	$(SRCDIR)/sff_trans.h \
	$(SRCDIR)/bcm82752_api.h \
	$(SRCDIR)/bcm82752_reg_def.h \
	$(SRCDIR)/bcm54194_api.h \
	$(SRCDIR)/bcm82752_test.h \
	$(SRCDIR)/bcm54194_test.h \
	$(SRCDIR)/platform_ext_lpbk.h \
	$(SRCDIR)/platform_xfi.h \
	$(SRCDIR)/diag_common_lib.h

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
	sff_trans.o \
	phy_common.o \
	miura_macsec_10G_bypass_port.o \
	epdm.o \
	quadra28_common.o \
	bcm82752_api.o \
	bcm82752_reg_tbl.o \
	bcm82752_test.o \
	bcm82752_ucode.o \
	bcm54194_api.o \
	bcm54194_ptp1588_lib.o \
	bcm54194_test.o \
	platform_ext_lpbk.o \
	platform_sgmii.o \
	platform_xfi.o \
	diag_common_lib.o \
	platform_sfp_ext_lpbk.o

vpath %.c \
	$(PROCSRCDIR): \
	$(COMSRCDIR): \
	$(OVERLORDDIR): \
	$(SRCDIR): \
	$(KERNDIR): \
	$(CVMDIR): \

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
	  $(CVMDIR): \

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
	sh $(IROOT)/utils/banner.sh NEP_DP
	$(CC) $(CFLAGS_GLOBAL) -c banner.c 
	$(CC) $(NET_OBJS) $(LDFLAGS_PATH) $(CVMX_LIB) $(BCM_QUADRA28_LIB) -Wl,-whole-archive $(BCM_EPDM_LIB) $(BCM_MIURA_LIB) -Wl,-no-whole-archive $(LDFLAGS_GLOBAL) banner.o -o $@
	$(OBJDUMP) -t $(NETTARGET) >  $(NETTARGET).symbols

clean:  
	rm -f $(NET_OBJS) $(LIBOBJS) \
	banner*.* temp.* *.dat syms.net .compile ID.temp
	rm -rf *.[od]

clobber: clean
	rm -f $(NETTARGET)*

DEPEND_LST = $(CBASE) $(HFILES)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(DEPCOMPILE) -M $(CBASE) $(ASM) > dependancies
	@echo "Computing TAGS file for common/src"
	$(shell which etags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	-lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)


dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.3  2020/01/09 01:02:24  jiajliu
# Merge Curie 2RU to main trunk
#
# Revision 1.2  2018/05/18 09:24:53  alpeng
#  Neptune merge to trunk with tag <neptune-branch-0518>
#
# Revision 1.1.2.20  2018/05/08 03:54:27  alpeng
# update make depend method
#
# Revision 1.1.2.19  2018/03/27 06:30:37  meho
# upgraded BCM API to MIUR_1_6 and epdm_1_5_8
#
# Revision 1.1.2.18  2017/10/31 07:07:14  meho
# Separated BCM54194 PTP1588 LIB from bcm54194_api.c.
#
# Revision 1.1.2.17  2017/10/17 07:51:42  meho
# Upgraded BCM API: MIUR_1_1, epdm_1_4_8.
#
# Revision 1.1.2.16  2017/04/10 06:21:57  meho
# changed BCM82752 API lib path.
#
# Revision 1.1.2.15  2017/04/10 05:42:25  meho
# Integrated BCM82752/82757 API.
#
# Revision 1.1.2.14  2017/04/06 09:36:27  alpeng
# support dependancies on Nep DP
#
# Revision 1.1.2.13  2017/01/25 02:36:53  alpeng
# fixed bug
#
# Revision 1.1.2.12  2017/01/18 03:09:12  alpeng
# remove OCTEON_CPPFLAGS_GLOBAL_ADD, we use source env-setup instead
#
# Revision 1.1.2.11  2016/11/15 07:45:20  alpeng
# fix depend and transfer to unix format
#
# Revision 1.1.2.10  2016/11/03 08:26:54  alpeng
# merge octeon_test.c with o2
#
# Revision 1.1.2.9  2016/08/11 12:01:35  meho
# Added BCM LIB: libphymodepil.a
#
# Revision 1.1.2.8  2016/07/20 06:36:23  xiaoyizh
# Use arch=octeon3 instead of octeon and remove -msoft-float.
#
# Revision 1.1.2.7  2016/07/18 06:55:45  meho
# Added read SFP module type from eeprom api.
#
# Revision 1.1.2.6  2016/07/07 09:04:29  meho
# 1. Added BCM54194 RDB register r/w utility.
# 2. Added GE PHY internal/external loopback skeleton.
# 3. Added 10GE PHY internal/external loopback skeleton.
#
# Revision 1.1.2.5  2016/06/23 12:44:54  meho
# Added bcm54194 soft-reset function.
#
# Revision 1.1.2.4  2016/06/13 02:26:18  xiaoyizh
# Change the target image name.
#
# Revision 1.1.2.3  2016/06/12 10:31:06  bowang3
# Add bcm82752 10G PHY code framework
#
# Revision 1.1.2.2  2016/06/06 05:58:50  xiaoyizh
# Initial Check-in for Neptune Data Plane diags.
#
# Revision 1.1.2.1  2016/04/29 19:40:48  ptong
# Creating neptune directory and initial Makefiles for x86 and cavium
#
# $Endlog$
#########################

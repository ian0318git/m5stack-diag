# $Id: Makefile,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/Makefile,v $
#------------------------------------------------------------------
# Makefile - Makefile to build Wallander diag image
#
# Feb 2014, Xiaoying Zhang
#
# Copyright (c) 2015 by Cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------

# Crash us if /bin/make is used:
ifeq ($(origin USER),undefined)
USER = Bozo
endif

default: all 

SHELL = /bin/sh
OPTIONS =
MAKE = make


# Tree root
#
IROOT = ../../..

# VENDOR_DIR := /ws/xiaoyizh-sjc/wallander
OCTEON_SDK_DIR := $(VENDOR_DIR)/OCTEON-SDK
CROSS_COMPILER_DIR=$(OCTEON_SDK_DIR)/tools/bin
VENDOR_PHY_DIR = $(VENDOR_DIR)/vtss_sw

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
VTSS_PHY_BASE_DIR = $(VENDOR_PHY_DIR)/api_4_49d/vtss_api/base
VTSS_PHY_INC_DIR = $(VENDOR_PHY_DIR)/api_4_49d/vtss_api/include
VTSS_PHY_BOARDS_DIR = $(VENDOR_PHY_DIR)/api_4_49d/vtss_api/boards

# Code signing utility
#
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client


# Object device drivers
#
CLK1_SRC = $(DEV_DRIVER)/dev_idt_ics844s2416i
CLK2_SRC = $(DEV_DRIVER)/dev_idt6v4900n
DIMM_SRC = $(DEV_DRIVER)/dev_dimm
TDM_PLL_SRC = $(DEV_DRIVER)/dev_idt82v32021
SM_PCLK_SRC = $(DEV_DRIVER)/dev_sm_pci_clk
ENV_SRC = $(DEV_DRIVER)/dev_csco_10698
SNSR_SRC = $(DEV_DRIVER)/dev_max1617a
RTC_SRC = $(DEV_DRIVER)/dev_1337_maxim
TMP421_SRC = $(DEV_DRIVER)/dev_tmp421
PSE2_SRC = $(DEV_DRIVER)/dev_pse2_fpga
LTC4215_SRC = $(DEV_DRIVER)/dev_ltc4215
PSE2_PHY_SRC = $(DEV_DRIVER)/dev_88e1111_marvell
EEPROM_SRC = $(DEV_DRIVER)/dev_at24c0n_atmel
USB_SRC = $(DEV_DRIVER)/dev_cy7c64215_cypress
I2CMX_SRC = $(DEV_DRIVER)/dev_pca9545a_ti
LM75_SRC = $(DEV_DRIVER)/dev_lm75_national
RPS_SRC = $(DEV_DRIVER)/dev_r2300_csco_rps
NM_ADPTR_CPLD_SRC = $(DEV_DRIVER)/dev_nm_adptr_cpld
AP_CON_SRC = $(DEV_DRIVER)/dev_ap_console

# IO module diag directories
#
SOCRATES = $(COMSRCDIR)/Socrates  #needed for CLI
NIOS2_FIREBEE = $(COMSRCDIR)/nios2_firebee/obj-nios-II
ISM_TESTCARD = $(COMSRCDIR)/ism_testcard
PSE2DIR = $(COMSRCDIR)/pse2_testboard
CX27470 = $(COMSRCDIR)/cx27470  #need for CLI

KLMINC = $(IROOT)/klm/.

# Cavium Octeon SDK and lib paths
OCTEON_ROOT = $(OCTEON_SDK_DIR)
CVMX_SDK_INC_PATH = $(OCTEON_ROOT)/executive
CVMX_SDK_INC_ASM_OCTEON_PATH = $(OCTEON_ROOT)/linux/kernel/linux/arch/mips/include
CVMX_LIB = $(OCTEON_ROOT)/executive/obj-linux_64-octeon3/libcvmx.a

INC = -I$(INCDIR) \
	-I$(CVMX_SDK_INC_PATH) \
	-I$(SRCDIR) -I. -I$(DEV_DRIVER) -I$(CHIPINC) -I$(PROCINCDIR) -I$(KLMINC) \
	-I$(EEPROM_SRC) -I$(TMP421_SRC) -I$(I2CMX_SRC) \
	-I$(VTSS_PHY_BASE_DIR) -I$(VTSS_PHY_INC_DIR) \
	-I$(VTSS_PHY_BASE_DIR)/phy/ts -I$(VTSS_PHY_BASE_DIR)/ail

OCTEON_MODEL = OCTEON_CN71XX
OCTEON_TARGET = linux_64
OCTEON_CPPFLAGS_GLOBAL_ADD= -DUSE_RUNTIME_MODEL_CHECKS=1 -DCVMX_ENABLE_PARAMETER_CHECKING=0 -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DCVMX_ENABLE_POW_CHECKS=0

linux_path=${OCTEON_ROOT}/linux/kernel/linux

CC = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-gcc
AR = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-ar
LD = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-ld
STRIP = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-strip
OBJDUMP = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-objdump
NM = $(CROSS_COMPILER_DIR)/mips64-octeon-linux-gnu-nm

#  Octeon tools
#  Octeon default flags
CPPFLAGS_GLOBAL += $(OCTEON_CPPFLAGS_GLOBAL_ADD)

CFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL)
CFLAGS_GLOBAL += $(OCTEON_CFLAGS_GLOBAL_ADD)

ASFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL) -g
ASFLAGS_GLOBAL += $(OCTEON_ASFLAGS_GLOBAL_ADD)

LDFLAGS_GLOBAL =
LDFLAGS_GLOBAL += $(OCTEON_LDFLAGS_GLOBAL_ADD)

LDFLAGS_PATH = -L$(OCTEON_ROOT)/target/lib

CFLAGS_COMMON_CONFIG += -DOCTEON_MODEL
CFLAGS_GLOBAL += $(CFLAGS_COMMON_CONFIG)
ASFLAGS_GLOBAL += $(CFLAGS_COMMON_CONFIG)

CFLAGS_GLOBAL += -DOCTEON_TARGET -mabi=64 -march=octeon -msoft-float 
ASFLAGS_GLOBAL += -DOCTEON_TARGET -mabi=64 -march=octeon -msoft-float
LDFLAGS_GLOBAL += -mabi=64 -static -lpthread -lrt -Xlinker -T -Xlinker $(OCTEON_ROOT)/target/lib/cvmx-shared-linux.ld
CFLAGS_LOCAL += -g -Wall -Wno-unused-parameter \
	-DLINUX_APP -DLINUX_64BIT -D_GNU_SOURCE $(INC) -DVINDICATOR \
	-DRUN_FROM_CACHE -DEVAL_BD -DWALLANDER


CC_VTSS_PHY_DEFS = -DVTSS_GPIOS -DVTSS_FEATURE_PHY_TIMESTAMP -DVTSS_PHY_TS_SPI_CLK_THRU_PPS0
CC_VTSS_PHY_DEFS += -DVTSS_FEATURE_MISC -DVTSS_FEATURE_QOS -DVTSS_FEATURE_SYNCE -DVTSS_OPSYS_LINUX -DVTSS_FEATURE_SERDES_MACRO_SETTINGS
CC_VTSS_PHY_DEFS += -DVTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION 
CC_VTSS_PHY_DEFS += -DVTSS_PHY_TS_SPI_CLK_THRU_PPS0 -DVTSS_FEATURE_MACSEC
CC_VTSS_PHY_DEFS += -DVTSS_OPT_PORT_COUNT=4 -DVTSS_FEATURE_PORT_CONTROL -DVTSS_FEATURE_CLAUSE_37
CC_VTSS_PHY_DEFS += -DVTSS_FEATURE_SERDES_MACRO_SETTINGS
CCOPTS += $(CC_VTSS_PHY_DEFS)

COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) $(CCOPTS) -MD -c -o $@ $<
DEPCOMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) $(CCOPTS)
COMPILE_NOWARN = $(COMPILE)
COMPILE_NOG = $(COMPILE)
ASSEMBLE = $(CC) $(ASFLAGS_GLOBAL) $(ASFLAGS_LOCAL) $(CCOPTS) -MD -c -o $@ $<

.PHONY: all clean clobber depend force-tools install superclean

NETTARGET = wallander

#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o 
	
VTSS_PHY_OBJS = \
	vtss_api.o \
	vtss_port_api.o \
	vtss_misc_api.o \
	vtss_common.o \
	vtss_wis_api.o \
	vtss_qos_api.o \
	vtss_phy_common.o \
	vtss_phy.o \
	vtss_phy_init_scripts.o \
	vtss_phy_veriphy.o \
	vtss_phy_ewis.o \
	vtss_phy_ts_api.o \
	vtss_macsec_api.o

DEV_CORE_BASE = \
	$(DEV_DRIVER)/dev_object.c

COM_OBJS = \
	menu.o \
	shcmd.o \
	alias.o \
	monitor.o \
	parsetoken.o \
	builtin.o \
	query.o \
	memops.o \
	testmem.o \
	linux_stub.o \
	linux_memory_tester_utils.o \
	linux_error.o \
	linux_api.o\
	linux_i2c.o\
	bit_swaps.o\
	print_hex.o

COM_BASE = \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/shcmd.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/monitor.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/bit_swaps.c

VTSS_PHY_BASE = \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_api.c \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_port_api.c \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_misc_api.c \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_common.c \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_wis_api.c \
	$(VTSS_PHY_BASE_DIR)/ail/vtss_qos_api.c \
	$(VTSS_PHY_BASE_DIR)/../appl/vtss_appl.c \
	$(VTSS_PHY_BASE_DIR)/../appl/vtss_appl_cli.c \
	$(VTSS_PHY_BASE_DIR)/../appl/vtss_version.c \
	$(VTSS_PHY_BASE_DIR)/phy/common/vtss_phy_common.c \
	$(VTSS_PHY_BASE_DIR)/phy/phy_1g/vtss_phy.c \
	$(VTSS_PHY_BASE_DIR)/phy/phy_1g/vtss_phy_init_scripts.c \
	$(VTSS_PHY_BASE_DIR)/phy/phy_1g/vtss_phy_veriphy.c \
	$(VTSS_PHY_BASE_DIR)/phy/ts/vtss_phy_ewis.c \
	$(VTSS_PHY_BASE_DIR)/phy/ts/vtss_phy_ts_api.c \
	$(VTSS_PHY_BASE_DIR)/phy/macsec/vtss_macsec_api.c

SRC_OBJS = \
	wallander_main.o \
	wallander_common_utils.o \
	diag.o \
	diag_common_drv.o \
	diag_vtss_phy.o \
	diag_bootflash_lib.o \
	diag_bootflash_test.o \
	diag_ge_phy_test.o \
	diag_ge_phy_vsc8584.o \
	diag_ge_phy_vsc8552.o \
	diag_fpga_test.o \
	diag_fpga_lib.o \
	diag_fpga_fw.o \
	platform_sfp_cookie.o \
	platform_i2c.o \
	mem_utils.o\
	stubs.o

SRC_BASE = \
	$(SRCDIR)/wallander_main.c \
	$(SRCDIR)/wallander_common_utils.c \
	$(SRCDIR)/diag.c \
	$(SRCDIR)/diag_common_drv.c \
	$(SRCDIR)/diag_vtss_phy.c \
	$(SRCDIR)/diag_bootflash_lib.c \
	$(SRCDIR)/diag_bootflash_test.c \
	$(SRCDIR)/diag_ge_phy_test.c \
	$(SRCDIR)/diag_ge_phy_vsc8584.c \
	$(SRCDIR)/diag_ge_phy_vsc8552.c \
	$(SRCDIR)/diag_fpga_test.c \
	$(SRCDIR)/diag_fpga_lib.c \
	$(SRCDIR)/diag_fpga_fw.c \
	$(SRCDIR)/platform_sfp_cookie.c \
	$(SRCDIR)/platform_i2c.c \
	$(SRCDIR)/mem_utils.c \
	$(SRCDIR)/stubs.c

NET_SRC = \
	$(SRC_BASE) \
	$(COM_BASE) \
	$(DEV_CORE_BASE) \
	$(VTSS_PHY_BASE)

NET_OBJS = \
	$(SRC_OBJS) \
	$(COM_OBJS) \
	$(DEV_CORE_OBJS) \
	$(VTSS_PHY_OBJS)

vpath %.c \
	$(SRCDIR): \
	$(COMSRCDIR): \
	$(DEV_DRIVER): \
	$(VTSS_PHY_BASE_DIR)/ail: \
	$(VTSS_PHY_BASE_DIR)/../appl: \
	$(VTSS_PHY_BASE_DIR)/phy/phy_1g: \
	$(VTSS_PHY_BASE_DIR)/phy/common: \
	$(VTSS_PHY_BASE_DIR)/phy/ts: \
	$(VTSS_PHY_BASE_DIR)/phy/macsec:

vpath %.h \
	$(CVMX_SDK_INC_PATH): \
	$(SRCDIR): \
	$(INCDIR): \
	$(DEV_DRIVER): \
	$(VTSS_PHY_BASE_DIR):


# The following .o files have compiled warnings suppressed (no -Wall option)
# NOTE: ONLY VENDOR FILES should be compiled without -Wall option
$(CVMX_OBJS):  %.o : %.c
	$(COMPILE_NOWARN)


# New CLI command object files
NEW_CLI_OBJS =  cli_cmd.o
NEW_CLI_BASE = $(COMSRCDIR)/cli_cmd.c
NEW_CLI_HFILES = $(INCDIR)/cli_cmd.h


HFILES=	\
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
	$(INCDIR)/linux_eth.h
UTOOLS= eprom_elf squish_elf mkffsi
HTOOLS= hxld
TOOLS= $(patsubst %,$(UTIL_DIR)/%,$(UTOOLS)) \
	$(patsubst %,$(UTIL_DIR)/hxldsrc/%,$(HTOOLS)) 

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

############################
# KLM

klm:
	$(MAKE) -C $(linux_path) SUBDIRS=`pwd` modules
klm_install:
	$(MAKE) -C $(linux_path) $(KOPT) modules_install
klm_clean:
	$(MAKE) -C $(linux_path) $(KOPT) clean

# KLM
###############################

ARCHIVE_FILES = $(NETTARGET) $(EXTRA_CVMX_APPLICATION_TARGETS)

archive: $(ARCHIVE_FILES)
	-@ if [ "${CVMX_ARCHIVE_DIRECTORY}" != "" ] ; then \
		mkdir -p ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
		cp -f $(ARCHIVE_FILES) ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL}/ ; \
		chmod -R a+r ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
	 fi ; \

application-target:$(NETTARGET) $(EXTRA_CVMX_APPLICATION_TARGETS) archive

$(NETTARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh WALLANDER
	$(CC) $(CFLAGS) -c banner.c
	$(CC) banner.o $(NET_OBJS) $(LDFLAGS_PATH) $(CVMX_LIB) $(LDFLAGS_GLOBAL) -o $@
	$(OBJDUMP) -t $(NETTARGET) >  $(NETTARGET).symbols

all: $(NETTARGET)

install: $(NETTARGET)
	cp $(NETTARGET) $(GNUSTEP_APPS)/wallander

clean:  
	rm -f $(NET_OBJS) $(LIBOBJS) $(HWIC_DSU_V3_UCODE_OBJS) $(SPIDEY_UCODE_T1E1_OBJS) \
	$(SPIDEY_UCODE_CET3_OBJS) $(CX98124_OBJS) $(TROI_OBJS) \
	banner*.* tools temp.* *.dat syms.net .compile
	rm -rf *.o *.d


clobber: clean
	rm -f $(NETTARGET)* moncode* pfsi syms*

superclean: clobber
	$(MAKE) --no-print-directory -C $(UTIL_DIR) clean
	$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc clean

force-tools:
	rm -f tools

tools:
	@$(MAKE) --no-print-directory -C $(UTIL_DIR) depend $(UTOOLS)
	@$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc depend $(HTOOLS)

depend:
	@$(DEPCOMPILE) -M $(DEV_BASE) $(NET_SRC) $(ASM) > dependancies

dependancies:
	rm -f dependancies
	touch dependancies

include dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.1  2015/02/26 07:18:29  xiaoyizh
# Initial check in for Wallander.
#
# $Endlog$
#########################

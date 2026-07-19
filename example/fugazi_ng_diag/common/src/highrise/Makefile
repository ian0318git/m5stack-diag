#------------------------------------------------------------------
#
# Makefile - Makefile to build Highrise EIO Diagnostic.
#
# Aug 2019, shuyyu
#
# Copyright (c) 2019 - 2020 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------

default: all

#
# Tool Chain
#

-include ./Makefile.local

# Using toolchain on Cisco server
CROSS_COMPILE_PATH    ?= /auto/srg-diag-xcompiler/APM/ESPN/Toolchain/LE/apm-aarch64/6.0.4/bin/
CROSS_COMPILE    ?= $(CROSS_COMPILE_PATH)aarch64-apm-linux-gnu-

CC				= $(CROSS_COMPILE)gcc
AS				= $(CROSS_COMPILE)as
AR				= $(CROSS_COMPILE)ar
LD				= $(CROSS_COMPILE)ld
NM				= $(CROSS_COMPILE)nm
OBJCOPY			= $(CROSS_COMPILE)objcopy
OBJDUMP			= $(CROSS_COMPILE)objdump
SIZE			= $(CROSS_COMPILE)size
STRIP			= $(CROSS_COMPILE)strip
MAKE			= make

LDFLAGS =  -lpthread ${LOCAL_LDFLAGS}
.PHONY: all depend clean

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

HIGHRISE_DIR := .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
KLM_DIR 	= $(IROOT)/klm
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
DEV_TELIT_SRC  = $(DEV_DRV)/dev_lte_telit
MTD_SDK_DIR = $(IROOT)/common/thirdparty_lib/mtd

#
# Include paths for compiler
#

INC	= \
	-I$(INCDIR) \
	-I$(DEV_DRV) \
	-I$(KLM_DIR) \
	-I$(HIGHRISE_DIR) \
	-I$(HIGHRISE_DIR)/common \
	-I$(HIGHRISE_DIR)/common/mdio \
	-I$(CHIPINC) \
	-I$(RTC_SRC) \
	-I$(LIB_TAM_ACT2_SRC) \
	-I$(DEV_TELIT_SRC) \
	-I$(MTD_SDK_DIR) \
	-I$(HIGHRISE_DIR)

#
# defines for compiler
#

DEFS = \
	-DHIGHRISE\
	-DLINUX_APP \
    -DTFTP_CLIENT \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	#-DMTD_CLAUSE_22_MDIO
#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS     = -Wall -Werror $(INC) ${DEFS}
CCOPTS		= -g -fsigned-char -pipe -Wa,
ASOPTS		= -fsigned-char -float
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS} ${LOCAL_CFLAGS}
CFLAGS		+= -DBOARD_P0

#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS      += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

#
#
# Build ACT2 lib into project
#
#
LIB_TAM_ACT2_SRC = /auto/sp-engops/diags/pld/act2lite/ARM/TSN/SUDI_2099_Aikido
LIB_TAM_ACT2 = $(LIB_TAM_ACT2_SRC)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing_diags.a
#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
TARGET	= highrise

#
# Common device driver objects
#
DEV_CORE_OBJS = \
    dev_object.o


DEV_OBJS = \
	dev_1337.o \
	dev_lte_telit.o \
	dev_lte_telit_at.o

# marvell 88X3310P SDK
MTD_OBJS = \
	aescrypt.o \
	aeskey.o \
	aestab.o \
	mtdAPI.o \
	mtdAPIInternal.o \
	mtdCunit.o \
	mtdDiagnostics.o \
	mtdDiagnosticsRegDumpData.o \
	mtdEEE.o \
	mtdFwDownload.o \
	mtdHXunit.o \
	mtdHunit.o \
	mtdHwCntl.o \
	mtdHwMsecPtpCntl.o \
	mtdInitialization.o \
	mtdIntr.o \
	mtdMsecApi.o \
	mtdMsecInternal.o \
	mtdPtpApi.o \
	mtdPtpConst.o \
	mtdPtpInit.o \
	mtdPtpInternal.o \
	mtdPtpUtils.o \
	mtdSemaphore.o \
	mtdUtils.o \
	mtdXgFastRetrain.o \
	mtdXunit.o

#
# Pluggable Testcard
#


#
# Pluggable Serial(host)
#

#
# Common object files
#


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
	linux_api.o \
	bit_swaps.o \
	print_hex.o \
	linux_coretest.o \
	linux_i2c.o \
	cookie_4_core.o \
	tam_act2_utils.o \
	$(DEV_CORE_OBJS) \
	$(DEV_OBJS) \


PLAT_OBJS = \
	linux_main.o \
	linux_popen.o \
	diag.o \
	stubs.o \
	platform_cpu.o \
	platform_storage.o \
	mb_tests.o \
	highrise.o \
	gpio.o \
	mem_rw.o \
	platform_stub.o \
	platform_emmc.o \
	full_load.o \
	platform_rtc.o \
	i2c_api.o \
	platform_i2c.o \
	platform_sensor.o \
	platform_cookie.o \
	tam_act2_api_drv_support.o \
    tam_lib_object_manager.o \
	diag_lte_test.o \
	diag_lte_telit_lib.o \
	diag_lte_telit_util.o \
	diag_lte_host_impl.o \
	smi_drv.o \
	xsmi_drv.o \
	platform_phy_fwcode.o \
	platform_phy.o \
	platform_uart.o \
	highrise_eth_traf.o \
	platform_intr.o \
	hr_commn_util.o

CPLD_OBJS = \
	highrise_cpld_api.o \
	highrise_cpld_lib.o \
	highrise_cpld_diag.o \
	highrise_cpld_fw.o

JBI_LIB = $(HIGHRISE_DIR)/common/jbi/jbi.obj

OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(CPLD_OBJS) \
	$(MTD_OBJS)

NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(CPLD_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(MTD_OBJS) \
	$(JBI_LIB)

all: $(TARGET)

clean:
	$(MAKE) -C $(HIGHRISE_DIR)/common/jbi clean
	rm -rf *.o banner.c $(TARGET)-diag.v* .*.prep *.d


$(JBI_LIB):
	$(MAKE) -C $(HIGHRISE_DIR)/common/jbi JBI_STANDALONE=0 PASSIN_INC_DIRS=-I../../ CROSS_COMPILE=$(CROSS_COMPILE)

$(TARGET) : ENV_PRT $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh HIGHRISE
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(NET_OBJS) banner.o -o $(TARGET) ${LDFLAGS}
	cp $(TARGET) $(TARGET)-diag.v$(MAJOR_VERSION).$(MINOR_VERSION).$(RELEASE)
	cp $(TARGET) $(TARGET)-diag.v$(MAJOR_VERSION).$(MINOR_VERSION).$(RELEASE)-$(BUILD_VAR)
	@$(OBJDUMP) -t $@ > $@.symbols

ENV_PRT :
	@echo "##########################################################"
	@echo "#################### Env Variables #######################"
	@echo "##########################################################"
	@echo "PATH: " $(PATH)
	@echo "CROSS_COMPILE: " $(CROSS_COMPILE)
	@echo "IROOT: " $(IROOT)
	@echo "HIGHRISE_DIR: " $(HIGHRISE_DIR)
	@echo "COMSRCDIR: " $(COMSRCDIR)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "##########################################################"

%.d : %.c
	$(CC) -c -E $< $(CFLAGS) -Wp,-MMD,$@ > /dev/null

%.o : %.c
	$(CC) -c -E $< $(CFLAGS) -o .$@.prep
	$(CC) -c $< $(CFLAGS)
	
sinclude ${OBJS:.o=.d}

#Keep the PHONY depend for compitable with daily regression build script which service old projects
depend:
	cat *.d > dependancies

vpath %.c \
	$(HIGHRISE_DIR): \
	$(HIGHRISE_DIR)/common: \
	$(HIGHRISE_DIR)/common/mdio: \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(RTC_SRC): \
	$(DEV_TELIT_SRC): \
	$(LIB_TAM_ACT2_SRC): \
	$(MTD_SDK_DIR)

vpath %.h \
	$(INCDIR): \
	$(HIGHRISE_DIR): \
	$(RTC_SRC): \
	$(DEV_TELIT_SRC): \
	$(LIB_TAM_ACT2_SRC): \
	$(MTD_SDK_DIR): \


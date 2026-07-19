# $Id: Makefile,v 1.3 2021/04/15 01:55:08 peteteng Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Neptune x86 diag image
#
# Adapted from overlord/x86.
#
# Copyright (c) 2016-2019 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#

###################################################################
#
# NOTE: When new files are added please sort it properly.
#
###################################################################

#
# NOTE: Need to build on a Linux box.
#

#
# Crash us if /bin/make is used:
#
SERVER:=$(HOSTNAME)

ifeq ($(origin USER),undefined)
USER=Bozo
endif

default: all

TOOLS_VER	= c4.7.0-p1
TOOLS_ARCH	= x86_64

#
# Tool Chain
#

CC		= gcc.$(TOOLS_VER).$(TOOLS_ARCH)
AS		= as
AR		= ar.$(TOOLS_VER).$(TOOLS_ARCH)
LD		= ld
MAKE		= make 
NM		= nm.$(TOOLS_VER).$(TOOLS_ARCH)
OBJCOPY		= objcopy.$(TOOLS_VER).$(TOOLS_ARCH)
OBJDUMP		= objdump.$(TOOLS_VER).$(TOOLS_ARCH)
SIZE		= size.$(TOOLS_VER).$(TOOLS_ARCH)
STRIP		= strip.$(TOOLS_VER).$(TOOLS_ARCH)

.PHONY: all clean clobber depend force-tools install

SHELL		= /bin/sh
OPTIONS 	=
IROOT       = ../../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

SRCDIR		= .
OVERLORDDIR     = $(IROOT)/common/src/overlord
KLMDIR		= $(IROOT)/klm
KATARDIR = $(IROOT)/common/src/katar
AQRINCDIR = $(KATARDIR)/x86/AQR4xx/include
AQRREGINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap
AQRAPPIAINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/APPIA
AQRATLINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/ATL
AQRCALINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/CAL
AQREURINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/EUR
AQRHHDINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/HHD
AQRRHEAINCDIR = $(KATARDIR)/x86/AQR4xx/include/registerMap/RHEA
AQRSRCDIR = $(KATARDIR)/x86/AQR4xx/src
#
# The processor specific source directory
#

PROCSRCDIR	= $(IROOT)/x86/src
PROCINCDIR	= $(IROOT)/x86/include

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include

CLK1_SRC	= $(DEV_DRV)/dev_idt_ics844s2416i
DIMM_SRC	= $(DEV_DRV)/dev_dimm
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel
ENV_SRC		= $(DEV_DRV)/dev_csco_10698
I2CMX_SRC	= $(DEV_DRV)/dev_pca9545a_ti
KERNDIR		= $(COMSRCDIR)/kernel
M29312DIR	= $(COMSRCDIR)/m29312
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
SNSR_SRC	= $(DEV_DRV)/dev_max1617a
Socrates 	= $(COMSRCDIR)/Socrates
TDM_PLL_SRC	= $(DEV_DRV)/dev_idt82v32021
USB_SRC		= $(DEV_DRV)/dev_cy7c64215_cypress
UTIL_DIR	= $(IROOT)/utils
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client
POE30_SRC = $(DEV_DRV)/dev_ilp
SHEDIR_SRC_PATH = $(COMSRCDIR)/shedir
RBCP_LIB_PATH = $(COMSRCDIR)/rbcp_lib
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido_v3_4_24/

BCM_LDFLAGS =  -lpthread -lm -lrt -Wl,--relax


# Cavium remote lib
#
CVMX_SDK_ROOT = /auto/neptune/vendor-src/cavium/cvm-sdk-3.1.2/Cavium_Networks/OCTEON-SDK/
CVMX_REMOTE_LIB_PATH = $(CVMX_SDK_ROOT)/host/remote-lib
CVMX_REMOTE_LIB = 

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(SRCDIR) \
	-I$(INCDIR) \
    -I$(OVERLORDDIR) \
	-I$(AQRINCDIR) \
	-I$(AQRREGINCDIR) \
	-I$(AQRAPPIAINCDIR) \
	-I$(AQRHHDINCDIR) \
	-I$(AQREURINCDIR) \
	-I$(AQRCALINCDIR) \
	-I$(PROCINCDIR) \
	-I$(KERNDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(CLK1_SRC) \
	-I$(CLK2_SRC) \
	-I$(CLKBUF_SRC) \
	-I$(DIMM_SRC) \
	-I$(EEPROM_SRC) \
	-I$(ENV_SRC) \
	-I$(I2CMX_SRC) \
	-I$(SNSR_SRC) \
	-I$(TDM_PLL_SRC) \
	-I$(USB_SRC) \
	-I$(POE30_SRC) \
	-I$(SHEDIR_SRC_PATH) \
	-I$(RBCP_LIB_PATH) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(TAMLIB)

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
# add -DSREC2BIN for power sequencer fw upgrade
#

DEFS	= \
    -DOVERLORD \
	-DLINUX_APP \
	-DSREC2BIN \
	-DDIAGMON \
	-DDRV_EXTENDED_REG_TEST \
	-DVIR_IS_PHY \
	-DL_ENDIAN \
	-DINFORMERS_LNX_APP \
	-DTFTP_CLIENT \
	-DTAM_2099_SUDI \
	-DSHA1_SHA2_SWSUDI2099 \
	-DSUDI_2099_2029
#	-DENABLE_SUDI_2099_CLI

DREAMLINER_DEFS = \
	-DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS

# If BANNER is on, add DISPLAY_ENGINEERING_BANNER to the DEFS.
# `make clean`
# `make BANNER=on` to display the engineering banner
# `make` to ignore the banner
ifeq ($(BANNER),on)
DEFS                += -DDISPLAY_ENGINEERING_BANNER
endif

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#
CPPOPTS         = -Wall -Werror -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=implicit-function-declaration $(INC) ${DEFS} $(DREAMLINER_DEFS)
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
ASFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

#LIBDIR		= $(IROOT)/lib
#LIBSRCDIR	= $(LIBDIR)/src
#LIB_AUTH	= smart_cookie_auth_c2_95_3-p11a-linux.a

# overlord is 64 bit
LIB_AUTH_DIR = /auto/sp-engops/diags/pld/lib/intel/Linux

# achanta is 32 bit lib, overlord is 64 bit
PLDLIB_INTEL_LINUX_ACHANTA = /auto/sp-engops/diags/pld/lib/intel/Linux

LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_ACT2_OBJ = $(TAMLIB)/tam_lib_objname.a

#LIB_CRYPTO = $(PLDLIB_INTEL_LINUX_ACHANTA)/cryptoc_rsa_toolkit.a
#LIB_CERTC = $(PLDLIB_INTEL_LINUX_ACHANTA)/certc_rsa_toolkit.a
LIB_QUACK = $(PLDLIB_INTEL_LINUX_ACHANTA)/ec_rsa_toolkit.a

LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= katarx86_diag
NETELF		= $(NETTARGET).elf
NETTARGET_BZ2	= $(NETTARGET).bz2

#
# Use -Qoption,bepostld,-verbose to turn on postld verbose 
# to see whether postld do the necessary big-endian global pointers swap
# POSTLDVERBOSE	= -Qoption,bepostld,-verbose
#
POSTLDVERBOSE	=
#RAMLDFLAGS	= -Xlinker -e -Xlinker -Tinformersram.link \

RAMLDFLAGS 	=




#
# Lib sources
#


CRYPTO_OBJS = \
	$(LIB_AUTH_DIR)/crypto_utilities.a \
  
#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_OBJS = \
	dev_32021.o \
	dev_idt_ics844s2416i.o \
	dev_ltc4215.o \
	dev_object.o \
	dev_pca9545.o \
	dev_at24c0n.o \
	dev_dimm.o \
	dev_1337.o \
	dev_csco_10698.o \
	dev_ilp.o \
	dev_cy7c64215.o


DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

DEV_BASE = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.c \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
	$(DEV_DRV)/dev_object.c \
	$(TDM_PLL_SRC)/dev_32021.c \
	$(I2CMX_SRC)/dev_pca9545.c \
	$(EEPROM_SRC)/dev_at24c0n.c \
	$(DIMM_SRC)/dev_dimm.c \
	$(ENV_SRC)/dev_csco_10698.c \
	$(POE30_SRC)/dev_ilp.c \
	$(USB_SRC)/dev_cy7c64215.c \


DEV_HFILES = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.h \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
	$(DEV_DRV)/dev_object.h \
	$(TDM_PLL_SRC)/dev_32021.h \
	$(I2CMX_SRC)/dev_pca9545.h \
	$(EEPROM_SRC)/dev_at24c0n.h \
	$(DIMM_SRC)/dev_dimm.h \
	$(RTC_SRC)/dev_1337.h \
	$(ENV_SRC)/dev_csco_10698.h \
	$(POE30_SRC)/dev_ilp.h \
	$(USB_SRC)/dev_cy7c64215.h

#
# Common object files
#

COM_OBJS = \
	linux_main.o \
	linux_i2c.o \
	linux_pciutils.o \
	linux_error.o \
	linux_stub.o \
	linux_mem.o \
	linux_uart.o \
	linux_pstat.o \
	linux_api.o \
	linux_memory_tester.o \
	linux_memory_tester_utils.o \
	menu.o \
	common_utils.o \
	print_hex.o \
	alias.o \
	parsetoken.o \
	builtin.o \
	linux_pcie.o \
	query.o \
	memops.o \
	testmem.o \
	crypto_credential.o \
	cookie_4_core.o \
	bit_swaps.o \
	uio_utils.o \
	linux_usb_test.o \
	tftpclient.o \
	tam_act2_utils.o \
    srec2bin.o


COM_BASE = \
	$(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_pciutils.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_uart.c \
	$(COMSRCDIR)/linux_pstat.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/linux_memory_tester.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/hwic_stub.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/print_hex.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/linux_pcie.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/crypto_credential.c \
	$(COMSRCDIR)/cookie_4_core.c \
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/uio_utils.c \
    $(COMSRCDIR)/linux_usb_test.c \
	$(COMSRCDIR)/tftpclient.c \
    $(COMSRCDIR)/tam_act2_utils.c


COM_HFILES = \
	$(INCDIR)/aglobal.h \
	$(INCDIR)/atypes.h \
	$(INCDIR)/bexterr.h \
	$(INCDIR)/bitdef.h \
	$(INCDIR)/bsafe.h \
	$(INCDIR)/bsfmacro.h \
	$(INCDIR)/bsfplatf.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/common.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/console.h \
	$(INCDIR)/cookie_4.h \
	$(INCDIR)/cpu.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/crypto_credential.h \
	$(INCDIR)/crypto_ecdsa.h \
	$(INCDIR)/defs.h \
	$(INCDIR)/dev_print.h \
	$(INCDIR)/dnld.h \
	$(INCDIR)/duart16552.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/error.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/extern.h \
	$(INCDIR)/flash.h \
	$(INCDIR)/free.h \
	$(INCDIR)/hwic_slot.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/ich9_i2c_regs.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_eth.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/menu.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/new_proto.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/pca9545a.h \
	$(INCDIR)/pca.h \
	$(INCDIR)/pci.h \
	$(INCDIR)/pm_utils.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/queryflags.h \
	$(INCDIR)/reason.h \
	$(INCDIR)/resizeob.h \
	$(INCDIR)/router_if.h \
	$(INCDIR)/s2681.h \
	$(INCDIR)/sgmii_defs.h \
	$(INCDIR)/signals.h \
	$(INCDIR)/smart_cookie_auth.h \
	$(INCDIR)/sm_patriot.h \
	$(INCDIR)/spi_cookie.h \
	$(INCDIR)/strings.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/sh.h \
	$(INCDIR)/slot.h \
	$(INCDIR)/smart_cookie.h \
	$(INCDIR)/sm_slot.h \
	$(INCDIR)/stdlibrf.h \
	$(INCDIR)/t1e1_common.h \
	$(INCDIR)/term.h \
	$(INCDIR)/testmem.h \
	$(INCDIR)/types.h \
	$(INCDIR)/uio_utils.h \


#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
    platform_stub.o \
	diag.o \
	ich9_i2c.o \
	platform_i2c.o \
	platform_prom.o \
	tam_aikido_upgrade.o \
	AQ_API.o \
	AQ_API_flash.o \
	mb_tests.o \
	dash_reg_test.o \
	dash_mem.o \
	platform_temp_sensor.o \
    platform_fpga.o \
    platform_led.o \
    platform_eth_util.o \
    platform_fan.o \
    diag_boot_flash_test.o \
    platform_cookie.o \
    tam_act2_api_drv_support.o \
    tam_lib_object_manager.o \
	platform_ext_lpbk.o \
    platform_rtc.o \
	platform_poe.o \
	platform_i2c_api.o \
	platform_aikido_fpga.o \
    diag_fpga_i2c_lib.o \
    platform_intr_test.o \
	linux_popen.o


PLAT_BASE = \
	linux_main.c \
	diag.c \
	mb_tests.c \
	platform_i2c.c \
    platform_stub.c \
	platform_led.c \
	platform_eth_util.c \
	platform_prom.c \
	tam_aikido_upgrade.c \
	$(AQRSRCDIR)/AQ_API.c \
	$(AQRSRCDIR)/AQ_API_flash.c \
	dash_reg_test.c \
	platform_fan.c \
    platform_fpga.c \
    diag_boot_flash_test.c \
    platform_cookie.c \
    tam_act2_api_drv_support.c \
    $(TAMLIB)/tam_lib_object_manager.c \
	platform_ext_lpbk.c \
    platform_rtc.c \
	platform_poe.c \
	platform_i2c_api.c \
	platform_aikido_fpga.c \
    diag_fpga_i2c_lib.c \
    platform_intr_test.c \
	linux_popen.c


PLAT_HFILES = \
	mb_tests.h \
	cookie_plat.h \
	platform_i2c_api.h \
	i2c_address.h \
    platform_fpga.h \
    platform_cookie_api.h \
    tam_act2_api_drv_support.h \
    platform_aikido.h \
    diag_fpga_i2c_lib.h

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(CVMX_REMOTE_LIB) \
	$(CRYPTO_OBJS) \
	$(LIB_PCI_UTILS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
    $(LIB_TAM_ACT2_OBJ) \
	$(LIB_QUACK)

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE)

#
#########   Header Files #########
#

HFILES  = \
        $(COM_HFILES) \
        $(DEV_HFILES) \
        $(PLAT_HFILES)

#
#########   ASM Files #########
#

ASM	= \
	$(PLAT_AFILES)

vpath %.c \
	$(SRCDIR): \
	$(AQRSRCDIR): \
    $(OVERLORDDIR): \
	$(COMSRCDIR): \
	$(PROCSRCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(CX27470): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DIMM_SRC): \
	$(DOBRO_FW_PATH): \
	$(DSP_5510_FW_PATH): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(ENZO_UCODE): \
	$(ENZOPLUS_UCODE) \
	$(FPGA_IMAGE_PATH): \
	$(GE_QUAD_PHY_SRC): \
	$(I2CMX_SRC): \
	$(M29312DIR): \
	$(NEZHA_DIR): \
	$(PATH_OF_BORG_FW): \
	$(PATH_OF_DSL_FW): \
	$(PATH_OF_BORG_FPGA): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(RTC_SRC): \
	$(LM75_SRC): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(Socrates): \
	$(SPIDEY_PLD): \
	$(SPIDEY_T3_UCODE): \
	$(SPIDEY_UCODE): \
	$(TDM_PLL_SRC): \
	$(USB_SRC): \
	$(MONTALVO_FW_PATH): \
	$(PRECISION_IMAGE_PATH): \
	$(SQUIER_IMAGE_PATH): \
	$(CX98124): \
	$(TROI_BASE): \
	$(GOOFY): \
	$(SHEDIR_SRC_PATH): \
	$(RBCP_LIB_PATH): \
    $(TAMLIB):

vpath %.h \
	$(SRCDIR): \
    $(OVERLORDDIR): \
	$(PROCINCDIR): \
	$(INCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	$(CHIPINC): \
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DEV_DRV)/dev_3086_infineon: \
	$(DEV_DRV)/dev_82912_infineon: \
	$(DIMM_SRC): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(GE_QUAD_PHY_SRC): \
	$(I2CMX_SRC): \
	$(M29312DIR): \
	$(NEZHA_INCDIR): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(LM75_SRC): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(RBCP_LIB_PATH): \
	$(TAMLIB):

# Override RCS stuff

#%: %,v

#%: RCS/%,v

.SUFFIXES:
.SUFFIXES: .o .lis .S .c .s .dis

%.o : %.S
	$(CC) -c $(ASFLAGS) $<
%.o : %.c
	$(CC) -c $(CFLAGS) $<
%.lis : %.c
	lister3 CC=$(CC) AS=$(AS) $< $(CFLAGS) | grep -v '.stab' >$*.lis

%.dis : %.o
	$(OBJDUMP) -l --disassemble --source $< > $*.dis

%.i : %.c
	$(CC) -E $(CFLAGS) $< > $*.i

%.s : %.c
	$(CC) -S -fsource-asm $(CFLAGS) $< 

#
# No ROM version for informers
#

all: $(NETTARGET_BZ2)

#
# do not delete the last line for object dump just in case
# we need to do this when debugging a problem. temp comment out
#
#install: $(SRECFILE) $(NETTARGET) install.sh
#
# temp comment out ROM version due to compiler update problem
#

install: $(NETTARGET) install.sh
	sh install.sh
	touch install.sh
	$(OBJDUMP) -l --disassemble --source $(NETELF) > $(NETTARGET).dis


clean:
	rm -f *.o  banner.c \
	tools temp.* *.dat .compile ID.temp

cleanall: clean
	rm -f dependancies $(NETTARGET).bz2 $(NETTARGET).bz2.symbols $(NETTARGET).symbols 

clobber: clean
	rm -f moncode* pfsi syms*

superclean: clobber
	$(MAKE) --no-print-directory -C $(UTIL_DIR) clean
	$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc clean

force-tools:
	rm -f tools

tools:
	@$(MAKE) --no-print-directory -C $(UTIL_DIR) depend $(UTOOLS)
	@$(MAKE) --no-print-directory -C $(IROOT)/sys/obj-4k-c3640 mksum_elf
	@$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc depend $(HTOOLS)

TARGET_DEPEND = \
	Makefile \
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : $(TARGET_DEPEND) $(HFILES)
	sh katar_banner.sh
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) \
	${BCM_LDFLAGS}  $(LSPCI_LDFLAGS)

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols
	touch $(NETTARGET).bz2.symbols

$(NETTARGET_BZ2): $(NETTARGET)
	bzip2 -9 -f -k $(NETTARGET)
	mv $(NETTARGET) $(NETTARGET).orig

#	$(CC) $(CFLAGS)
#	$(LD) -o $(TARGET_OBJS)
#        @sh $(IROOT)/utils/banner.sh INFORMERS
#        $(CC) $(CFLAGS) -c banner.c
#        $(LD) -o $(TARGET_OBJS)

#$(NETTARGET): $(TARGET_DEPEND)
#	$(CC) $(CFLAGS) $(TARGET_OBJS) -o $(NETTARGET)


#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES) $(ASM)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) $(ASM) > dependancies
	@echo "Computing TAGS file for common/src"
	$(shell which etags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	 -lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)

scope:
	cscope -b $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies
	sinclude dependancies




# DO NOT DELETE THIS LINE -- make depend depends on it.

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.3  2021/04/15 01:55:08  peteteng
# Upgrade to TAM Lib-v3.4.24 based on PRRQ#5091945
#
# Revision 1.2  2019/06/14 05:24:47  mikech2
# Collapse katar-branch00 to Main Trunk
#
# Revision 1.1.2.22  2019/06/10 03:47:18  mikech2
# Remove platform_fru.h base on PRRQ#4685780 Comment#6
#
# Revision 1.1.2.21  2019/06/10 01:34:01  mikech2
# Remove diag_i2c_addr.h base on PRRQ#4685780 Comment#3
#
# Revision 1.1.2.20  2019/06/06 02:44:25  mikech2
# clean out the unnecessary definitions and TSN string
#
# Revision 1.1.2.19  2019/05/29 05:59:17  mikech2
# Code cleanup
#
# Revision 1.1.2.18  2019/04/30 06:06:58  mikech2
# Code cleanup
#
# Revision 1.1.2.17  2019/04/26 01:17:33  mikech2
# clean up Makefile
#
# Revision 1.1.2.16  2019/04/16 03:23:58  mikech2
# Code cleanup
#
# Revision 1.1.2.15  2019/04/12 01:35:54  peteteng
# Code cleanup
#
# Revision 1.1.2.14  2019/03/14 08:36:58  peteteng
# Code cleanup
#
# Revision 1.1.2.13  2019/03/14 03:58:52  peteteng
# Code cleanup
#
# Revision 1.1.2.12  2019/03/08 07:19:19  mikech2
# Clean up codes
#
# Revision 1.1.2.11  2019/03/05 03:33:35  mikech2
# Use Aquantia tool dump SFP eeprom
#
# Revision 1.1.2.10  2019/03/04 06:21:25  mikech2
# Remove unnecessary files
#
# Revision 1.1.2.9  2019/03/04 02:29:53  peteteng
# Move file from overlord to katar folder
#
# Revision 1.1.2.8  2019/03/04 00:45:13  mikech2
# Clean up codes and remove unnecessary files
#
# Revision 1.1.2.7  2019/02/12 08:06:28  mikech2
# rename katar_*.h files
#
# Revision 1.1.2.6  2019/01/29 01:54:19  mikech2
# rename katar_* files
#
# Revision 1.1.2.5  2019/01/04 06:07:16  peteteng
# Add SUDI_2099_2029
#
# Revision 1.1.2.4  2018/12/20 09:10:56  peteteng
# Add FPGA I2C read/write/scan/reset util
#
# Revision 1.1.2.3  2018/11/14 08:14:58  peteteng
# Add Aikido FPGA register test
#
# Revision 1.1.2.2  2018/11/08 02:21:52  peteteng
# Remove names in comment
#
# Revision 1.1.2.1  2018/10/22 08:02:33  mikech2
# Move project folder to common/src/katar/x86
#
# Revision 1.1.2.26  2018/10/22 03:04:32  peteteng
# use common code tam_act2_utils.c without smart_cookie.c
#
# Revision 1.1.2.25  2018/10/19 07:51:40  iachang
# Update TAM lib. from v3.3.25 to v3.4.4, fixed endian function naming issue
#
# Revision 1.1.2.24  2018/10/18 02:32:25  mikech2
# Add RTC test in mb_tests.c
#
# Revision 1.1.2.23  2018/09/25 06:26:49  peteteng
# Update Katar PCAMAP
#
# Revision 1.1.2.22  2018/09/07 03:14:20  peteteng
# Add system info utility
#
# Revision 1.1.2.21  2018/09/04 06:09:08  mikech2
# Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
#
# Revision 1.1.2.20  2018/07/24 09:54:12  peteteng
# Add SFP cookie - read
#
# Revision 1.1.2.18  2018/07/17 11:40:46  benlu
# For i2c scan / Temperature / PoE diag test
#
# Revision 1.1.2.17  2018/07/12 08:02:08  peteteng
# add tam_lib_platform_write/read
#
# Revision 1.1.2.16  2018/07/10 10:29:05  benlu
# Add PHY internal/exteranl loopback and AQR4xx API
#
# Revision 1.1.2.15  2018/07/10 07:50:30  mikech2
# Add security FPGA FW update util
#
# Revision 1.1.2.14  2018/07/02 09:12:50  mikech2
# Add Ethernet utility Menu
#
# Revision 1.1.2.13  2018/07/02 08:21:10  peteteng
# Update katar tam lib
#
# Revision 1.1.2.12  2018/06/29 07:48:27  mikech2
# Add make cleanall command
#
# Revision 1.1.2.11  2018/06/29 07:17:31  mikech2
# Remove compile warning and unused files
#
# Revision 1.1.2.10  2018/06/29 03:40:01  peteteng
# Add ACT2 utility Menu
#
# Revision 1.1.2.9  2018/06/28 03:32:55  mikech2
# Add interrupt mask control menu
#
# Revision 1.1.2.8  2018/06/27 07:45:16  mikech2
# remove unused test in mb_test
#
# Revision 1.1.2.7  2018/06/26 06:30:09  peteteng
# Add Aikido Cookie menu
#
# Revision 1.1.2.6  2018/06/25 08:24:52  mikech2
# Add interupt test menu
#
# Revision 1.1.2.5  2018/06/21 08:24:09  mikech2
# remove unused menu, add scratchpad reg test
#
# Revision 1.1.2.4  2018/06/20 07:32:01  mikech2
# Add fan/led/margin control menu
#
# Revision 1.1.2.3  2018/06/11 07:05:53  peteteng
# add bootflash test from viper
#
# Revision 1.1.2.2  2018/06/07 03:23:53  peteteng
# fix diag launch issue
#
# Revision 1.1.2.1  2018/06/07 01:19:22  peteteng
# add project katar - based on neptune
#
#
#
# $Endlog$

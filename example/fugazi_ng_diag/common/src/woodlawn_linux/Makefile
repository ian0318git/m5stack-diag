# $Id: Makefile,v 1.3 2013/11/26 08:40:39 hroni Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/Makefile,v $
#------------------------------------------------------------------
# Makefile - Makefile to build Woodlawn cavium data plan diag image
#
# January 2012, Kody Ko
#
# Copyright (c) 2013 by Cisco Systems, Inc.
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

OCTEON_SDK_DIR := ../../../../OCTEON-SDK
CROSS_COMPILER_DIR=$(OCTEON_SDK_DIR)/tools/bin

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
WOODLAWNDIR = ..
FPGA_PATH	= /auto/sp-engops/diags/pld/fpga_image

# Code signing utility
#
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client

# Path to fpga images
FPGA_IMAGE_PATH = /auto/sp-engops/diags/pld/fpga_image

GE_QUAD_PHY_SRC = $(DEV_DRIVER)/dev_88e1340_marvell

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
GE_PHY_SRC = $(DEV_DRIVER)/dev_88e111x_marvell
GE_QUAD_PHY_SRC = $(DEV_DRIVER)/dev_88e1340_marvell
NM_ADPTR_CPLD_SRC = $(DEV_DRIVER)/dev_nm_adptr_cpld
AP_CON_SRC = $(DEV_DRIVER)/dev_ap_console

# IO module diag directories
#
SOCRATES = $(COMSRCDIR)/Socrates  #needed for CLI
NIOS2_FIREBEE = $(COMSRCDIR)/nios2_firebee/obj-nios-II
ISM_TESTCARD = $(COMSRCDIR)/ism_testcard
PSE2DIR = $(COMSRCDIR)/pse2_testboard
CX27470 = $(COMSRCDIR)/cx27470  #need for CLI



LIB = librsa_elf_multi.a
LIB_QUACK = ec_rsa_tk-obj-4k-linux64_diag.a
PLDDIR = /nfs/sp-engops/diags/pld/lib/OcteonLinux64/
LIBDIR = $(IROOT)/lib/elf_multi
DSP_UTILS = ../../dsp_src/utils
PSE_FPGA_PATH= /auto/sp-engops/diags/pld/pse

# Path to 5510 DSP FW for Guido & Dobro.
# IMPORTANT NOTE: PLEASE COPY the file dsp_5510_fw.c from
# /auto/sp-engops/diags/pld/sync dir to /auto/sp-engops/diags/pld/dsp5510 dir
# when sync branch collapses to main trunk.
#

PRECISION_IMAGE_PATH = /nfs/sp-engops/diags/pld/precision
SQUIER_IMAGE_PATH = /nfs/sp-engops/diags/pld/squier
MONTALVO_FW_PATH = /auto/sp-engops/diags/pld/sp26dsp
XFORMERS_SPI_PROM_PATH = /auto/sp-engops/diags/pld/xformers/spi_prom
KLMINC = $(IROOT)/klm/.

# Cavium Octeon SDK and lib paths
OCTEON_ROOT = $(OCTEON_SDK_DIR)
CVMX_SDK_INC_PATH = $(OCTEON_ROOT)/executive 
CVMX_LIB = $(SRCDIR)/libcvmx.a

INC = -I$(INCDIR) \
	-I$(CVMX_SDK_INC_PATH) \
	-I$(SRCDIR) -I. -I$(DEV_DRIVER) -I$(CHIPINC) -I$(PROCINCDIR) \
	-I$(NM_ADPTR_CPLD_SRC) -I$(KLMINC) -I$(CLK2_SRC) -I$(SM_PCLK_SRC) -I$(CLK1_SRC) -I$(SNSR_SRC) -I$(LM75_SRC) \
	-I$(PSE2_SRC) -I$(LTC4215_SRC) \
	-I$(TDM_PLL_SRC) -I$(ENV_SRC) -I$(CX27470) -I$(EEPROM_SRC) -I$(DEV_DRIVER)/dev_98dxn17_marvell -I$(SOCRATES) \
	-I$(RTC_SRC) -I$(TMP421_SRC) -I$(DEV_DRIVER)/dev_pmc_4359 -I$(RPS_SRC) -I$(USB_SRC) \
	-I$(DIMM_SRC) -I$(I2CMX_SRC) -I$(WOODLAWNDIR) -I$(GE_QUAD_PHY_SRC) -I$(GE_PHY_SRC) -I../woodlawn

#OCTEON_MODEL = OCTEON_CN63XX_PASS1_2
OCTEON_MODEL = OCTEON_CN66XX_PASS1_0
OCTEON_TARGET = linux_64
OCTEON_CPPFLAGS_GLOBAL_ADD= -DUSE_RUNTIME_MODEL_CHECKS=1 -DCVMX_ENABLE_PARAMETER_CHECKING=0 -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DCVMX_ENABLE_POW_CHECKS=0

#linux_path=${OCTEON_ROOT}/linux
#kernel=-ld0x11000000:${linux_path}/kernel_2.6/linux/vmlinux.64
#filesystem=-ld0x40000000:${linux_path}/minimal_rootfs/minimal_rootfs

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
CFLAGS_LOCAL += -g -Wall -Werror -Wno-unused-parameter -DLINUX_APP -DLINUX_64BIT -D_GNU_SOURCE $(INC) -DVINDICATOR \
	-DRUN_FROM_CACHE -L$(PLD)/$(LIB_QUACK) -DEVAL_BD -DWOODLAWN

COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -o $@ $<
DEPCOMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) 
COMPILE_NOWARN = $(COMPILE)
COMPILE_NOG = $(COMPILE)
ASSEMBLE = $(CC) $(ASFLAGS_GLOBAL) $(ASFLAGS_LOCAL) -MD -c -o $@ $<

.PHONY: all clean clobber depend force-tools install superclean

NETTARGET = woodlawnnet

# Common Device Driver
CBASE = \
	$(SRCDIR)/fdt.c \
	$(SRCDIR)/fdt_ro.c \
	$(SRCDIR)/cavium_register_access.c \
	$(SRCDIR)/stub.c \
    	$(SRCDIR)/diag_fpga_lib.c \
   	$(SRCDIR)/diag_fpga_fw.c \
	$(SRCDIR)/diag_xaui_88X2222M_lib.c \
	$(SRCDIR)/diag_xaui_88X3120_lib.c \
	$(SRCDIR)/diag_ge_phy_88E1548L_lib.c \
	$(SRCDIR)/diag_ge_phy_88E1340_lib.c \
	$(SRCDIR)/diag_fpga_test.c \
	$(SRCDIR)/diag_temperature_sensor_lib.c \
	$(SRCDIR)/diag_temperature_sensor_test.c \
    	$(SRCDIR)/diag_ge_phy_88E1340_test.c \
	$(SRCDIR)/diag_ge_phy_88E1548L_test.c \
    	$(SRCDIR)/diag_bootflash_test.c \
	$(SRCDIR)/diag_bootflash_lib.c \
    	$(SRCDIR)/diag_xaui_88X3120_test.c \
	$(SRCDIR)/diag_xaui_88X2222M_test.c \
    	$(SRCDIR)/diag.c \
    	$(SRCDIR)/diag_common_drv.c \
	$(SRCDIR)/diag_common_lib.c \
	$(COMSRCDIR)/linux_eth.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_i2c.c \
	$(SRCDIR)/main.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/menu.c \
	$(SRCDIR)/woodlawn_common_utils.c \
	$(SRCDIR)/woodlawn_linux_error.c \
	$(SRCDIR)/platform_eth.c \
	$(SRCDIR)/platform_ext_lpbk.c \
	$(SRCDIR)/platform_sfp_ext_lpbk.c \
	$(SRCDIR)/platform_sgmii.c \
	$(SRCDIR)/platform_xaui.c \
	$(SRCDIR)/platform_smi.c \
	$(SRCDIR)/platform_sfp_cookie.c \
    	$(SRCDIR)/platform_i2c.c \
	$(SRCDIR)/diag_phy_3120_fw.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/testmem.c \
        $(SRCDIR)/diag_pll_test.c \
        $(SRCDIR)/diag_pll_pll.c \
        $(SRCDIR)/diag_backplane_xaui_test.c \
        $(SRCDIR)/diag_tlk10232_test.c \
        $(SRCDIR)/diag_tlk10232_lib.c \
        $(SRCDIR)/diag_ge_phy_88E1112C_test.c \
        $(SRCDIR)/diag_ge_phy_88E1112C_lib.c \
# The following .o files have compiled warnings suppressed (no -Wall option)
# NOTE: ONLY VENDOR FILES should be compiled without -Wall option
$(CVMX_OBJS):  %.o : %.c
	$(COMPILE_NOWARN)


# New CLI command object files
NEW_CLI_OBJS =  cli_cmd.o
NEW_CLI_BASE = $(COMSRCDIR)/cli_cmd.c
NEW_CLI_HFILES = $(INCDIR)/cli_cmd.h


HFILES=	\
	$(SRCDIR)/diag_fpga_test.h \
	$(SRCDIR)/diag_fpga_fw.h \
	$(SRCDIR)/diag_temperature_sensor_test.h \
	$(SRCDIR)/diag_ge_phy_88E1340_lib.h \
        $(SRCDIR)/diag_ge_phy_88E1548L_lib.h \
	$(SRCDIR)/diag_ge_phy_88E1340_test.h \
	$(SRCDIR)/diag_ge_phy_88E1548L_test.h \
	$(SRCDIR)/diag_bootflash_test.h \
	$(SRCDIR)/diag_bootflash_lib.h \
	$(SRCDIR)/diag_xaui_88X3120_lib.h \
        $(SRCDIR)/diag_xaui_88X2222M_lib.h \
	$(SRCDIR)/diag_xaui_88X3120_test.h \
	$(SRCDIR)/diag_xaui_88X2222M_test.h \
	$(SRCDIR)/diag_common_drv.h \
	$(SRCDIR)/diag_common_lib.h \
	$(SRCDIR)/sys_regs.h \
	$(SRCDIR)/mon_plat_defs.h \
	$(SRCDIR)/nvmonvars.h \
	$(SRCDIR)/pcmap.h \
	$(SRCDIR)/mb_tests.h \
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
	$(SRCDIR)/platform_smi.h \
	$(SRCDIR)/platform_smi_lib.h \
	$(SRCDIR)/platform_sfp_cookie.h \
	$(SRCDIR)/fdt.h \
	$(SRCDIR)/libfdt.h \
	$(SRCDIR)/libfdt_env.h \
	$(SRCDIR)/libfdt_internal.h \
	$(SRCDIR)/asm.h \
	$(SRCDIR)/sgidefs.h \
        $(SRCDIR)/diag_pll_test.h \
        $(SRCDIR)/diag_pll_pll.h \
        $(SRCDIR)/diag_backplane_xaui.h \
        $(SRCDIR)/diag_tlk10232_test.h \
        $(SRCDIR)/diag_tlk10232_lib.h \
        $(SRCDIR)/diag_ge_phy_88E1112C_test.h \
        $(SRCDIR)/diag_ge_phy_88E1112C_lib.h \
UTOOLS= eprom_elf squish_elf mkffsi
HTOOLS= hxld
TOOLS= $(patsubst %,$(UTIL_DIR)/%,$(UTOOLS)) \
	$(patsubst %,$(UTIL_DIR)/hxldsrc/%,$(HTOOLS))

DEV_BASE = $(DEV_DRIVER)/dev_object.c \
		   $(GE_QUAD_PHY_SRC)/dev_phy_88e1340.c

DEV_OBJS = dev_object.o \
			dev_phy_88e1340.o

# For diaglinux.
# RAM Based Object files : linux_main.o must be first.
# This wiill ensure that _start is first code in .text section.
#
NET_OBJS = \
	$(DEV_OBJS) \
	diag_fpga_lib.o \
	diag_fpga_fw.o \
	diag_xaui_88X2222M_lib.o \
	diag_xaui_88X3120_lib.o \
	diag_ge_phy_88E1548L_lib.o \
	diag_ge_phy_88E1340_lib.o \
	diag_fpga_test.o \
	diag_bootflash_lib.o \
	diag_temperature_sensor_lib.o \
	diag_temperature_sensor_test.o \
	diag_bootflash_test.o \
	diag_ge_phy_88E1340_test.o \
	diag_ge_phy_88E1548L_test.o \
	diag_xaui_88X3120_test.o \
	diag_xaui_88X2222M_test.o \
	diag_common_drv.o \
	diag_common_lib.o \
	main.o \
	diag_phy_3120_fw.o \
	linux_eth.o \
	linux_mem.o \
	linux_i2c.o \
	memops.o \
	menu.o \
	platform_smi.o \
	platform_eth.o \
	platform_ext_lpbk.o \
	platform_sfp_ext_lpbk.o \
	platform_sgmii.o \
	platform_xaui.o \
	platform_sfp_cookie.o \
	platform_i2c.o \
	query.o \
	testmem.o \
	woodlawn_common_utils.o \
	linux_api.o \
	bit_swaps.o \
	print_hex.o \
	woodlawn_linux_error.o \
	dev_tmp421.o \
        dev_mrvl_ge.o \
	diag.o \
	cavium_register_access.o \
	fdt_ro.o \
	fdt.o \
	stub.o \
        diag_pll_test.o \
        diag_pll_lib.o \
        diag_backplane_xaui_test.o \
        diag_tlk10232_test.o \
        diag_tlk10232_lib.o \
        diag_ge_phy_88E1112C_test.o \
        diag_ge_phy_88E1112C_lib.o \

vpath %.c \
	$(PROCSRCDIR): \
	$(COMSRCDIR): \
	$(WOODLAWNDIR): \
	$(SRCDIR): \
	$(KERNDIR): \
	$(DEV_DRIVER): \
	$(DEV_DRIVER)/dev_tmp421 \
	$(DEV_DRIVER)/dev_88e111x_marvell \
	$(FPGA_PATH): \
	$(GE_QUAD_PHY_SRC):

vpath %.h $(PROCINCDIR): \
	  $(CVMX_SDK_INC_PATH): \
	  $(WOODLAWNDIR): \
	  $(SRCDIR): \
	  $(INCDIR): \
	  $(KERNDIR): \
	  $(GS_4WIRE): \
	  $(GS_NEW): \
	  $(DEV_DRIVER): \
	  $(DIMM_SRC): \
	  $(EEPROM_SRC): \
	  $(FPGA_PATH):

vpath %.S $(PROCSRCDIR):$(SRCDIR):$(COMSRCDIR):$(SRCDIR)

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


ARCHIVE_FILES = $(NETTARGET) $(EXTRA_CVMX_APPLICATION_TARGETS)

archive: $(ARCHIVE_FILES)
	-@ if [ "${CVMX_ARCHIVE_DIRECTORY}" != "" ] ; then \
		mkdir -p ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
		cp -f $(ARCHIVE_FILES) ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL}/ ; \
		chmod -R a+r ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
	 fi ; \

application-target:$(NETTARGET) $(EXTRA_CVMX_APPLICATION_TARGETS) archive

$(NETTARGET): $(NET_OBJS)
	sh $(IROOT)/utils/banner.sh WOODLAWN
	$(CC) $(CFLAGS) -c banner.c
	$(CC) banner.o $(NET_OBJS) $(LDFLAGS_PATH) $(CVMX_LIB) $(LDFLAGS_GLOBAL) -o $@
	$(OBJDUMP) -t $(NETTARGET) >  $(NETTARGET).symbols

all: $(NETTARGET)

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
	@$(DEPCOMPILE) -M $(DEV_BASE) $(CBASE) $(ASM) > dependancies

dependancies:
	rm -f dependancies
	touch dependancies

include dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.3  2013/11/26 08:40:39  hroni
# fix compiler warning
#
# Revision 1.2  2013/10/08 08:48:26  tirawan
# Woodlawn collapsed to main trunk
#
# Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
# Branch into woodlawn-branch2 and port woodlawn code
#
# Revision 1.1.2.2  2013/06/13 11:42:44  tirawan
# Implement LED nc dispatch command for host side to be able to control SM LED
#
# Revision 1.1.2.1  2013/04/24 10:37:12  tirawan
# Initial check-in for woodlawn linux code
#
# Revision 1.5  2013/03/27 08:45:04  kuangik
# Code cleanup
#
# Revision 1.4  2013/03/27 07:28:59  kuangik
# Update Makefile to remove unnecessary file
#
# Revision 1.3  2013/03/27 04:49:34  kuangik
# Code cleanup after adding -Wall
#
# Revision 1.2  2013/03/13 07:26:24  kuangik
# Provide cross compiler directory
#
# Revision 1.1  2013/03/13 06:42:49  kuangik
# Add for the first time
#
# Revision 1.23  2013/03/08 09:34:07  kuangik
# Clear all warning
#
# Revision 1.22  2013/03/02 11:39:54  kuangik
# Add banner for version control
#
# Revision 1.21  2013/02/19 01:00:55  leslie
# Add 88e111x device driver
#
# Revision 1.20  2013/01/15 23:14:28  leslie
# Add PHY 88E1112C library.
#
# Revision 1.19  2013/01/13 22:15:58  leslie
# Add TLK10232 library.
#
# Revision 1.18  2012/12/11 02:31:34  leslie
# Add backplane xaui loopback test.
#
# Revision 1.17  2012/08/30 06:28:10  leslie
# Add PLL library.
#
# Revision 1.16  2012/07/23 07:37:55  leslie
# Add libfdt library.
#
# Revision 1.14  2012/05/18 10:07:35  kody
# Add diag common library
#
# Revision 1.13  2012/05/18 04:07:58  leslie
# Add bootflash lib
#
# Revision 1.12  2012/05/16 02:22:16  leslie
# Add platform SMI code
#
# Revision 1.11  2012/04/16 12:33:39  kuangik
# Add FPGA Firmware upgrade function
#
# Revision 1.10  2012/04/16 02:40:41  kody
# Add Marvell XAUI 88X2222M test and remove 88X2120L test.
#
# Revision 1.9  2012/04/06 06:16:37  kuangik
# Remove duplicated dev_object compilation
#
# Revision 1.8  2012/04/06 06:03:18  kuangik
# Add 88E1340 driver for compilation
#
# Revision 1.7  2012/03/26 07:17:00  kody
# Add TMP421 temperature sensor test code.
#
# Revision 1.6  2012/02/15 10:03:13  kuangik
# Remove dependancy files when doing make clean
#
# Revision 1.5  2012/02/13 09:34:01  kuangik
# Change the image name to woodlawnnet
#
# Revision 1.4  2012/02/13 09:28:30  kuangik
# Change OCTEON_ROOT to the /usr/local directory and CVMX_LIB to the current directory
#
# Revision 1.3  2012/02/10 09:02:31  leslie
# Add Woodlawn Makefile.
#
# Revision 1.1  2012/02/10 05:59:50  kody
# Initial revision
#
# $Endlog$
#########################

# $Id: Makefile,v 1.6 2020/07/14 06:45:56 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
#
# Copyright (c) 2018 by Cisco Systems, Inc.
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

default: all


TOOLS_VER	= c4.7.0-p3
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
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

VIPERDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
MRV_88E6176_SRC = $(DEV_DRV)/dev_88e6176_marvell
MRV_88E151X_SRC = $(DEV_DRV)/dev_88e151x_marvell
LTE_SWI_SRC = $(DEV_DRV)/dev_lte_swi
NXP_LM75B_SRC = $(DEV_DRV)/dev_nxp_lm75b

# TAM lib. request : ENGTAM-1223
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/ISR4K_2099_SUDI

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(VIPERDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(MRV_88E6176_SRC) \
	-I$(MRV_88E151X_SRC) \
	-I$(LTE_SWI_SRC) 
	

#
# defines for compiler
#

DEFS = \
	-DVIPER \
	-DLINUX_APP \
    -DTFTP_CLIENT  \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS}
LDFLAGS         = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into VIPER project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= viperlnx

#
# Common device driver objects
#
DEV_OBJS = \
		   dev_object.o \
		   dev_nxp_lm75b.o \
		   dev_88e6176.o \
		   dev_88e151x.o \
		   dev_lte_swi.o \
		   dev_lte_swi_at.o

DEV_BASE = \
		   $(DEV_DRV)/dev_object.c \
		   $(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		   $(MRV_88E6176_SRC)/dev_88e6176.c \
		   $(DEV_DRV)/dev_object.c \
		   $(MRV_88E151X_SRC)/dev_88e151x.c \
		   $(LTE_SWI_SRC)/dev_lte_swi.c \
		   $(LTE_SWI_SRC)/dev_lte_swi_at.c


DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(MRV_88E6176_SRC)/dev_88e6176.h \
	$(MRV_88E151X_SRC)/dev_88e151x.h \
	$(LTE_SWI_SRC)/dev_lte_swi.h \
	$(LTE_SWI_SRC)/dev_lte_swi_at.h

#
# Common object files
#


COM_OBJS = \
    linux_main.o \
	linux_i2c.o \
    linux_error.o \
    linux_stub.o \
    linux_mem.o \
    linux_api.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	linux_ntwk.o \
	rtc.o

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/menu.c \
    $(COMSRCDIR)/common_utils.c \
    $(COMSRCDIR)/print_hex.c \
    $(COMSRCDIR)/query.c \
    $(COMSRCDIR)/linux_pcie.c \
    $(COMSRCDIR)/memops.c \
    $(COMSRCDIR)/testmem.c \
    $(COMSRCDIR)/bit_swaps.c \
    $(COMSRCDIR)/tftpclient.c \
    $(COMSRCDIR)/cookie_4_core.c \
    $(COMSRCDIR)/tam_act2_utils.c \
    $(COMSRCDIR)/linux_ntwk.c \
    $(COMSRCDIR)/linux_popen.c \
    $(COMSRCDIR)/rtc.c


COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
    $(INCDIR)/common_utils.h \
    $(INCDIR)/linux_usb_test.h \
    $(INCDIR)/i2c_api.h \
    $(INCDIR)/i2c_dev.h \
    $(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/linux_ntwk.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h \

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files :
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	mb_tests.o \
	linux_popen.o \
	diag_boot_flash_test.o \
	diag_spi_flash_util.o \
	diag_fpga_test.o \
	diag_gephy_test.o \
	diag_gephy_lib.o \
	diag_gephy_util.o \
	diag_esw_test.o \
	diag_esw_util.o \
	diag_esw_lib.o \
	diag_xdsl_test.o \
	diag_dsl_libs.o \
	diag_dsl_util.o \
	diag_fpga_lib.o \
	diag_led_test.o \
	diag_led_util.o \
        diag_eth_pkt_txrx.o \
        diag_eth_pkt_txrx_api.o \
        diag_eth_pkt_txrx_utils.o \
	diag_lte_test.o \
	diag_lte_lib.o \
	diag_ddr4_lib.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	i2c_api.o \
	diag_fpga_i2c_lib.o \
	diag_storage_lib.o \
	diag_usb_lib.o \
	diag_usb_util.o \
	diag_usb_test.o \
	diag_temp_snsr_lib.o \
	diag_temp_snsr_test.o \
	diag_cpu_util.o \
	platform_stub.o \
	platform_cookie.o \
	tam_act2_api_drv_support.o \
	diag_emmc_test.o \
	diag_emmc_util.o \
	dnv_gpio_lib.o \
	diag_uart_lib.o \
	diag_nc_lib.o \
	dnv_eth_lib.o \
	diag_fpga_upgrade.o \
	diag_rtc_lib.o \
	diag_rtc_test.o \
	diag_rtc_util.o \
	platform_cpu.o \
	diag_reset_button.o \
	viper_comm.o \
	full_load_util.o \
	diag_lte_util.o 

PLAT_BASE = \
	$(VIPERDIR)/linux_main.c \
	$(VIPERDIR)/diag.c \
	$(VIPERDIR)/mb_tests.c \
	$(VIPERDIR)/diag_boot_flash_test.c \
	$(VIPERDIR)/diag_spi_flash_util.c \
	$(VIPERDIR)/diag_fpga_test.c \
	$(VIPERDIR)/diag_gephy_test.c \
	$(VIPERDIR)/diag_gephy_lib.c \
	$(VIPERDIR)/diag_gephy_util.c \
	$(VIPERDIR)/diag_esw_test.c \
	$(VIPERDIR)/diag_esw_util.c \
	$(VIPERDIR)/diag_esw_lib.c \
	$(VIPERDIR)/diag_xdsl_test.c \
	$(VIPERDIR)/diag_dsl_libs.c \
	$(VIPERDIR)/diag_dsl_util.c \
	$(VIPERDIR)/diag_fpga_lib.c \
	$(VIPERDIR)/diag_led_test.c \
	$(VIPERDIR)/diag_led_util.c \
	$(VIPERDIR)/diag_eth_pkt_txrx.h \
	$(VIPERDIR)/diag_eth_pkt_txrx_api.h \
	$(VIPERDIR)/diag_eth_pkt_txrx_utils.h \
	$(VIPERDIR)/diag_lte_test.c \
	$(VIPERDIR)/diag_lte_lib.c \
	$(VIPERDIR)/diag_ddr4_lib.c \
	$(VIPERDIR)/diag_i2c_lib.c \
	$(VIPERDIR)/diag_i2c_test.c \
	$(VIPERDIR)/i2c_api.c \
	$(VIPERDIR)/diag_fpga_i2c_lib.c \
	$(VIPERDIR)/diag_fpga_upgrade.c \
	$(VIPERDIR)/diag_storage_lib.c \
	$(VIPERDIR)/diag_usb_lib.c \
	$(VIPERDIR)/diag_usb_util.c \
	$(VIPERDIR)/diag_usb_test.c \
	$(VIPERDIR)/diag_temp_snsr_lib.c \
	$(VIPERDIR)/diag_temp_snsr_test.c \
	$(VIPERDIR)/diag_cpu_util.c \
	$(VIPERDIR)/platform_stub.c \
	$(VIPERDIR)/platform_cookie.c \
	$(VIPERDIR)/tam_act2_api_drv_support.c \
	$(VIPERDIR)/diag_emmc_test.c \
	$(VIPERDIR)/diag_emmc_util.c \
	$(VIPERDIR)/dnv_gpio_lib.c \
	$(VIPERDIR)/diag_uart_lib.c \
	$(VIPERDIR)/diag_nc_lib.c \
	$(VIPERDIR)/dnv_eth_lib.c \
	$(VIPERDIR)/diag_rtc_lib.c \
	$(VIPERDIR)/diag_rtc_test.c \
	$(VIPERDIR)/diag_rtc_util.c \
	$(VIPERDIR)/platform_cpu.c \
	$(VIPERDIR)/diag_reset_button.c \
	$(VIPERDIR)/viper_comm.c \
	$(VIPERDIR)/full_load_util.c \
	$(VIPERDIR)/diag_lte_util.c


PLAT_HFILES = \
    $(VIPERDIR)/diag_common.h \
    $(VIPERDIR)/platform_i2c.h \
    $(VIPERDIR)/mb_tests.h \
    $(VIPERDIR)/diag_xdsl_test.h \
    $(VIPERDIR)/diag_dsl_libs.h \
    $(VIPERDIR)/diag_dsl_util.h \
    $(VIPERDIR)/diag_eth_info.h \
    $(VIPERDIR)/diag_eth_pkt_txrx.h \
    $(VIPERDIR)/diag_eth_pkt_txrx_api.h \
    $(VIPERDIR)/diag_eth_pkt_txrx_utils.h \
    $(VIPERDIR)/diag_fpga.h \
    $(VIPERDIR)/diag_ddr4_lib.h \
    $(VIPERDIR)/diag_i2c_lib.h \
    $(VIPERDIR)/diag_i2c_test.h \
    $(VIPERDIR)/diag_i2c_addr.h \
    $(VIPERDIR)/diag_boot_flash_test.h \
    $(VIPERDIR)/diag_spi_flash_util.h \
    $(VIPERDIR)/diag_fpga_i2c_lib.h \
    $(VIPERDIR)/diag_storage_lib.h \
    $(VIPERDIR)/diag_usb_lib.h \
    $(VIPERDIR)/diag_usb_util.h \
    $(VIPERDIR)/diag_usb_test.h \
    $(VIPERDIR)/diag_temp_snsr_lib.h \
    $(VIPERDIR)/diag_temp_snsr_test.h \
    $(VIPERDIR)/diag_esw_test.h \
    $(VIPERDIR)/diag_esw_util.h \
    $(VIPERDIR)/diag_esw_lib.h \
    $(VIPERDIR)/diag_gephy_test.h \
    $(VIPERDIR)/diag_gephy_lib.h \
    $(VIPERDIR)/diag_gephy_util.h \
    $(VIPERDIR)/diag_cpu_util.h \
    $(VIPERDIR)/platform_stub.h \
    $(VIPERDIR)/platform_cookie.h \
    $(VIPERDIR)/tam_act2_api_drv_support.h \
    $(VIPERDIR)/diag_emmc_test.h \
    $(VIPERDIR)/diag_emmc_util.h \
    $(VIPERDIR)/dnv_gpio_lib.h \
    $(VIPERDIR)/diag_uart_lib.h \
    $(VIPERDIR)/diag_nc_lib.h \
    $(VIPERDIR)/dnv_eth_lib.h \
    $(VIPERDIR)/diag_fpga_upgrade.h \
    $(VIPERDIR)/diag_rtc_lib.h \
    $(VIPERDIR)/diag_rtc_test.h \
    $(VIPERDIR)/diag_rtc_util.h \
    $(VIPERDIR)/platform_cpu.h \
    $(VIPERDIR)/diag_reset_button.h \
    $(VIPERDIR)/viper_comm.h \
    $(VIPERDIR)/full_load_util.h \
    $(VIPERDIR)/diag_lte_util.h

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2)

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


vpath %.c \
	$(GE_PHY_SRC): \
	$(VIPERDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(MRV_88E6176_SRC): \
	$(MRV_88E151X_SRC): \
	$(LTE_SWI_SRC): \
	$(TAMLIB)

vpath %.h \
	$(GE_PHY_SRC): \
	$(VIPERDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
    $(RTC_SRC): \
	$(NXP_LM75B_SRC): \
    $(MRV_88E6176_SRC): \
    $(MRV_88E151X_SRC): \
    $(LTE_SWI_SRC): \
    $(TAMLIB)

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

all: $(NETTARGET)

clean:
	rm -rf *.o banner.c \
	.compile ID.temp


TARGET_DEPEND = \
	Makefile \
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)
	
ENV_PRT :
	@echo "##########################################################"
	@echo "#################### Env Variables #######################"
	@echo "##########################################################"
	@echo "PATH: " $(PATH)
	@echo "VIPER_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "VIPERDIR: " $(VIPERDIR)
	@echo "KLMDIR: " $(KLMDIR)
	@echo "DEV_DRV: " $(DEV_DRV)
	@echo "INCDIR: " $(INCDIR)
	@echo "COMSRCDIR: " $(COMSRCDIR)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "##########################################################"

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : ENV_PRT $(TARGET_DEPEND) $(HFILES)
	sh $(IROOT)/utils/banner.sh VIPER_INTEL
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS}

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols
	
#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) > dependancies
	@echo "Computing TAGS file for common/src"
	#/router/bin/etags -t $(DEPEND_LST)
	$(shell which ctags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	 -lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)
	
scope:
	cscope -b $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.6  2020/07/14 06:45:56  harrchan
# Fixed bug of RTC set/display utility(CSCvu92708)
#
# Revision 1.5  2018/09/04 01:10:18  harrchan
# Set up the flag for SUDI_2099 in Makefile (CSCvm32284)
#
# Revision 1.4  2018/08/31 03:59:30  chieyang
# Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
#
# Revision 1.3  2018/08/14 22:47:13  iachang
# CSCvk51378: Supportted Act2 SUDI 2099, please refer to Viper Makefile.
#
# Revision 1.2  2018/08/06 02:31:49  harrchan
# Merge viper E2E to the main trunk (CSCvk28469)
#
# Revision 1.1.2.15  2018/07/09 08:27:33  olin2
# CSCvk17781: Support util to verify SIM Detect pin
#
# Revision 1.1.2.14  2018/06/27 06:27:48  harrchan
# Follow the coding rule to clean up code
#
# Revision 1.1.2.13  2018/06/25 10:07:29  lucywang
# Modified messages for full load utility
#
# Revision 1.1.2.12  2018/06/25 07:02:35  olin2
# Remove Viper-Intel P0 DSL support
#
# Revision 1.1.2.11  2018/06/06 11:38:06  lucywang
# Modified the process to enable pSLC and create 8MB GPP on eMMC based on Cisco SW requirement
#
# Revision 1.1.2.10  2018/05/21 08:42:35  olin2
# Support DSL LED on/off utility
#
# Revision 1.1.2.9  2018/05/10 09:04:37  lucywang
# Added USB 2.0 test mode utility
#
# Revision 1.1.2.8  2018/05/09 03:17:07  harrchan
# Modify Makefile ctags path
#
# Revision 1.1.2.7  2018/04/26 04:39:39  harrchan
# Fix the path of DEV_BASE MRV_88E6176_SRC
#
# Revision 1.1.2.6  2018/03/29 10:35:44  lucywang
# Added Reset button test
#
# Revision 1.1.2.5  2018/03/29 01:11:19  lucywang
# Added RTC test and utility
#
# Revision 1.1.2.4  2018/03/28 09:18:13  lucywang
# Added CPU test
#
# Revision 1.1.2.3  2018/03/28 07:55:52  lucywang
# Changed Thermal sersor to LM75B, TBD : bug fix
#
# Revision 1.1.2.2  2018/03/26 09:21:03  harrchan
# Add led utility
#
#
# $Endlog$




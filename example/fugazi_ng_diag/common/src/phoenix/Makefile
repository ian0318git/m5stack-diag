# $Id: Makefile,v 1.2 2021/04/15 00:52:22 achiu2 Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
#
# Copyright (c) 2019 by Cisco Systems, Inc.
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

PATH := $(PATH):/router/bin

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

PHOENIXDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
ENV_SRC     = $(DEV_DRV)/dev_csco_10698
MRV_88E151X_SRC = $(DEV_DRV)/dev_88e151x_marvell
NXP_LM75B_SRC   = $(DEV_DRV)/dev_nxp_lm75b
PCA9557_SRC     = $(DEV_DRV)/dev_pca9557
PCA9555_SRC     = $(DEV_DRV)/dev_pca9555
TMPX75_SRC      = $(DEV_DRV)/dev_tmpx75
TELIT_LTE_SRC   = $(DEV_DRV)/dev_lte_telit
RBCP_LIB_PATH = $(COMSRCDIR)/rbcp_lib
SHEDIR_SRC_PATH = $(COMSRCDIR)/shedir
AT24C0N_SRC = $(DEV_DRV)/dev_at24c0n_atmel


# Broadcom PHY EPDM lib
BCM_PHY_PATH := /auto/neptune/vendor-src/broadcom/epdm
BCM_MIURA_ROOT := $(BCM_PHY_PATH)/MIUR_1_8
BCM_MIURA_LIB := $(BCM_MIURA_ROOT)/build/libphymodepil-x86_64.a
BCM_QUADRA28_ROOT := $(BCM_PHY_PATH)/QUADRA28_1_2
BCM_QUADRA28_LIB := $(BCM_QUADRA28_ROOT)/build/libphymodepil-x86_64.a
BCM_EPDM_ROOT := $(BCM_PHY_PATH)/epdm_1_7_4
BCM_EPDM_LIB := $(BCM_EPDM_ROOT)/libepdm-x86_64.a
BCM_EPDM_INC := $(BCM_EPDM_ROOT)
BCM_PHY_LIBS := $(BCM_EPDM_LIB) $(BCM_QUADRA28_LIB) $(BCM_MIURA_LIB)


# TAM lib. request : ENGTAM-1223
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido_v3_4_24/

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(PHOENIXDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(PCA9557_SRC) \
	-I$(PCA9555_SRC) \
	-I$(TMPX75_SRC) \
	-I$(MRV_88E151X_SRC) \
	-I$(TELIT_LTE_SRC) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(ENV_SRC) \
	-I$(RBCP_LIB_PATH) \
	-I$(SHEDIR_SRC_PATH) \
	-I$(AT24C0N_SRC)

INC     += -I$(BCM_EPDM_INC)

#
# defines for compiler
#

DEFS = \
	-DPHOENIX \
	-DLINUX_APP \
	-DTFTP_CLIENT  \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DAIKIDO_ACT2 \
	-DAIKIDO_SUPPORT_AIK \
	-DAIKIDO_DEV_KEY \
	-DAIKIDO_CT_DEV_KEY \

# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS     = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS}
LDFLAGS     = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS      += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into PHOENIX project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJNAME = $(TAMLIB)/tam_lib_objname.a


LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= phoenixlnx

#
# Common device driver objects
#
DEV_OBJS = \
		dev_object.o \
		dev_nxp_lm75b.o \
		dev_ltc4215.o \
		dev_pca9557.o \
		dev_pca9555.o \
		dev_tmpx75.o \
		dev_88e151x.o \
		dev_csco_10698.o \
        dev_lte_telit.o \
    	dev_lte_telit_at.o \


DEV_BASE = \
		$(DEV_DRV)/dev_object.c \
		$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
		$(MRV_88E151X_SRC)/dev_88e151x.c \
		$(PCA9557_SRC)/dev_pca9557.c \
		$(PCA9555_SRC)/dev_pca9555.c \
		$(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		$(TMPX75_SRC)/dev_tmpx75.c \
		$(ENV_SRC)/dev_csco_10698.c \
	    $(TELIT_LTE_SRC)/dev_lte_telit.c \
	    $(TELIT_LTE_SRC)/dev_lte_telit_at.c\


DEV_HFILES = \
$(DEV_DRV)/dev_object.h \
        $(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
    	$(PCA9557_SRC)/dev_pca9557.h \
    	$(PCA9555_SRC)/dev_pca9555.h \
    	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
    	$(TMPX75_SRC)/dev_tmpx75.h \
    	$(MRV_88E151X_SRC)/dev_88e151x.h \
    	$(ENV_SRC)/dev_csco_10698.h \
    	$(TELIT_LTE_SRC)/dev_lte_telit.h \
    	$(TELIT_LTE_SRC)/dev_lte_telit_at.h\


FORTITUDE_OBJS = ngwic_fortitude.o
FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c


#
## Dynamo NIM
#
DYNAMO_OBJS = nim_dynamo.o
DYNAMO_BASE = $(COMSRCDIR)/nim_dynamo.c


SHEDIR_OBJS = \
        ngwic_shedir.o \
        shedir_rbcp_main.o \
        rbcp_platform.o \
        rbcp_lib.o

SHEDIR_BASE = \
        $(SHEDIR_SRC_PATH)/ngwic_shedir.c \
        $(SHEDIR_SRC_PATH)/shedir_rbcp_main.c \
        $(SHEDIR_SRC_PATH)/rbcp_platform.c \
        $(RBCP_LIB_PATH)/rbcp_lib.c

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
    linux_pci.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    linux_uart.o \
    linux_pciutils.o \
    linux_usb_test.o \
    linux_block_test.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    adapter_fpga.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	linux_ntwk.o \
	pca.o \
	linux_coretest.o \
	oir_ltc4215_api.o \
	rtc.o

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
    $(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_pci.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/linux_uart.c \
    $(COMSRCDIR)/linux_pciutils.c \
    $(COMSRCDIR)/linux_usb_test.c \
    $(COMSRCDIR)/linux_block_test.c \
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
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/slot.c \
    $(COMSRCDIR)/adapter_fpga.c \
    $(COMSRCDIR)/tam_aikido_upgrade.c \
    $(COMSRCDIR)/rtc.c

COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_pciutils.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/linux_block_test.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/adapter_fpga.h \
	$(INCDIR)/linux_ntwk.h \
	$(INCDIR)/linux_pci.h \
	$(INCDIR)/list.h \
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
	diag_fpga_lib.o \
	diag_cpld_lib.o \
	diag_led_test.o \
	diag_led_util.o \
    diag_eth_pkt_txrx.o \
    diag_eth_pkt_txrx_api.o \
    diag_eth_pkt_txrx_utils.o \
	diag_ddr4_lib.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	i2c_api.o \
	diag_fpga_i2c_lib.o \
	diag_usb_lib.o \
	diag_usb_util.o \
	diag_usb_test.o \
	diag_temp_snsr_test.o \
	diag_cpu_util.o \
	platform_stub.o \
	platform_cookie.o \
	platform_fan.o \
	legacy_smart_cookie.o \
	tam_act2_api_drv_support.o \
	tam_aikido_upgrade.o \
	diag_emmc_test.o \
	diag_emmc_util.o \
	dnv_gpio_lib.o \
	diag_uart_lib.o \
	diag_fpga_upgrade.o \
	diag_rtc_lib.o \
	diag_rtc_test.o \
	diag_rtc_util.o \
	platform_cpu.o \
	phoenix_comm.o \
	full_load_util.o \
	diag_i350_test.o \
	ngio.o \
	slot.o \
	platform_slot.o \
	platform_barometer.o \
	$(PLUG_OBJS) \
	diag_hdd_test.o \
	diag_m2_test.o \
	platform_pwr_seq.o \
	platform_spi.o \
	fpga_smartfan.o \
    phoenix_virtual_sm.o \
    platform_psu.o \
	diag_pcie_test.o \
	nios_mbox.o



PLAT_BASE = \
	$(PHOENIXDIR)/linux_main.c \
	$(PHOENIXDIR)/diag.c \
	$(PHOENIXDIR)/mb_tests.c \
	$(PHOENIXDIR)/diag_boot_flash_test.c \
	$(PHOENIXDIR)/diag_spi_flash_util.c \
	$(PHOENIXDIR)/diag_fpga_test.c \
	$(PHOENIXDIR)/diag_fpga_lib.c \
	$(PHOENIXDIR)/diag_cpld_lib.c \
	$(PHOENIXDIR)/diag_led_test.c \
	$(PHOENIXDIR)/diag_led_util.c \
	$(PHOENIXDIR)/diag_ddr4_lib.c \
	$(PHOENIXDIR)/diag_i2c_lib.c \
	$(PHOENIXDIR)/diag_i2c_test.c \
	$(PHOENIXDIR)/i2c_api.c \
	$(PHOENIXDIR)/diag_fpga_i2c_lib.c \
	$(PHOENIXDIR)/diag_fpga_upgrade.c \
	$(PHOENIXDIR)/diag_usb_lib.c \
	$(PHOENIXDIR)/diag_usb_util.c \
	$(PHOENIXDIR)/diag_usb_test.c \
	$(PHOENIXDIR)/diag_temp_snsr_test.c \
	$(PHOENIXDIR)/diag_cpu_util.c \
	$(PHOENIXDIR)/platform_stub.c \
	$(PHOENIXDIR)/platform_cookie.c \
	$(PHOENIXDIR)/platform_fan.c \
	$(PHOENIXDIR)/legacy_smart_cookie.c \
	$(PHOENIXDIR)/tam_act2_api_drv_support.c \
	$(PHOENIXDIR)/diag_emmc_test.c \
	$(PHOENIXDIR)/diag_emmc_util.c \
	$(PHOENIXDIR)/dnv_gpio_lib.c \
	$(PHOENIXDIR)/diag_uart_lib.c \
	$(PHOENIXDIR)/diag_rtc_lib.c \
	$(PHOENIXDIR)/diag_rtc_test.c \
	$(PHOENIXDIR)/diag_rtc_util.c \
	$(PHOENIXDIR)/platform_cpu.c \
	$(PHOENIXDIR)/phoenix_comm.c \
	$(PHOENIXDIR)/full_load_util.c \
	$(PHOENIXDIR)/diag_i350_test.c \
	$(PHOENIXDIR)/ngio.c \
	$(PHOENIXDIR)/platform_slot.c \
	$(PHOENIXDIR)/platform_barometer.c \
	$(PLUG_BASE) \
	$(PHOENIXDIR)/diag_hdd_test.c \
	$(PHOENIXDIR)/diag_m2_test.c \
	$(PHOENIXDIR)/platform_pwr_seq.c \
	$(PHOENIXDIR)/platform_spi.c \
	$(PHOENIXDIR)/fpga_smartfan.c \
    $(PHOENIXDIR)/phoenix_virtual_sm.c \
    $(PHOENIXDIR)/platform_psu.c \
    $(PHOENIXDIR)/diag_pcie_test.c \
    $(PHOENIXDIR)/nios_mbox.c



PLAT_HFILES = \
    $(PHOENIXDIR)/diag_common.h \
    $(PHOENIXDIR)/platform_i2c.h \
    $(PHOENIXDIR)/mb_tests.h \
    $(PHOENIXDIR)/diag_eth_info.h \
    $(PHOENIXDIR)/diag_eth_pkt_txrx.h \
    $(PHOENIXDIR)/diag_eth_pkt_txrx_api.h \
    $(PHOENIXDIR)/diag_eth_pkt_txrx_utils.h \
    $(PHOENIXDIR)/diag_fpga.h \
    $(PHOENIXDIR)/diag_ddr4_lib.h \
    $(PHOENIXDIR)/diag_i2c_lib.h \
    $(PHOENIXDIR)/diag_i2c_test.h \
    $(PHOENIXDIR)/diag_i2c_addr.h \
    $(PHOENIXDIR)/diag_boot_flash_test.h \
    $(PHOENIXDIR)/diag_spi_flash_util.h \
    $(PHOENIXDIR)/diag_fpga_i2c_lib.h \
    $(PHOENIXDIR)/diag_storage_lib.h \
    $(PHOENIXDIR)/diag_usb_lib.h \
    $(PHOENIXDIR)/diag_usb_util.h \
    $(PHOENIXDIR)/diag_usb_test.h \
    $(PHOENIXDIR)/diag_temp_snsr_test.h \
    $(PHOENIXDIR)/diag_gephy_test.h \
    $(PHOENIXDIR)/diag_cpu_util.h \
    $(PHOENIXDIR)/platform_stub.h \
    $(PHOENIXDIR)/platform_cookie.h \
    $(PHOENIXDIR)/legacy_smart_cookie.h \
    $(PHOENIXDIR)/tam_act2_api_drv_support.h \
	$(PHOENIXDIR)/diag_emmc_test.h \
	$(PHOENIXDIR)/diag_emmc_util.h \
    $(PHOENIXDIR)/dnv_gpio_lib.h \
    $(PHOENIXDIR)/diag_uart_lib.h \
    $(PHOENIXDIR)/dnv_eth_lib.h \
    $(PHOENIXDIR)/diag_fpga_upgrade.h \
    $(PHOENIXDIR)/diag_rtc_lib.h \
    $(PHOENIXDIR)/diag_rtc_test.h \
    $(PHOENIXDIR)/diag_rtc_util.h \
    $(PHOENIXDIR)/platform_cpu.h \
    $(PHOENIXDIR)/phoenix_comm.h \
    $(PHOENIXDIR)/full_load_util.h \
	$(PHOENIXDIR)/diag_i350_test.h \
	$(PHOENIXDIR)/ngio.h \
	$(PHOENIXDIR)/platform_slot.h \
	$(PHOENIXDIR)/platform_barometer.h \
	$(PHOENIXDIR)/diag_hdd_test.h \
	$(PHOENIXDIR)/diag_m2_test.h \
	$(PHOENIXDIR)/platform_pwr_seq.h \
	$(PHOENIXDIR)/fpga_smartfan.h \
	$(PHOENIXDIR)/platform_margin_utils.h \
	$(PHOENIXDIR)/bcm_gesw_defs.h \
	$(PHOENIXDIR)/phoenix_virtual_sm.h \
	$(PHOENIXDIR)/platform_psu.h \
	$(PHOENIXDIR)/diag_pcie_test.h \
	$(PHOENIXDIR)/nios_mbox_api.h



#
# Test Card related files
#
TESTCARD_OBJS = \
        ngio_testcard.o \
        testcard_fpga.o \
        testcard_xaui.o \
        testcard_plx_pcie_sw.o \
        testcard_tlk_10232.o \
        testcard_uart.o

TESTCARD_BASE = \
        $(PHOENIXDIR)/ngio_testcard.c \
        $(PHOENIXDIR)/testcard_fpga.c \
        $(PHOENIXDIR)/testcard_xaui.c \
        $(PHOENIXDIR)/testcard_plx_pcie_sw.c \
        $(PHOENIXDIR)/testcard_tlk_10232.c \
        $(PHOENIXDIR)/testcard_uart.c

TESTCARD_HFILES = \
        $(PHOENIXDIR)/ngio_testcard.h \
        $(PHOENIXDIR)/testcard_fpga.h \
        $(PHOENIXDIR)/testcard_plx_pcie_sw.h \
        $(PHOENIXDIR)/testcard_tlk_10232.h \
        $(PHOENIXDIR)/testcard_xaui.h 



#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_PCI_UTILS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_TAM_OBJNAME) \
	$(FORTITUDE_OBJS) \
    $(TESTCARD_OBJS) \
	$(DYNAMO_OBJS) \
	$(SHEDIR_OBJS)

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE) \
    $(FORTITUDE_BASE) \
    $(TESTCARD_BASE) \
	$(DYNAMO_BASE) \
	$(SHEDIR_BASE)


#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
    $(TESTCARD_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(PHOENIXDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(ENV_SRC): \
	$(RTC_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(NXP_LM75B_SRC): \
	$(MRV_88E151X_SRC): \
	$(LTE_SWI_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(TAMLIB): \
	$(PLUG_LTE_TELIT_SRC): \
	$(TELIT_LTE_SRC): \
	$(RBCP_LIB_PATH): \
	$(SHEDIR_SRC_PATH): \

vpath %.h \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(PHOENIXDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(ENV_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(MRV_88E151X_SRC): \
	$(LTE_SWI_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(TAMLIB): \
	$(PLUG_LTE_TELIT_SRC): \
	$(TELIT_LTE_SRC): \
	$(RBCP_LIB_PATH): \
	$(SHEDIR_SRC_PATH): \

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
	@echo "PHOENIX_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "PHOENIXDIR: " $(PHOENIXDIR)
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
	sh $(IROOT)/utils/banner.sh PHOENIX_INTEL
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS} \
        -Wl,-whole-archive $(BCM_PHY_LIBS) -Wl,-no-whole-archive 

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


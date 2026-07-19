# $Id: Makefile,v 1.3 2018/08/06 02:37:59 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build CSX-Tachi Intel app 
#
# June 2015, Alan Peng
#
# Copyright (c) 2015-2018 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#
# Note: compiled on sjc-acme-v07 server. 
#

MAIN_APP = tachi_nimtest

#
# Tool Chain
#
TOOLS_VER   = c4.7.0-p1
TOOLS_ARCH      = x86_64
OBJDUMP         = objdump.$(TOOLS_VER).$(TOOLS_ARCH)


CC = gcc
DEFS = -DLINUX_APP -DTACHI_INTEL
DREAMLINER_DEFS = \
        -DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS
NET_DEFS = $(DEFS) $(DREAMLINER_DEFS)

#CFLAGS = -Werror -Wall -g $(DEFS)
CFLAGS = -Werror -Wall -g $(NET_DEFS) $(INC) 
LDFLAGS = $(LD) -lpci_gcc4.1.2-48 -lpthread -lrt
SRCDIR = .
BMCDIR = ..
IROOT           = ../../../..
INCDIR          = $(IROOT)/common/include
COMSRCDIR       = $(IROOT)/common/src
DEV_DRV         = $(IROOT)/common/chips
CHIPINC         = $(DEV_DRV)/include
KLMDIR          = $(IROOT)/klm
DREAMLINER_SRC_PATH  = $(COMSRCDIR)/dreamliner
DREAMLINER_PHY_PATH  = $(COMSRCDIR)/dreamliner/phy
DREAMLINER_CPSS_PATH = $(COMSRCDIR)/dreamliner/cpss
DREAMLINER_PLD_PATH = /auto/sp-engops/diags/pld/dreamliner
LIB_PCI_UTILS = \
	/auto/sword/Utils/pciutils/3.1.10

AR = ar
ARFLAGS = rs
#r - replace lib to new one 
#v - verbose
#s - create a symbol table 

LD      = \
	-L$(LIB_PCI_UTILS)

INC     = \
	-I$(SRCDIR) \
	-I$(BMCDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(KLMDIR) \
	-I$(INCDIR) 

INC     += \
	-I$(DREAMLINER_SRC_PATH) \
	-I$(DREAMLINER_PHY_PATH) \
	-I$(DREAMLINER_CPSS_PATH)/include/ \
	-I$(DREAMLINER_CPSS_PATH)/include/cisco \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/driver/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDr \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDrv/drivers \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os/gtOs \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/trace \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/oam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager\
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/extMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/i2c/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/init \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/interrupts/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/mirror/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/configElementDb \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/siliconIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/cgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/geMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/hglMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/ilknMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/tcamMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xlgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/cgPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/gPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/hglPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/ilknPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/mmPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/tcamPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/xPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyH \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyHRev2 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/smi \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/hardware \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/dxExMx \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/generic \

DREAMLINER_LIB = \
	$(DREAMLINER_PLD_PATH)/libdreamlinersdk.a

COM_HFILES = \
	$(BMCDIR)/nvmonvars.h \
	$(INCDIR)/aglobal.h \
	$(INCDIR)/atypes.h \
	$(INCDIR)/bexterr.h \
	$(INCDIR)/bitdef.h \
	$(INCDIR)/bsafe.h \
	$(INCDIR)/bsfmacro.h \
	$(INCDIR)/bsfplatf.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/cli_cmd.h \
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
	$(INCDIR)/oir_ltc4215_api.h \
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
	$(INCDIR)/uio_utils.h 

#
#########   ASM Files #########
#

ASM     = \
        $(PLAT_AFILES)

vpath %.c \
        $(SRCDIR): \
	$(COMSRCDIR): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PLD_PATH): \
	$(DREAMLINER_PHY_PATH): \
	$(DREAMLINER_CPSS_PATH)/src: \

vpath %.h \
        $(INCDIR): \
	$(CHIPINC): \
	$(KLMDIR): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PHY_PATH): \

DREAMLINER_OBJS = \
	dreamliner_phy.o \
	dreamliner_fpga.o \
	dreamliner_ge_switch.o \
	dreamliner_xcat2_init.o \
	dreamliner_xcat2_utils.o \
	dreamliner_fpga_fw.o \
	dreamliner_poe.o \
	nim_dm_cpss_extdrv.o \
	nim_dm_cpss_extserv.o \
	nim_dm_cpss_os.o \
	nim_dm_cpss_trace.o

DREAMLINER_BASE = \
	$(DREAMLINER_SRC_PATH)/dreamliner_phy.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_fpga.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_poe.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_ge_switch.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_init.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_utils.c \
	$(DREAMLINER_PLD_PATH)/dreamliner_fpga_fw.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extdrv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extserv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_os.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_trace.c

COM_OBJS = \
	linux_api.o \
	bit_swaps.o \
	print_hex.o \
	linux_stub.o \
	linux_error.o \
	linux_mem.o \
	linux_pcie.o \
	common_utils.o \
	memops.o \
	menu.o \
	testmem.o \
	query.o 

COM_BASE = \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/print_hex.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_pcie.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/query.c 

TARGET_OBJ = \
	linux_pciutils.o \
	nim_dreamliner.o \
	nim_testcard.o \
	nim_test.o \
	platform_stub.o 

TARGET_BASE = \
	linux_pciutils.c \
	nim_dreamliner.c \
	nim_testcard.c \
	nim_test.c \
	platform_stub.c 

DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \

PLAT_HFILES = \
	nim_test_defs.h \
	platform_fru.h

HFILES = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
	linux_pciutils.h 

NET_OBJS = \
	$(COM_OBJS) \
	$(TARGET_OBJ) \
	$(DREAMLINER_OBJS) \
	$(DREAMLINER_LIB) 

CBASE = \
	$(COM_BASE) \
	$(TARGET_BASE) \
	$(DREAMLINER_BASE) 

RM_OBJS = \
	$(COM_OBJS) \
	$(TARGET_OBJ) \
	$(DREAMLINER_OBJS) \

all: $(MAIN_APP)

# shared the same verssion file 
.PHONY: $(MAIN_APP)
$(MAIN_APP): $(HFILES) $(NET_OBJS) 
	@cp $(BMCDIR)/version $(SRCDIR) 
	sh $(IROOT)/utils/banner.sh TACHI_NIM_PCIE
	$(CC) $(CFLAGS) -c banner.c 
	$(CC) $(CFLAGS) $(NET_OBJS) banner.o  -o $(MAIN_APP) $(LDFLAGS)
	@cp $(MAIN_APP) $(BMCDIR)
	@$(OBJDUMP) -t $(MAIN_APP) > $(MAIN_APP).symbols

	touch dependancies
	touch $(MAIN_APP).symbols
        

.PHONY: clean
clean:
	rm -f $(RM_OBJS) banner.* $(MAIN_APP)

#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES) $(ASM)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CFLAGS) -M $(CBASE) $(ASM) > dependancies
	@echo "Computing TAGS file for common/src"
	$(shell which ctags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	 -lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies



######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.3  2018/08/06 02:37:59  harrchan
# Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)
#
# Revision 1.2  2016/04/20 08:54:00  benchen2
# add tachi fru portion
#
# Revision 1.1.2.7  2016/03/01 02:40:36  alpeng
# update version for supporting daily-build
#
# Revision 1.1.2.6  2016/02/19 02:17:05  benchen2
# change nameing rule
#
# Revision 1.1.2.5  2016/02/18 02:53:02  alpeng
# update version and banner
#
# Revision 1.1.2.4  2016/01/29 09:15:07  alpeng
# include nim pcie test while complie
#
# Revision 1.1.2.3  2016/01/29 02:23:39  alpeng
# rename files
#
# Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
# support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code
#
# Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
# first check in for tachi-intel test; fix smart_cookie.c and free.h
#
#
# $Endlog$


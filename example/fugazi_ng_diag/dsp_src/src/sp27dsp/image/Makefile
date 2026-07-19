# $Id: Makefile,v 1.6 2016/10/07 17:51:17 srane Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/Makefile,v $
#------------------------------------------------------------------
# Makefile - Make final downloadable file (with ARM and DSS diag)
#
# Mar, 2012 srane
#
# Copyright (c) 2012-2016 by Cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
#
#------------------------------------------------------------------
# Makefile to create SP27xx load
#
# Oct 2010, pbecerra
#
# Copyright (c) 2010-2011 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
dsp_sp2700_fw_diag.img:

ifeq ($(origin VOBROOT), undefined)
VOBROOT := ../..
endif

OS := $(shell uname -s)
ifneq ($(OS), Linux)
$(error Only a Linux build is supported)
endif

# Locate the base directory for LSI DSP tools
LSI_DIR ?= /auto/aegir-ios/tools/dsp/lsi
ELFUTIL := $(LSI_DIR)/elfutils/linux/bin
RM = rm -f
TFTP_COPY ?= cp -p
TFTP_SERVER ?= 
TFTP_DIR ?= /tftpboot/$(USER)

-include ../makefile.version

# Utility that combines the ELF images into a downloadable form
CONV26X := conv26x-$(OS)
#DUMPSEC := $(VOBROOT)/devtools/dump2cda/get_dump_sections.rb
DUMPSEC := get_dump_sections.rb
CS_DSP := code_sign_dsp

ifeq ($(TDM_ONLY), 1)
DSS_IMG = ../obj-27-tdm/dss_sp2700_sw.eld
else
DSS_IMG = ../dss/obj/dss_sp2700_sw.eld
endif
PPB_IMG =  ../ppb/obj/arm_sp2700_sw.elf

#USE_LSI_SP2700_DRV := yes
DEBUG_CONSOLE := yes

vpath reset_msg.xml ../ppb/src

ifdef RELEASE
KEY_TYPE = REL
else
KEY_TYPE = DEV
endif

dsp_sp2700_fmw.rbf: $(DSS_IMG) $(PPB_IMG) $(CONV26X)
	$(CONV26X) -V -sp27xx \
	-d $(DSS_IMG) \
	-a $(PPB_IMG) -s .jump -s .text \
	-s .rodata -s .first -s .data \
	-s .dss_overlay -s .start -s .dma_seg -s .svt \
	-r $(basename $@).rbf -c $@ -o $(basename $@)_old.c \
	$(XBUILD) $(XUSER) $(XS1) $(XS2) $(XS3)	$(XS4) $(XRELEASE_STRING) || \
	($(RM) $(basename $@).rbf $@ && exit 1;)

dsp_sp2700_fw_diag.img: dsp_sp2700_fmw.rbf $(CS_DSP)
	$(CS_DSP) -P PVDM4 -k $(KEY_TYPE) -v A -T /auto/dspgroup/pvdm_$(KEY_TYPE)_ticket.tic \
	-f dsp_sp2700_fmw.rbf -o $@ || ($(RM) $@ && exit 1;) 
	@(echo "cleaning temps..."; $(RM) *.envsize *.publickey* *.signature*)

reset_msg.elf: reset_msg.xml
	$(ELFUTIL)/borg-reset -O elf -l 0x00000108 $< $@

install: dsp_sp2700_fw_diag.img
	$(TFTP_COPY) dsp_sp2700_fmw.rbf $(TFTP_SERVER)$(TFTP_DIR)
	$(TFTP_COPY) dsp_sp2700_fw_diag.img $(TFTP_SERVER)$(TFTP_DIR)

IMG_BUILD_OPTIONS = \
	DEBUG_CONSOLE=$(DEBUG_CONSOLE)

$(DSS_IMG):
	$(MAKE) -C $(dir $@) $(notdir $@) RELEASE=1 $(IMG_BUILD_OPTIONS)

# SR ??
#dump_sections.h: $(DSS_IMG)
#	ruby get_dump_sections.rb  $(basename $^).map > $@

#$(PPB_IMG): $(DSS_IMG) dump_sections.h
#$(PPB_IMG): $(DSS_IMG)
$(PPB_IMG): 
	$(MAKE) -C $(dir $@) $(notdir $@) RELEASE=0 $(IMG_BUILD_OPTIONS) \
          DIR_DUMP_SECTIONS_H=$(CURDIR)

$(CONV26X):
	$(MAKE) -f conv26x.mk -C .

$(CS_DSP):
	$(MAKE) -f $@.mk -C .

clean:
	$(RM) *.elf dsp_sp2700_f*.* dump_sections.h conv26x.o env.o
	$(MAKE) -C $(dir $(DSS_IMG)) clean
	$(MAKE) -C $(dir $(PPB_IMG)) clean
	$(MAKE) -f conv26x.mk clean
	$(MAKE) -f code_sign_dsp.mk clean


superclean: 
	$(RM) *.elf dsp_sp2700_fmw*.* dump_sections.h conv26x.o env.o
	$(MAKE) -C $(dir $(DSS_IMG)) superclean
	$(MAKE) -C $(dir $(PPB_IMG)) superclean
	$(MAKE) -f conv26x.mk clean
	$(MAKE) -f code_sign_dsp.mk clean

.PHONY: clean superclean $(DSS_IMG) $(PPB_IMG) $(CONV26X)

# $Log: Makefile,v $
# Revision 1.6  2016/10/07 17:51:17  srane
# CSCvb61570 - Move to SWIMS server for code signing
#
# Revision 1.5  2016/09/28 06:13:47  srane
# Fix the broken code sign build.
#
# Revision 1.4  2012/09/10 06:46:39  srane
# Add code signing.
#
# Revision 1.3  2012/06/28 21:20:09  srane
# cleanup all files.
#
# Revision 1.2  2012/05/10 22:46:45  srane
# Add DSS sanity tests, TDM loopback test and exception handling, cleanup the
# startup file for ARM.
#
# Revision 1.1  2012/04/18 18:15:17  srane
# Initial checkin
#
#
#------------------------------------------------------------------
# $Endlog$
#


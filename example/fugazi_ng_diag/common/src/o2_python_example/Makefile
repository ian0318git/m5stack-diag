#$Id: Makefile,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/Makefile,v $
#------------------------------------------------------------------------------
# Makefile - this Makefile will generate executable of the platform test
#            and also call other makefile to do the same and eventually
#            combine them to generate .tgz file.
#            ex: o2_python_example and lebowski_python
#            for the target system(ex. O2) and lebowski module respectively
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
default: all

#
# Tool Chain
#
CC = gcc
PY = python2.7
# make ver.3.81 support else if condition in Makefile
MAKE = make-3.81

#
# platform folder
#
PLAT_TARGET = o2_python_example

#
# The SRG environment common source directory
#
IROOT       = ../../..
INC_DIR     = ../../include
# tar cmd needs related path which started from current folder
SRC_DIR     = ..

#
# The SRG platform specific source directory
#
O2_X86_DIR = $(SRC_DIR)/overlord/x86

##################copy these samples for other module host side################
# lebowski SM source path example
LEB_SRC_DIR = $(SRC_DIR)/lebowski_python/host
DIRS += \
	$(LEB_SRC_DIR)
###############################################################################

#
# The python-specific source directory
#
PY_BIN         = $(IROOT)/../py_interpreter_v2_7_3/py_interpreter
PY_DIR         = $(PY_BIN)/include/$(PY)
PY_SRC_DIR     = $(SRC_DIR)/Python
PY_INC_DIR     = $(PY_SRC_DIR)/include
PY_SCRIPT_DIR  = $(PY_SRC_DIR)/script
PY_COM_SRC_DIR = $(PY_SRC_DIR)/src
PY_CFG_DIR     = $(PY_SRC_DIR)/config

#
# Include paths for compiler
#
INC	= \
	-I$(PY_DIR) \
	-I$(PY_INC_DIR) \
	-I$(PY_SRC_DIR) \
	-I$(INC_DIR) \
	-I$(O2_X86_DIR)

#
# GCC FLAGS
#
LDFLAGS            = -L$(PY_BIN)/lib -l$(PY) -lutil -ldl -lnsl -lpthread -lm \
-lrt -Wl,--relax

#
# python-specific source files
#
PY_BASE = \
	$(PY_COM_SRC_DIR)/python_error.c \
	$(PY_COM_SRC_DIR)/python_util.c

PY_DIAG_PCFG = \
	$(PY_CFG_DIR)/diag_flag.pcfg

PY_DIAG_HFILE = \
	$(PY_INC_DIR)/diag_flag_create_from_py.h

#
# SRG environment Source files
#
PLAT_BASE = \
	o2_platform_fru.c

NET_BASE = \
	$(PY_BASE) \
	$(PLAT_BASE)


############## modify the part below for o2 platform side######################
#
# The SRG platform specific source directory
#
O2_SRC_DIR = $(SRC_DIR)/$(PLAT_TARGET)

PLAT_BASE += \
	$(O2_SRC_DIR)/o2_util.c

#
# Include paths for compiler
#
INC	+= \
	-I$(O2_SRC_DIR)

#
# o2 platform source code example
#
O2_PLAT_INIT = \
	o2_platform_init.c

O2_SM = \
	o2_sm_test.c

O2_VM = \
	o2_vm_test.c

O2_WIC = \
	o2_wic_test.c

O2_USB = \
	o2_usb_test.c

O2_FLAG = \
	o2_diag_flag_disp_test.c

O2_CF = \
	o2_cf_test.c

O2_DASH_FPGA = \
	o2_dash_fpga_reg_test.c
	
#
# o2 platform obj file example
#
OBJ_O2_PLAT_INIT = o2_platform_init
OBJ_O2_SM        = o2_sm_test
OBJ_O2_VM        = o2_vm_test
OBJ_O2_WIC       = o2_wic_test
OBJ_O2_USB       = o2_usb_test
OBJ_O2_FLAG      = o2_diag_flag_disp_test
OBJ_O2_CF        = o2_cf_test
OBJ_O2_DASH_FPGA = o2_dash_fpga_reg_test
OBJ_O2 = \
	$(OBJ_O2_PLAT_INIT) \
	$(OBJ_O2_SM) \
	$(OBJ_O2_VM) \
	$(OBJ_O2_WIC) \
	$(OBJ_O2_USB) \
	$(OBJ_O2_FLAG) \
	$(OBJ_O2_CF) \
	$(OBJ_O2_DASH_FPGA)

#
# o2 make rule example
#
$(OBJ_O2_PLAT_INIT):$(O2_PLAT_INIT)
$(OBJ_O2_SM)       :$(O2_SM)
$(OBJ_O2_VM)       :$(O2_VM)
$(OBJ_O2_WIC)      :$(O2_WIC)
$(OBJ_O2_USB)      :$(O2_USB)
$(OBJ_O2_FLAG)     :$(O2_FLAG)
$(OBJ_O2_CF)       :$(O2_CF)
$(OBJ_O2_DASH_FPGA):$(O2_DASH_FPGA)
$(OBJ_O2):
	$(CC) $(NET_BASE) $^ -o $@ $(INC) $(LDFLAGS)

#
# the sets of directories to do various things in
#
CLEANDIRS = $(DIRS:%=clean-%)

all:$(PY_DIAG_HFILE) $(OBJ_O2)
	for dir in $(DIRS); do \
		$(MAKE) PLAT_TYPE=OVLD -C $$dir; \
	done
	tar zcvf $(PLAT_TARGET).tgz --exclude=*.pyc --exclude=*.c \
	--exclude=*.h --exclude=create_diag_flag_to_C.py --exclude=Makefile \
	--exclude=o2_py_tftp.sh --exclude=CVS --exclude=*.tgz \
	$(PY_SCRIPT_DIR) $(PY_CFG_DIR) $(O2_SRC_DIR) $(DIRS)

$(PY_DIAG_HFILE):$(PY_DIAG_PCFG)
#generate the diag_flag_create_from_py.h by executing following python cmd
#note that the arg[1] after create_diag_flag_to_C.py is diag_flag.pcfg path
#arg[2] is diag_flag_create_from_py.h path
#$@ : Target name
#$^ : All dependencies/prerequisites
	$(PY_BIN)/bin/python $(PY_SCRIPT_DIR)/create_diag_flag_to_C.py $^ $@

clean:$(CLEANDIRS)
	rm -f $(OBJ_O2) $(PLAT_TARGET).tgz
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean


.PHONY: all clean

#**********History**********
#$Log: Makefile,v $
#Revision 1.2  2014/06/03 10:53:31  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.13  2014/04/29 11:40:37  erwu2
#update python file structure
#
#Revision 1.1.2.12  2014/04/24 08:53:52  erwu2
#merge makefile and add flag example to test
#
#Revision 1.1.2.11  2014/04/19 07:14:35  erwu2
#modularized makefile updated
#
#Revision 1.1.2.10  2014/04/10 09:42:23  erwu2
#create obj folders in Makefile
#
#Revision 1.1.2.9  2014/04/10 06:24:05  erwu2
#classify o2 and lebowski executable to obj folder
#
#Revision 1.1.2.8  2014/01/27 11:32:56  erwu2
#improve print out message and add description to py files
#
#Revision 1.1.2.7  2014/01/23 09:13:23  erwu2
#show slot num to executable
#
#Revision 1.1.2.6  2014/01/20 05:53:36  erwu2
#improve executable building
#
#Revision 1.1.2.5  2014/01/16 11:00:10  erwu2
#update python files
#
#Revision 1.1.2.4  2013/12/19 10:25:19  erwu2
#improve tftp dnld process
#
#Revision 1.1.2.3  2013/12/13 11:01:34  erwu2
#improve diag flag to C process
#
#Revision 1.1.2.2  2013/12/13 09:46:10  erwu2
#add lebowski pcfg and create_diag_flag_to_C.py
#
#Revision 1.1.2.1  2013/12/09 06:20:30  erwu2
#python menu for o2 example
#
#
#
#$Endlog$

#$Id: o2_py_menu.sh,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_py_menu.sh,v $
#------------------------------------------------------------------------------
#Description: script for executing python menu in o2 python example
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
#!/bin/bash
# absolute path in target system below: cd /python_menu/Python/script
cd ../Python/script
/py_interpreter/bin/python diag_menu.py \
/python_menu/o2_python_example/python/o2_diag_export.txt

#**********History**********
#$Log: o2_py_menu.sh,v $
#Revision 1.2  2014/06/03 10:53:31  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.4  2014/01/27 11:32:56  erwu2
#improve print out message and add description to py files
#
#Revision 1.1.2.3  2014/01/16 11:00:10  erwu2
#update python files
#
#Revision 1.1.2.2  2013/12/19 10:25:19  erwu2
#improve tftp dnld process
#
#Revision 1.1.2.1  2013/12/09 06:20:30  erwu2
#python menu for o2 example
#
#
#
#$Endlog$

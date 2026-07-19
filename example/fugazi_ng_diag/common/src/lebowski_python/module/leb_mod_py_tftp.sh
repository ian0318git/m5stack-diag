#$Id: leb_mod_py_tftp.sh,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_py_tftp.sh,v $
#------------------------------------------------------------------------------
#Description: tftp download script for python interpreter binary and
#             lebowski_python_example.
#
#Oct 2013 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
#!/bin/bash

if [ ! -d "/tmp/python" ]
then
    mkdir /tmp/python
fi
if [ ! -d "/module_python_menu" ]
then
    mkdir /tmp/module_python_menu
fi

#tftp download interpreter binary and leb_python_example from TFTPDIR
tftp -g -r $TFTPDIR/py_bin_v2_7_3.tgz -l /tmp/python/py_bin_v2_7_3.tgz \
$TFTP_SERVER
tftp -g -r $TFTPDIR/leb_python_example.tgz -l \
/tmp/python/leb_python_example.tgz $TFTP_SERVER

#decompress tgz files
tar zxvf /tmp/python/py_bin_v2_7_3.tgz -C /
tar zxvf /tmp/python/leb_python_example.tgz -C /module_python_menu
chmod 777 -R /py_interpreter /module_python_menu

#remove tgz files under /tmp/python
rm -rf /tmp/python

#**********History**********
#$Log: leb_mod_py_tftp.sh,v $
#Revision 1.2  2014/06/03 10:53:30  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.1  2014/04/24 08:53:50  erwu2
#merge makefile and add flag example to test
#
#
#
#$Endlog$

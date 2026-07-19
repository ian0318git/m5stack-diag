'''
$Id: create_diag_flag_to_C.py,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/script/create_diag_flag_to_C.py,v $
-------------------------------------------------------------------------
Description: create Diag flags from python menu to C executable
Oct 2013 - erwu2

Copyright (c) 2013-2014 by Cisco Systems, Inc.

All rights reserved.

-------------------------------------------------------------------------
'''
# import other py as module
from glo_var import *
from util import *
import sys
import os
import collections

#------------------------------define marco------------------------------------
AUTO_GEN_DEF = '__DIAG_FLAG_CREATE_FROM_PY_H__'
DIAG_FLAG_PATH = sys.argv[1]
AUTO_PY_H = sys.argv[2]

#------------------------------------------------------------------------------
#
#  Function : generate_diag_flag_for_C_exec
#
#  Description : this function is used for converting Diag flags from
#                diag_flag.pcfg to diag_flag_create_from_py.h.
#                there should be two args which are passed to this function
#                arg[1] : diag_flag.pcfg path
#                arg[2] : diag_flag_create_from_py.h path
#
# Inputs  : None
#
# Outputs : None
#
#------------------------------------------------------------------------------
def generate_diag_flag_for_C_exec() :
    # load flag capital to dict KEY
    capital_dict = collections.OrderedDict()
    flag_macro_val_in_h = 0x1

    if (os.path.exists(''.join(DIAG_FLAG_PATH))) :
        fp = open(''.join(DIAG_FLAG_PATH), 'r')
    else :
        print 'open file %s failed' % DIAG_FLAG_PATH
        raise

    for line in fp.readlines() :
        line = line.strip()
        # ignore lines started with '#' and empty ones
        if not len(line) or line.startswith('#') :
            continue

        if (line.startswith('$')) :
            items = [item.strip() for item in line.split(':')]
            var.flag_menu_keyname = items[0]
    fp.close()

    mod_table = collections.OrderedDict()
    mod_table = parse_module_file(DIAG_FLAG_PATH)

    if (mod_table == collections.OrderedDict()) :
        print "diag flag table is empty!, please check diag_flag.pcfg path"
        raise

    if (os.path.exists(AUTO_PY_H)) :
        os.remove(AUTO_PY_H)
    fp = open(AUTO_PY_H,'w')
    fp.write('#ifndef ' + AUTO_GEN_DEF + '\n')
    fp.write('#define ' + AUTO_GEN_DEF + '\n\n')

    # iteritems() : Return an iterator over the dictionary (key, value) pairs
    for key,value in (mod_table.iteritems()) :
        if (MACRO.QUO_FLAG in key[0]) and ( (MACRO.ON_FLAG in value[7]) or \
        (MACRO.OFF_FLAG in value[7]) ) :
            key_loaded = key[0].split(MACRO.QUO_FLAG)[1]

            capital_dict[ get_capital(str(key_loaded))] = [key_loaded,\
            ''.join(value[7]), flag_macro_val_in_h]

            fp.write('#define F_' + key_loaded.upper() + ' ' \
            + str(hex(flag_macro_val_in_h)) + '\n')

            flag_macro_val_in_h = flag_macro_val_in_h * 2

    fp.write('\n')
    for key,value in (capital_dict.iteritems()) :
        fp.write('bool '+ ''.join(value[0]).lower() + ';' '\n')

    fp.write('\n#endif /*' + AUTO_GEN_DEF + '*/\n')
    fp.close()


if __name__ == "__main__":
    # if this file is imported from outside py file,
    # __name__ will be filename "create_diag_flag_to_C"
    generate_diag_flag_for_C_exec()

'''
**********History**********
$Log: create_diag_flag_to_C.py,v $
Revision 1.2  2014/06/03 10:53:28  erwu2
python menu collapsed to main trunk

Revision 1.1.2.9  2014/03/07 10:29:39  erwu2
add common submenu comments

Revision 1.1.2.8  2014/02/26 08:43:07  erwu2
1. support basic util submenu. 2. reduce args in parse function

Revision 1.1.2.7  2014/02/20 08:33:18  erwu2
support Xa~Xz mitem_letter

Revision 1.1.2.6  2014/02/17 11:38:24  erwu2
support 1a~1z mitem_letter and global MACRO/var class

Revision 1.1.2.5  2014/02/10 11:29:06  erwu2
add more comments to files

Revision 1.1.2.3  2014/01/16 11:15:54  erwu2
update python files

Revision 1.1.2.2  2013/12/13 11:17:16  erwu2
improve diag flag to C process

Revision 1.1.2.1  2013/12/13 09:46:10  erwu2
add lebowski pcfg and create_diag_flag_to_C.py



$Endlog$
'''


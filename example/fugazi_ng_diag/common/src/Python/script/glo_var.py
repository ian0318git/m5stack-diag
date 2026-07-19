'''
$Id: glo_var.py,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/script/glo_var.py,v $
-------------------------------------------------------------------------
Description: Diag python menu global definitions shared across py files
             in the same folder

Oct 2013 - erwu2

Copyright (c) 2013-2014 by Cisco Systems, Inc.

All rights reserved.

-------------------------------------------------------------------------
'''
import collections

#------------------------------------------------------------------------------
#
#  Class : MACRO
#
#  Description : Diag python menu global definitions shared across files
#                in the same folder. Other py file will use
#                "class.attribute" form to access attribute
#                e.g. in glo_var.py : C_FLAG = 'C'
#                                     (C_FLAG initialise to 'C')
#                        in util.py : if (MACRO.C_FLAG in self.flag.keys())
#                                     (use MACRO.C_FLAG as 'C')
#                all global macro will be difined in the class
#
#------------------------------------------------------------------------------
class MACRO(object) :
    #------------------------------define marco--------------------------------
    #tmp file created from executable to inform python menu
    SRC_PATH       = './'
    ERRACCU_FILE   = 'erraccu.tmp'
    STOPONERR_FILE = 'stoponerr.tmp'
    WARNCNT_FILE   = 'warncnt.tmp'

    #menu item flag
    C_FLAG = 'C'
    S_FLAG = 'S'
    U_FLAG = 'U'
    X_FLAG = 'X'
    M_FLAG = 'M'
    E_FLAG = 'E'

    #flag string
    ON_FLAG   = 'ON'
    OFF_FLAG  = 'OFF'
    QUO_FLAG  = '"'
    ZERO_FLAG = '0'
    FLAG_STR  = 'flag'
    BASIC_STR = 'basic_util'
    RET_INT   = 'keyboard interrupt'

#------------------------------------------------------------------------------
#
#  Class : var
#
#  Description : Diag python menu global definitions shared across files
#                in the same folder. Other py file will use
#                "class.attribute" form to access attribute
#                e.g. in glo_var.py : path_exec_path = []
#                                     (path_exec_path initialise to empty)
#                        in util.py : var.path_exec_path = path_list
#                                     (path_exec_path assigned to path_list)
#                all global variable will be difined in the class
#
#------------------------------------------------------------------------------
class var(object) :
    #------------------------------define variable-----------------------------
    path_exec_path = [] # store 'EXEC' path which be defined in
                        # $product_diag_export.txt
    menu_keyname_list = [] # store menu_keyname

    welcome_str = '' # store welcome message from $product_diag_export.txt
    img_mode_str = 'one big image'  # store image mode(one or individual img)
                                    # from $product.pcfg
    prod_type_str = 'routing' # store product type from $product.pcfg
    flag_menu_keyname = '' # diag_flag.pcfg menu_keyname(e.g. $alter_diag_flags)
    basic_menu_keyname = '' # basic.pcfg menu_keyname(e.g. $basic_utilities)

    stoponerr_flag = 0 # toggle to 1 if 'stoponerr.tmp' is detected

    # common submenu(e.g. alter diag flags/basic utilities) start id
    com_submenu_menu_id = 100
    # common submenu (e.g. alter diag flags/basic utilities) mod side start id
    com_submenu_mod_menu_id = 0
    mod_menu_start_id = 200 # module side menu start id

    #--------------------------------------------------------------------------
    # module_dict
    # parse all the mitems of $io.pcfg to dictionary.
    # If mitem_submenu is '$alter_diag_flags', all the mitems of diag_flag.pcfg
    # will first be parsed to common_table_dict first, then added to this
    # module_dict. And then if ...
    # 1. module side
    #  add this var.module_dict to g_menu_dict if module id is read
    #  (e.g. add sm slotX leb_host_16p.pcfg to g_menu_dict)
    #
    # 2. special case - create diag_flag_create_from_py.h, from
    #                   create_diag_flag_to_C.py
    #  normally diag_flag.pcfg would be parsed to common_table_dict then added
    #  to g_menu_dict if the menu item has "$alter_diag_flags" as its
    #  submenu. But in this case diag_flag.pcfg will still be parsed to
    #  module_dict but not added to g_menu_dict
    #
    #--------------------------------------------------------------------------
    module_dict = collections.OrderedDict()

    #--------------------------------------------------------------------------
    # common_table_dict
    #
    # store diag_flag.pcfg and $basic.pcfg to dictionary.
    # If mitem_submenu is '$alter_diag_flags' or $basic_utilities, all the
    # mitems of diag_flag.pcfg/$basic.pcfg will first be parsed to this
    # common_table_dict first, then added to g_menu_dict or module_dict.
    #
    #--------------------------------------------------------------------------
    common_table_dict = collections.OrderedDict()


'''
**********History**********
$Log: glo_var.py,v $
Revision 1.2  2014/06/03 10:53:28  erwu2
python menu collapsed to main trunk

Revision 1.1.2.13  2014/04/29 11:40:39  erwu2
update python file structure

Revision 1.1.2.12  2014/04/10 06:24:05  erwu2
classify o2 and lebowski executable to obj folder

Revision 1.1.2.11  2014/03/26 03:32:35  erwu2
support image mode and product type

Revision 1.1.2.10  2014/03/07 10:29:39  erwu2
add common submenu comments

Revision 1.1.2.9  2014/02/26 08:43:07  erwu2
1. support basic util submenu. 2. reduce args in parse function

Revision 1.1.2.8  2014/02/20 08:33:18  erwu2
support Xa~Xz mitem_letter

Revision 1.1.2.7  2014/02/17 11:38:24  erwu2
support 1a~1z mitem_letter and global MACRO/var class

Revision 1.1.2.6  2014/02/13 11:17:48  erwu2
improve variable naming

Revision 1.1.2.5  2014/02/10 11:29:06  erwu2
add more comments to files

Revision 1.1.2.3  2014/01/23 09:13:23  erwu2
show slot num to executable

Revision 1.1.2.2  2014/01/16 11:15:55  erwu2
update python files

Revision 1.1.2.1  2013/12/09 06:20:31  erwu2
python menu for o2 example



$Endlog$
'''

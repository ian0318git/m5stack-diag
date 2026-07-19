'''
$Id: util.py,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/script/util.py,v $
-------------------------------------------------------------------------
Description: utility functions used across py files in the same folder

Oct 2013 - erwu2

Copyright (c) 2013-2014 by Cisco Systems, Inc.

All rights reserved.

-------------------------------------------------------------------------
'''
# import other py as module
from glo_var import *
from re import findall
import os
import shutil
import pyinotify
import time
import sys
import subprocess
import collections

#------------------------------define marco------------------------------------
WELCOME_STR_LENGTH = 45
FIND_CMD = 'find '       # find executable file name
FIND_CMD_ARG = ' -name ' # find cmd parameter
TMP_PATH  = './path_tmp'
TMP1_PATH = './path_tmp1'

#------------------------------define variable---------------------------------
# global variable begin with "g_" : across THIS file
#     e.g.  global g_path_dict
#
# global class begin with class name : ACROSS py files in the same folder
#     e.g.  MACRO.FLAG_STR     : global macro class
#           var.stoponerr_flag : global variable class

# Store mitem_submenu of all .pcfg file including $platform.pcfg & $io.pcfg..etc
# & use it to search for corresponding menu,
g_mitem_submenu_list = []

# When parsing diag_flag.pcfg/basic.pcfg for each '$alter_diag_flags' or
# '$basic_utilities' submenu, set the flag to 1 to increase menu id
# for each '$alter_diag_flags' or '$basic_utilities' submenu corresponding to
# different menu. Menu id of these submenu NEEDS to be unique because we need
# to know which menu it's coming from so that it can go back.
g_common_menu_flag = 0

# Parse and store all paths in order, from $product_diag_export.txt
# key = platform/flag/basic_util or moduleID
# value = corresponding path
g_path_dict = collections.OrderedDict()

# Parse all the mitems of $platfrom.pcfg to dictionary.
# If mitem_submenu is '$alter_diag_flags', all the mitems of diag_flag.pcfg
# will first be parsed to common_table_dict first, then added to this
# g_menu_dict.
# If platform support io modules, all the mitems of $io.pcfg are parsed to
# var.module_dict first, then added to this g_menu_dict.
# key   - (mitem_name,
#          menu_id)
# value - [menu_id,
#          mitem_letter,
#          mitem_function,
#          from_where,
#          how_to_enter,
#          menu_title,
#          mitem_submenu,
#          mitem_flag]
#
# (e.g.1. key   - ('motherboard test', 0)
#         value - [0, 'g', '0', '', '', 'Python Diag Main Menu',
#                 '$Motherboard_tests', ['DOALL']])
#
# (e.g.2. key   - ('DASH FPGA register test', 2)
#         value - [2, 'j', 'DASH_FPGA_register_test', 0, 'gg',
#                 'Motherboard Subtest Menu', '0', ['DOALL']]
g_menu_dict = collections.OrderedDict()

#------------------------------------------------------------------------------
#
#  Class : file_mon
#
#  Description : this class define a file monitor trigger to observe whether
#                stoponerr.tmp is created by C executable or not.
#
#  Inputs : pyinotify.ProcessEvent - register a process event to observe
#
#  Outputs : None
#
#------------------------------------------------------------------------------
class file_mon(pyinotify.ProcessEvent) :
    #--------------------------------------------------------------------------
    #
    #  Function : process_IN_MODIFY
    #
    #  Description : this function observes whether any files in specific path
    #                are modified or not.
    #
    #  Inputs : event - observe any file modified events
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def process_IN_MODIFY(self, event) :
        if (event.name == MACRO.STOPONERR_FILE) and \
        (os.path.exists(MACRO.SRC_PATH + MACRO.STOPONERR_FILE)) :
            var.stoponerr_flag = 1


# utility functions

#------------------------------------------------------------------------------
#
#  Function : parse_menu_file
#
#  Description : this function parses pcfg content to dictionary form.
#
#  dictionary form : refer to the description of g_menu_dict
#
#  Inputs : filename - absolute pcfg file path
#
#  Outputs : None - if failed, retrun None
#            g_menu_dict - if successful, return parsed menu dictionary
#
#------------------------------------------------------------------------------
def parse_menu_file(filename) :
    global g_mitem_submenu_list
    global g_common_menu_flag
    global g_path_dict
    global g_menu_dict

    ret = collections.OrderedDict()
    key_title = '' # key of g_menu_dict : (mitem_name,menu id)
    mitem_submenu = ''
    mitem_flag = ''
    menu_keyname = ''
    line_cnt = 0 # line number in each .pcfg file, for debuging purpose
    item_line_count = 0 # one menu item has 2 lines in pcfg body part,
                        # if set to 1, go to line 2 : mitem_function

    skip_flag = 0 # if 1 : skip rest line until next dollar sign.
                  # if menu_keyname doesn't match any mitem_submenu,
                  # the script would skip the body.
    menu_cnt = 0 # count dollar sign to detect 2 or more consecutive
                 # menu_keyname (e.g. menu is declared with no body part)

    menu_id = 0  # specific id to every single menu
    mitem_letter = '' # menu item option/selection
    mitem_function = '' # menu item executable and its args
    from_where = '' # parent's menu id
    how_to_enter = '' # double characters selection input,
                      # which came from parent's menu
                      # (e.g. input 'gg' to go to motherboard submenu,
                      # each motherboard submenu item will record 'gg')
    menu_title = '' # each menu title above the printed menu
    mitem_name = '' # menu item name in body part


    if (os.path.exists(filename)) :
        fp = open(filename, 'r')
    else :
        print 'parse_menu_file open file %s failed' % (filename)
        return ret

    try :
        for line in fp.readlines() :
            line = line.strip() # remove leading/trailing whitespace
            line_cnt = line_cnt + 1 # line number counter

            # ignore lines started with '#' and empty ones
            if ((not len(line)) or line.startswith('#')) :
                continue
            # lines started with '@' provide image mode and product type
            if (line.startswith('@')) :
                items = [item.strip() for item in line.split(':')]
                if (len(items) != 2) :
                    print "line for image mode or product type in .pcfg",
                    print "should have 2 items"
                    raise

                if ('img_mode' in items[0]) :
                    var.img_mode_str  = items[1]

                if ('prod_type' in items[0]) :
                    var.prod_type_str  = items[1]

            elif (line.startswith('$')) :
                # title part
                menu_cnt = menu_cnt + 1

                if (menu_cnt >= 2) :
                    # if user write 2 consecutive menu_keyname(dollar sign line)
                    # in pcfg without body part, raise exception
                    print 'please check file %s if 2' % (filename),
                    print "consecutive titles without body part in the file"
                    raise

                # every time go to line.startswith('$'), clear the flag assuming
                # menu_keyname has a matching mitem_submenu
                # and subsequent lines are the body part. Might set skip flag
                # is equal 1, see below.
                skip_flag = 0

                if (line.find('main_menu') != -1) :
                    # main menu id starts from 0
                    menu_id = 0
                else :
                    items = [item.strip() for item in line.split(':')]
                    if (len(items) != 2) :
                        print "title name's line in .pcfg should have 2 items"
                        raise
                    menu_keyname = items[0]
                    # append menu_keyname to list for showing menu title
                    var.menu_keyname_list.append(menu_keyname)

                    if (menu_keyname not in g_mitem_submenu_list) :
                        # if menu_keyname doesn't appear in each
                        # mitem_submenu column, no need to add to menu,
                        # skip the rest line until next "$" line
                        skip_flag = 1
                        continue

                    for key,value in (g_menu_dict.iteritems()) :
                        mitem_submenu = value[6]
                        if (mitem_submenu in line) :
                            # if menu_keyname has listed in previous parent
                            # mitem_submenu, create submenu
                            # note that if this funciton is being called
                            # recursively, this menu_id will be overwritten.
                            menu_id = menu_id + 1
                            # note that if the mitem_submenu is a common menu
                            # (e.g. $alter diag flag, $basic utilities )
                            # from_where is the last occurrence in the
                            # g_menu_dict
                            from_where = value[0]
                            mitem_letter = value[1]
                            if (len(mitem_letter) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = mitem_letter + mitem_letter[1]
                            else :
                                how_to_enter = mitem_letter * 2

                    # add common menu (alter diag flag/ basic util) as submenu
                    # in this case menu_id is overwritten starting from 100
                    # for common menu
                    if (g_common_menu_flag == 1) :
                        var.com_submenu_menu_id = var.com_submenu_menu_id + 1
                        menu_id = var.com_submenu_menu_id
                        # eventually, com_submenu_mod_menu_id will continue
                        # from the last com_submenu_menu_id
                        var.com_submenu_mod_menu_id = var.com_submenu_menu_id

                items = [item.strip() for item in line.split(':')]
                try :
                    menu_title = items[1]
                except :
                    print 'please check menu_title in line %s'%(line_cnt)
                    return None

            else :
                # body part
                if (skip_flag == 0) :
                    menu_cnt = 0
                    items = [item.strip() for item in line.split(':')]

                    if ((item_line_count == 0) and (items[0] != '')) :
                        # increment to prepare to read the 2nd line of mitem
                        item_line_count = item_line_count + 1

                        # 1st line of mitem
                        mitem_letter = items[0]
                        mitem_name = items[1]
                        key_title = (mitem_name,menu_id)
                        mitem_submenu = items[2]
                        # for now, only support either 'DOALL' or '0', or in
                        # diag flag menu:"OFF" or "ON"
                        mitem_flag = \
                        [item.strip() for item in items[3].split('|')]

                        g_mitem_submenu_list.append(mitem_submenu)
                        # dictionary key can be string, number, tuple.(fixed)
                        # mitem_function is in 2nd line of each mitem which
                        # will be filled later. for now put in ''
                        g_menu_dict[key_title] = \
                        [menu_id, mitem_letter, '', from_where, how_to_enter, \
                        menu_title, mitem_submenu, mitem_flag]

                        # add common submenu(e.g. basic utilities menu)
                        # if mitem_submenu == "$basic_utilities"
                        if (mitem_submenu == var.basic_menu_keyname) :
                            for item in g_path_dict.keys() :
                                if (item == MACRO.BASIC_STR) :
                                    common_table_full_path = \
                                    ''.join(g_path_dict[item])# list to string

                            g_common_menu_flag = 1
                            var.common_table_dict = parse_menu_file\
                            (common_table_full_path)
                            g_common_menu_flag = 0

                            if (var.common_table_dict == collections\
                            .OrderedDict()) :
                                print ' %s submenu parsed failed!' %\
                                (var.basic_menu_keyname)
                            g_menu_dict.update(var.common_table_dict)

                        # add common submenu(e.g. alter diag flags menu)
                        # if mitem_submenu == "$alter_diag_flags"
                        if (mitem_submenu == var.flag_menu_keyname) :
                            for item in g_path_dict.keys() :
                                if (item == MACRO.FLAG_STR) :
                                    common_table_full_path = \
                                    ''.join(g_path_dict[item])# list to string

                            g_common_menu_flag = 1
                            var.common_table_dict = parse_menu_file\
                            (common_table_full_path)
                            g_common_menu_flag = 0

                            if (var.common_table_dict == collections\
                            .OrderedDict()) :
                                print ' %s submenu parsed failed!' %\
                                (var.flag_menu_keyname)
                            g_menu_dict.update(var.common_table_dict)

                    elif ((item_line_count == 1) and (items[0] == '')) :
                        # 2nd line of mitem
                        item_line_count = 0
                        mitem_function = items[1]
                        g_menu_dict[key_title][2] = mitem_function

                    else :
                        print "please check column format in .pcfg"
                        raise

    except :
        g_menu_dict = collections.OrderedDict()
        print 'please check format,\nfilename = %s\nline[%s] %s'%\
        (filename,line_cnt,line)

    fp.close()
    line_cnt = 0
    item_line_count = 0
    return g_menu_dict

#------------------------------------------------------------------------------
#
#  Function : parse_module_file
#
#  Description : this function parses module side pcfg (e.g. sm/vm/wic) content
#                to dictionary form. it will be added as module submenu
#                if module id is found.
#
#  module_dict : store file to dictionary from non-main menu side
#                1. module side
#                 add var.module_dict to g_menu_dict if module id is got
#                 (e.g. add sm slotX leb_host_16p.pcfg to g_menu_dict)
#
#                2. diag flag side (special case)
#                 normally diag_flag.pcfg would be parsed to common_table_dict
#                 and added to main menu structure if the menu item has
#                 "alter diag flag" as its submenu. In this special case
#                 diag_flag.pcfg would be parsed in create_diag_flag_to_C.py
#                 to generate diag_flag_create_from_py.h
#
#  dictionary form : refer to the description of g_menu_dict
#
#  Inputs : filename - absolute pcfg file path
#
#  Outputs : None - if failed, retrun None
#            var.module_dict - if successful, return parsed menu dictionary
#
#------------------------------------------------------------------------------
def parse_module_file(filename) :
    global g_mitem_submenu_list
    global g_common_menu_flag
    global g_path_dict

    ret = collections.OrderedDict()
    common_table_dict = collections.OrderedDict()
    key_title = '' # key of var.module_dict : (mitem_name,menu id)
    mitem_submenu = ''
    mitem_flag = ''
    menu_keyname = ''
    line_cnt = 0 # line number in each .pcfg file, for debuging purpose
    item_line_count = 0 # one menu item has 2 lines in pcfg body part,
                        # if set to 1, go to line 2 : mitem_function

    skip_flag = 0 # if 1 : skip rest line until next dollar sign.
                  # if menu_keyname doesn't match any mitem_submenu,
                  # the script would skip the body.
    menu_cnt = 0 # count dollar sign to detect 2 or more consecutive
                 # menu_keyname (e.g. menu is declared with no body part)

    menu_id = 0  # specific id to every single menu
    mitem_letter = '' # menu item option/selection
    mitem_function = '' # menu item executable and its args
    from_where = '' # parent's menu id
    how_to_enter = '' # double characters selection input,
                      # which came from parent's menu
                      # (e.g. input 'gg' to go to motherboard submenu,
                      # each motherboard submenu item will record 'gg')
    menu_title = '' # each menu title above the printed menu
    mitem_name = '' # menu item name in body part


    if (os.path.exists(filename)) :
        fp = open(filename, 'r')
    else :
        print 'parse_module_file open file %s failed' % (filename)
        return ret

    try :
        for line in fp.readlines() :
            line = line.strip() # remove leading/trailing whitespace
            line_cnt = line_cnt + 1 # line number counter

            # ignore lines started with '#' and empty ones
            if ((not len(line)) or line.startswith('#')) :
                continue

            if (line.startswith('$')) :
                # title part
                menu_cnt = menu_cnt + 1

                if (menu_cnt >= 2) :
                    # if user write 2 consecutive menu_keyname(dollar sign line)
                    # in pcfg without body part, raise exception
                    print 'please check file %s if 2' % (filename),
                    print "consecutive titles without body part in the file"
                    raise

                # every time go to line.startswith('$'), clear the flag assuming
                # menu_keyname has a matching mitem_submenu
                # and subsequent lines are the body part. Might set skip flag
                # is equal 1, see below.
                skip_flag = 0

                if (menu_id == 0 and g_common_menu_flag == 0) :
                    # menu id starts from 200
                    menu_id = var.mod_menu_start_id
                else :
                    items = [item.strip() for item in line.split(':')]
                    if (len(items) != 2) :
                        print "title name's line in .pcfg should have 2 items"
                        raise
                    menu_keyname = items[0]
                    # append menu_keyname to list for showing menu title
                    var.menu_keyname_list.append(menu_keyname)

                    if (menu_keyname not in g_mitem_submenu_list) :
                        # if menu_keyname doesn't appear in each
                        # mitem_submenu column, no need to add to menu,
                        # skip the rest line until next "$" line
                        skip_flag = 1
                        continue

                    for key,value in (var.module_dict.iteritems()) :
                        mitem_submenu = value[6]
                        if (mitem_submenu in line) :
                            # if menu_keyname has listed in previous parent
                            # mitem_submenu, create submenu
                            # note that if this funciton is being called
                            # recursively, this menu_id will be overwritten.
                            menu_id = menu_id + 1
                            # note that if the mitem_submenu is a common menu
                            # (e.g. $alter diag flag, $basic utilities )
                            # from_where is the last occurrence in the
                            # g_menu_dict
                            from_where = value[0]
                            mitem_letter = value[1]
                            if (len(mitem_letter) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = mitem_letter + mitem_letter[1]
                            else :
                                how_to_enter = mitem_letter * 2

                    # add common menu (alter diag flag/ basic util) as submenu
                    # in this case menu_id is overwritten starting from 100
                    # for common menu
                    if (g_common_menu_flag == 1) :
                        var.com_submenu_mod_menu_id = \
                        var.com_submenu_mod_menu_id + 1
                        # eventually, com_submenu_mod_menu_id will continue
                        # from the last com_submenu_menu_id
                        menu_id = var.com_submenu_mod_menu_id

                items = [item.strip() for item in line.split(':')]
                try :
                    menu_title = items[1]
                except :
                    print 'please check menu_title in line %s'%(line_cnt)
                    return None

            else :
                # body part
                if (skip_flag == 0) :
                    menu_cnt = 0
                    items = [item.strip() for item in line.split(':')]

                    if ((item_line_count == 0) and (items[0] != '')) :
                        # increment to prepare to read the 2nd line of mitem
                        item_line_count = item_line_count + 1

                        # 1st line of mitem
                        mitem_letter = items[0]
                        mitem_name = items[1]
                        key_title = (mitem_name,menu_id)
                        mitem_submenu = items[2]
                        # for now, only support either 'DOALL' or '0', or in
                        # diag flag menu:"OFF" or "ON"
                        mitem_flag = \
                        [item.strip() for item in items[3].split('|')]

                        g_mitem_submenu_list.append(mitem_submenu)
                        # dictionary key can be string, number, tuple.(fixed)
                        # mitem_function is in 2nd line of each mitem which
                        # will be filled later. for now put in ''
                        var.module_dict[key_title] = \
                        [menu_id, mitem_letter, '', from_where, how_to_enter, \
                        menu_title, mitem_submenu, mitem_flag]

                        # add common submenu(e.g. basic utilities menu)
                        # if mitem_submenu == "$basic_utilities"
                        if (mitem_submenu == var.basic_menu_keyname) :
                            for item in g_path_dict.keys() :
                                if (item == MACRO.BASIC_STR) :
                                    common_table_full_path = \
                                    ''.join(g_path_dict[item])# list to string

                            g_common_menu_flag = 1
                            common_table_dict = parse_module_file\
                            (common_table_full_path)
                            g_common_menu_flag = 0

                            if (common_table_dict == collections\
                            .OrderedDict()) :
                                print ' %s submenu parsed failed!' %\
                                (var.basic_menu_keyname)
                            var.module_dict.update(common_table_dict)

                        # add common submenu(e.g. alter diag flags menu)
                        # if mitem_submenu == "$alter_diag_flags"
                        if (mitem_submenu == var.flag_menu_keyname) :
                            for item in g_path_dict.keys() :
                                if (item == MACRO.FLAG_STR) :
                                    common_table_full_path = \
                                    ''.join(g_path_dict[item])# list to string

                            g_common_menu_flag = 1
                            common_table_dict = parse_module_file\
                            (common_table_full_path)
                            g_common_menu_flag = 0

                            if (common_table_dict == collections\
                            .OrderedDict()) :
                                print ' %s submenu parsed failed!' %\
                                (var.flag_menu_keyname)
                            var.module_dict.update(common_table_dict)

                    elif ((item_line_count == 1) and (items[0] == '')) :
                        # 2nd line of mitem
                        item_line_count = 0
                        mitem_function = items[1]
                        var.module_dict[key_title][2] = mitem_function

                    else :
                        print "please check column format in .pcfg"
                        raise

    except :
        var.module_dict = collections.OrderedDict()
        print 'please check format,\nfilename = %s\nline[%s] %s'%\
        (filename,line_cnt,line)

    fp.close()
    line_cnt = 0
    item_line_count = 0
    var.mod_menu_start_id = menu_id
    return var.module_dict

#------------------------------------------------------------------------------
#
#  Function : parse_path_file
#
#  Description : this function parses diag export file(e.g. o2_diag_export.txt)
#                content to dictionary form.
#
#
#  g_path_dict example :
#
#                key   : value
#  '$CONFIG_FILE_PATH' : ['/python_menu'],
#    '$EXEC_FILE_PATH' : ['/python_menu/o2_python_example']
#           'platform' : ['/python_menu/o2_python_example/python/o2.pcfg',
#                         'o2_platform_init'],
#               'flag' : ['/python_menu/Python/config/diag_flag.pcfg'],
#              '0xb49' : ['/python_menu/lebowski_python/host/python/
#                          leb_host_16p.pcfg'],
#              '0xb4a' : ['/python_menu/lebowski_python/host/python/
#                          leb_host_24p.pcfg'],
#        'welcome_str' : ['Diagnostic DEMO'],
#
#
#  Inputs : filename - absolute diag export filepath
#
#  Outputs : None - if failed, retrun None
#            g_path_dict - if successful, return parsed path dictionary
#
#------------------------------------------------------------------------------
def parse_path_file(filename) :
    global g_path_dict
    line_cnt = 0 # line number in each parsed file
    ret = None
    path_file_dict = collections.OrderedDict()
    config_path = ''

    if (os.path.exists(filename)) :
        fp = open(filename, 'r')
    else :
        print 'open file %s failed' % filename
        return ret

    for line in fp.readlines() :
        line_cnt = line_cnt + 1
        line = line.strip()

        # ignore lines started with '#' and empty ones
        if ((not len(line)) or line.startswith('#')) :
            continue

        items = [item.strip() for item in line.split(':')]

        if ('FILE_PATH' in items[0]) :

            path_list = [item.strip() for item in items[1].split(';')]

            if ('EXEC' in items[0]) :
                var.path_exec_path = path_list

            if ('CONFIG' in items[0]) :
                path_config_path = path_list

            for each_item in path_list :

                if (os.path.exists(''.join(each_item))) :
                    pass
                else :
                    print 'please check PATH in $product_diag_export.txt existed!'
                    return ret

        if ('.pcfg' in items[1]) :
            # find file in given path
            config_path = find_file(path_config_path,items[1])
            items[1] = config_path

        if (items[0].startswith('$')) :
            path_file_dict[items[0]] = items[1]

        try :
            if (len(items) == 2) :
                g_path_dict[items[0]] = [items[1]]
            elif (len(items) == 3) :
                g_path_dict[items[0]] = [items[1],items[2]]
            else :
                raise
        except :
            print "please check file = %s \n line[%s]=%s "%\
            (filename,line_cnt,line)

    fp.close()
    return g_path_dict

#------------------------------------------------------------------------------
#
#  Function : menu_pr_err_accum
#
#  Description : this function parses error accumulated file(e.g. erraccu.tmp)
#                to get accumulate error value if file existed.
#                accumulated error is recorded from C executable for
#                error times in continuous mode.
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
def menu_pr_err_accum() :
    if (os.path.exists(MACRO.SRC_PATH + MACRO.ERRACCU_FILE)) :
        fp = open(MACRO.SRC_PATH + MACRO.ERRACCU_FILE, 'r')
        for line in fp.readlines() :
            line = line.strip()
        print "\n%s total accumulated errors\n"%(line)
        fp.close()
    else :
        print "\n0 total accumulated errors\n"

#------------------------------------------------------------------------------
#
#  Function : print_welcome_msg
#
#  Description : this function prints welcome message from diag export file
#                (e.g. "Diagnostic DEMO" in o2_diag_export.txt)
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
def print_welcome_msg() :
    print '=' * WELCOME_STR_LENGTH # print '=' 45 times
    print '%s' % (var.welcome_str)
    print '=' * WELCOME_STR_LENGTH
    print "Image Mode = %s"%(var.img_mode_str)
    print "Product Type = %s"%(var.prod_type_str)

#------------------------------------------------------------------------------
#
#  Function : find_file
#
#  Description : this function search absolute path for given file if existed.
#
#  Inputs : path_config_path - target system paths for searching files
#                  Example : ['/python_menu','/ABC']
#           search_items - file for searching
#                  Example : o2.pcfg
#
#  Outputs : None - if file's path can't be found, return None
#            config_path - if file's absolute path is found, return file path
#            MACRO.RET_INT - if ctrl+c is detected,
#                              return 'keyboard interrupt'
#
#------------------------------------------------------------------------------
def find_file(path_config_path, search_items) :
    config_path = ''
    exec_cmd = ''

    try :
        for each_path in path_config_path :
            exec_cmd = FIND_CMD + each_path + FIND_CMD_ARG \
            + ''.join(search_items) + '> ' + ''.join(TMP1_PATH)
            subprocess.call(exec_cmd, shell=True)

            if ((os.path.exists(TMP1_PATH)) and \
            (isfileempty(''.join(TMP1_PATH)) == False)) :
                # copy ./path_tmp1 to ./path_tmp
                shutil.copy2(''.join(TMP1_PATH), ''.join(TMP_PATH))
                os.remove(''.join(TMP1_PATH))

        if (os.path.exists(TMP_PATH)) :
            fp = open(TMP_PATH, 'r')

            for config_path in fp.readlines() :
                config_path = config_path.strip()

                if (len(config_path) == 0) :
                    print "please check EXEC PATH of %s "%(search_items)
                    return None
            os.remove(''.join(TMP_PATH))

        return config_path
    except KeyboardInterrupt :
        return MACRO.RET_INT

#------------------------------------------------------------------------------
#
#  Function : isfileempty
#
#  Description : this function checks if file's size is 0 or not.
#
#  Inputs : filepath - file path
#
#  Outputs : True - file size is 0
#            False - file size is not 0
#
#------------------------------------------------------------------------------
def isfileempty(filepath) :
    fileSize = os.stat(filepath).st_size
    if (fileSize == 0) :
        return True
    else :
        return False

#------------------------------------------------------------------------------
#
#  Function : get_capital
#
#  Description : this function gets capital from given string
#
#  Inputs : str - given string
#
#  Outputs : ret - return capital of given string
#            None - if capital can't be got in double quotes for each flag
#                   from diag_flag.pcfg
#
#------------------------------------------------------------------------------
def get_capital(str) :
    ret = ''.join(findall(r'[A-Z]', str)[:])

    if (len(ret) < 1) :
        print "can't get capital in double quotes from diag_flag.pcfg, "
        print "please confirm only one capital in double quotes"
        return None
    return ret

#------------------------------------------------------------------------------
#
#  Function : trim_double_chars
#
#  Description : this function trim characters string to single or
#                double the single character
#
#  Inputs : str - given string
#
#  Outputs : if given string is single character, return double characters
#            if given double characters, return first character
#            if given triple characters, return 1st & 2nd characters
#
#------------------------------------------------------------------------------
def trim_double_chars(str) :
    if (len(str) == 1) :
        return str * 2
    elif (len(str) == 2) :
        return str[0]
    else :
        # support 'Xa'~'Xz' mitme_letter
        return str[0] + str[1]

#------------------------------------------------------------------------------
#
#  Function : print_error_input
#
#  Description : this function prints error message when input error
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
def print_error_input() :
    print 'Input error.. Please check your input!'

#------------------------------------------------------------------------------
#
#  Function : sleep
#
#  Description : this function makes an alias for time.sleep method
#
#  Inputs : int - This is the number of seconds execution to be suspended,
#                 number could be decimal.(e.g. sleep(0.1) to suspend 100ms)
#
#  Outputs : sleep time in second
#
#------------------------------------------------------------------------------
def sleep(int) :
    return time.sleep(int)

'''
**********History**********
$Log: util.py,v $
Revision 1.2  2014/06/03 10:53:28  erwu2
python menu collapsed to main trunk

Revision 1.1.2.12  2014/04/29 11:40:39  erwu2
update python file structure

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

Revision 1.1.2.3  2014/01/20 05:53:35  erwu2
improve executable building

Revision 1.1.2.2  2014/01/16 11:15:55  erwu2
update python files

Revision 1.1.2.1  2013/12/09 06:20:31  erwu2
python menu for o2 example



$Endlog$
'''

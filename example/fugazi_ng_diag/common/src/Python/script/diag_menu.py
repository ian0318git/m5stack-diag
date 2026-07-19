'''
$Id: diag_menu.py,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/script/diag_menu.py,v $
-------------------------------------------------------------------------
Description: main python menu file

Oct 2013 - erwu2

Copyright (c) 2013-2014 by Cisco Systems, Inc.

All rights reserved.

-------------------------------------------------------------------------
'''
#------------------------------------------------------------------------------
#  please follow the annotation to edit this python file
#  use #(single line) or '''(multiple lines) to make python file comment
#------------------------------------------------------------------------------
# import other py as module
from glo_var import *
from util import *
import os
import glob
import collections
import copy
import pyinotify
import thread
import sys
import subprocess
import copy

#------------------------------define marco------------------------------------
# return value
RET_FALSE                 = 0
RET_TRUE                  = 1
RET_NO_SUBMENU            = 2
RET_NO_EXEC               = 3
RET_INVALID_INPUT         = 4

# task
RET_QUIT                  = 5
RET_SHOW_MENU             = 6
RET_INVOKE_TEST           = 7
RET_GO_BACK_TO_UPPER_MENU = 8

# state
STATE_IS_INIT    = 0
STATE_IS_MENU    = 1
STATE_IS_SUBMENU = 2

# system command
DO_ALL_CMD      = 'do_all'
DO_GRP_CMD      = 'do_grp'
RM_CMD          = 'rm -f '
CAT_CMD         = 'cat '

# delay time
DELAY_TIME      = 0.1 # 100ms

# ascii code
ASCII_ESC       = 27  # esc ascii code is 27
ASCII_RS        = 30  # rs ascii code is 30, use for error input

# common menu id
COM_MENU_ID     = 101

# menu item flag
DO_ALL_FLAG     = 'DOALL'

# file and path
TMP_FILE        = "*.tmp"
PYC_FILE        = "*.pyc"
SLOT_ID_FILE    = 'slot_id.tmp'
ERR_LOG_PATH    = "/errlog.txt"
SHOW_DEBUG_PATH = "/dblog.txt"
MOD_ID_PATH     = './module_id.tmp'

# diag export item
PLAT_STR = 'platform'
WEL_STR  = 'welcome_str'

# error message CLI
ERR_LOG_STR  = 'errlog'
CLR_ERR_STR  = 'clrerr'
SHOW_DBG_STR = 'showdebug'
CLR_DBG_STR  = 'clrdebug'

# python CLI
PATH_STR = 'path'

# image mode and product type
ONE_IMAGE_MODE = 'one_big_image'
ROUTING_TYPE = 'routing'

# alter diags menu title
ALTER_DIAGS_MENU_TITLE = 'toggle menu'

#------------------------------define variable---------------------------------
# interrupt flag
# action to be taken when <ctrl + c> (i.e. g_kb_intr = 1)
# menu prompt level, <ctrl + c> will take care of menu level interrupt,
# return no submenu without printing message
g_kb_intr  = 0
# action to be taken when <ctrl + c> during continuous test (i.e. g_con_intr = 1)
g_con_intr = 0

# user input path
g_export_file_path    = ''
# get pcfg absolute path
g_usr_input_plat_path = ''

# store parsed file path from $product_diag_export.txt in order
g_diag_py_path_dict = collections.OrderedDict()

#------------------------------------------------------------------------------
#
#  Class : diag_menu
#
#  Description : main class for building and running Diag menu
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
class diag_menu(object) :

    #--------------------------------------------------------------------------
    #
    #  Function : update_state
    #
    #  Description : this function updates task and menu level state machine
    #
    #  Inputs : task - RET_QUIT          : input esc and enter for leaving
    #                  RET_SHOW_MENU     : print current menu for user
    #                  RET_INVOKE_TEST   : execute menu item
    #                  RET_GO_BACK_TO_UPPER_MENU : go back to upper level until
    #                                              main menu
    #           opt_subopt - user selected input(optional), default is None
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def update_state(self, task, opt_subopt = None) :
        # update opt, go to menu
        init_show   = self.state == STATE_IS_INIT and task == RET_SHOW_MENU
        menu_invoke = self.state == STATE_IS_MENU and task == RET_INVOKE_TEST

        # update opt, go to submenu
        menu_show = self.state == STATE_IS_MENU and task == RET_SHOW_MENU

        # reset opt subopt, go to init
        menu_quit = self.state == STATE_IS_MENU and \
        (task == RET_QUIT or task == RET_GO_BACK_TO_UPPER_MENU)

        # update subopt, go to submenu
        submenu_invoke = self.state == STATE_IS_SUBMENU and \
        task == RET_INVOKE_TEST

        # reset opt subopt, go to menu
        submenu_quit = self.state == STATE_IS_SUBMENU and \
        (task == RET_QUIT or task == RET_GO_BACK_TO_UPPER_MENU)

        # retain opt, clear subopt, go to submenu
        submenu_show = self.state == STATE_IS_SUBMENU and task == RET_SHOW_MENU

        # opt(how to enter), double input characters to go to submenu
        if (init_show or menu_invoke) :
            self.opt    = opt_subopt
            self.subopt = None
            self.state  = STATE_IS_MENU
        elif (menu_show) :
            self.opt    = opt_subopt
            self.subopt = None
            self.state  = STATE_IS_SUBMENU
        elif (menu_quit) :
            self.opt    = None
            self.subopt = None
            self.state  = STATE_IS_INIT
        elif (submenu_invoke) :
            self.opt    = opt_subopt
            self.subopt = opt_subopt
            self.state  = STATE_IS_SUBMENU
        elif (submenu_quit) :

            if (ord(self.selected_input) == ASCII_ESC) : # call by esc
                # delete last item in list, opt history path
                del self.path_key_list[-1]

                # (from where) == 0, level 2 submenu
                if (self.last_menu.values()[0][3] == 0) :
                    self.opt    = None
                    self.subopt = None
                    self.state  = STATE_IS_MENU
                # (from where) > 0, level 3~N submenu
                elif (self.last_menu.values()[0][3] > 0) :
                    self.opt    = opt_subopt
                    self.subopt = None
                    self.task   = RET_GO_BACK_TO_UPPER_MENU

            if (ord(self.selected_input) == ASCII_RS) :
                # invalid input, quit to last status
                # (from where) == '', level 1 main menu
                if (self.last_menu.values()[0][3] == '') :
                    self.opt    = None
                    self.subopt = None
                    self.state  = STATE_IS_MENU
                else :
                    self.opt    = opt_subopt
                    self.subopt = None
                    self.state  = STATE_IS_SUBMENU
                    self.task   = RET_GO_BACK_TO_UPPER_MENU

        elif (submenu_show) :
            self.opt    = opt_subopt
            self.subopt = None
            self.state  = STATE_IS_SUBMENU
        else :
            print "Error.. Invalid event! Please Check your input.."
            print "(state: %d and self.task: %d)" % (self.state, task)

    #--------------------------------------------------------------------------
    #
    #  Function : load_flags
    #
    #  Description : this function loads Diag flag in order from diag_flag.pcfg
    #                and each bit stands for various flags.
    #                (e.g. 0000 0000 0000 0001  Continuous
    #                      0000 0000 0000 0010  Stop_on_error
    #                           :          :         :
    #                           :          :         :
    #                      0100 0000 0000 0000  exterNal_customer)
    #
    #  Inputs : None
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def load_flags(self) :
        self.default_flag_dict = collections.OrderedDict()
        # flag value of each item of diag_flag.pcfg
        flag_macro_val_in_h = 0x1
        try :
            for key,value in (var.common_table_dict.iteritems()) :
                if (MACRO.QUO_FLAG in key[0]) and (key[1] == COM_MENU_ID) and \
                   ( (MACRO.ON_FLAG in value[7]) or \
                     ( MACRO.OFF_FLAG in value[7]) ) :
                    # key[1] == 101, read first common table(101) to load flag
                    key_loaded = key[0].split(MACRO.QUO_FLAG)[1]
                    self.default_flag_dict[''.join(value[2])]\
                    = [key_loaded,''.join(value[7]),flag_macro_val_in_h ]

                    flag_macro_val_in_h = flag_macro_val_in_h * 2
                    # self.default_flag_dict example
                    # key: value
                    # 'C': ['Continuous', 'OFF', 1]
                    # 'S': ['Stop_on_error', 'OFF', 2]
        except :
            print "please confirm alter diag flags in diag_flag.pcfg"
            print "1. flag name MUST includ capital in quotation mark"
            print "2. defalut flag value MUST be ON/OFF"

        self.flag = copy.deepcopy(self.default_flag_dict)

    #--------------------------------------------------------------------------
    #
    #  Function : show_flag
    #
    #  Description : this function prints Diag flags with ON/OFF state between
    #                menu and prompt
    #
    #  Inputs : None
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def show_flag(self) :
        count = 0
        line = 0
        diag_flag_list = [MACRO.C_FLAG, MACRO.S_FLAG, MACRO.E_FLAG, \
                          MACRO.M_FLAG, MACRO.X_FLAG]
        print 'FLAGS:'
        # for alter diag menu, show all diag flags
        if (self.query_string == ALTER_DIAGS_MENU_TITLE) :
            for key in sorted(self.flag.keys()) :
                if (line == 0) :# first line
                    count = count +1
                print ''.ljust(1),
                # for "FLAGS:" , left justified,  str.ljust(width[, fillchar])
                print self.flag[key][0].ljust(19),
                # comma symbol prevent line changed from print function
                print self.flag[key][1].ljust(3) + ',',
                if (count > 2) :
                    print '' # equal to print'\n'
                    count = 0
                count = count + 1
                line = line + 1

        # other menu, show essential diag flags
        else :
            for key in sorted(self.flag.keys()) :
                # only C,S,E,M,X diag flag need be printed
                if (key in diag_flag_list) :
                    if (line == 0) :# first line
                        count = count +1
                    print ''.ljust(1),
                    # for "FLAGS:", left justified, str.ljust(width[, fillchar])

                    print self.flag[key][0].ljust(19),
                    # comma symbol prevent line changed from print function
                    print self.flag[key][1].ljust(3) + ',',
                    if (count > 2) :
                        print '' # equal to print'\n'
                        count = 0
                    count = count + 1
                    line = line + 1
        print '\n'

    #--------------------------------------------------------------------------
    #
    #  Function : toggle_flag
    #
    #  Description : this function toggles Diag flags when inputting
    #                corresponding flag capital.
    #
    #  Inputs : flag - Diag flags corresponding capital
    #
    #  Outputs : RET_TRUE  - toggle successfully
    #            RET_FALSE - toggle failed
    #
    #--------------------------------------------------------------------------
    def toggle_flag(self, flag) :
        try :
            if (self.flag[flag][1] == MACRO.OFF_FLAG) :
                self.flag[flag][1] = MACRO.ON_FLAG
            elif (self.flag[flag][1] == MACRO.ON_FLAG) :
                self.flag[flag][1] = MACRO.OFF_FLAG
            else :
                print "please check default alter diag flag"

            return RET_TRUE
        except :
            return RET_FALSE

    #--------------------------------------------------------------------------
    #
    #  Function : __init__
    #
    #  Description : this function initialise variables in class,
    #                __init__ will be executed at first if it be defined.
    #
    #  Inputs : None
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def __init__(self) :
        self.opt = None
        self.subopt = None
        self.state = STATE_IS_INIT
        self.update_state(RET_SHOW_MENU)
        self.member = collections.OrderedDict() # parsed whole menu
        self.flag = collections.OrderedDict()
        self.module_id = ''
        self.slot_id = ''
        self.pass_counter = 0
        self.menu_outside = 0
        self.get_main_menu_dict = collections.OrderedDict()
        self.submenu_dict = collections.OrderedDict()
        self.path_key_list = [] # path saved key list

    #--------------------------------------------------------------------------
    #
    #  Function : add_menu
    #
    #  Description : this function add parsed main menu table into
    #                class diag_menu.
    #
    #  Inputs : menu_table - parsed menu table
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def add_menu(self, menu_table) :
        # self.member is main menus parsed
        # from $product.pcfg
        self.member = menu_table

    #--------------------------------------------------------------------------
    #
    #  Function : print_menu
    #
    #  Description : this function prints Diag menu
    #
    #  Inputs : None
    #
    #  Outputs : RET_NO_SUBMENU    - no related submenu
    #            RET_INVALID_INPUT - user input doesn't have related usage
    #            RET_TRUE          - print menu successfully due to input
    #
    #--------------------------------------------------------------------------
    def print_menu(self) :
        global g_kb_intr

        menu_title_store = ''

        if (self.state == STATE_IS_SUBMENU) : # check menu type: main/sub
            self.get_main_menu_dict.clear()
            # get current menu from whole menu
            for key,value in (self.member.iteritems()) :

                if (self.task == RET_GO_BACK_TO_UPPER_MENU and \
                self.print_menu_again != 1) :
                    if (value[0] == self.last_menu.values()[0][3]) :
                        self.get_main_menu_dict[key] = value

                if (value[3] == self.last_menu.values()[0][0] and \
                value[4] == self.opt) :
                    # value[0]: menu id,
                    # value[3]: from where, value[4]: how to enter
                    self.get_main_menu_dict[key] = value

            if (self.print_menu_again != 1) :
                for keys,values in self.last_menu.iteritems() :
                    # save opt history path
                    try :
                        if ((self.opt == values[1] * 2) or \
                        (self.opt == (values[1] + values[1][1]))) and\
                        (self.opt == self.get_main_menu_dict.values()[0][4]) :
                            self.path_key_list.append(keys[0])
                    except :
                        pass

            # invalid input, print menu again
            if (self.print_menu_again == 1) :
                self.get_main_menu_dict = self.last_menu

            menu = self.get_main_menu_dict

            if (menu == collections.OrderedDict()) :
                if (g_kb_intr != 1) :
                    if (self.menu_outside == 1) :
                        pass
                    else :
                        print "submenu isn't supported for this menu"
                return RET_NO_SUBMENU

            count_for_value = 0

            for key,value in (menu.iteritems()) :
                if (count_for_value == 0) :
                    menu_title_store = value[5]
                    if (os.path.exists(MACRO.SRC_PATH + SLOT_ID_FILE)) :
                        fp = open(MACRO.SRC_PATH + SLOT_ID_FILE,'r')
                        self.slot_id = fp.readline()
                        value[5] = 'slot ' + self.slot_id + ' ' + value[5]
                        fp.close()

                    print '\n====',value[5],'====\n'
                    self.query_string = value[5]
                    count_for_value = count_for_value + 1
                    value[5] = menu_title_store

                # value[5]: main menu title, value[6]: submenu display
                if (value[6] in var.menu_keyname_list) or \
                (value[6] == '$' + key[0]) :
                    print '', value[1], ":",  key[0], "*"
                else :
                    print '', value[1], ":",  key[0]

        else :
            # main menu
            if (self.member == collections.OrderedDict()) :
                print ''
                return RET_INVALID_INPUT

            self.get_main_menu_dict.clear()
            count_for_value = 0

            for key,value in (self.member.iteritems()) :
                if (value[0] == 0) :
                    self.get_main_menu_dict[key] = value
                    if (os.path.exists(MACRO.SRC_PATH + SLOT_ID_FILE)) :
                        os.remove(MACRO.SRC_PATH + SLOT_ID_FILE)
                    self.module_id = ''
                    self.slot_id = ''
                    if (count_for_value == 0) : # print main menu title once
                        print '\n====',value[5],'====\n'
                        self.query_string = value[5]

                    if (value[6] in var.menu_keyname_list) or \
                    (value[6] == '$' + key[0]) :
                        print '', value[1], ":",  key[0], "*"
                    else :
                        print '', value[1], ":",  key[0]

                    count_for_value = count_for_value + 1

            menu = self.get_main_menu_dict

            if (menu == collections.OrderedDict()) :
                return RET_INVALID_INPUT

        self.last_menu = self.get_main_menu_dict.copy()
        self.print_menu_again = 0
        return RET_TRUE

    #--------------------------------------------------------------------------
    #
    #  Function : run_handler
    #
    #  Description : this function parses input to get test name(e.g. test SM
    #                Slot 1), test command(e.g. sm_test 1) from pcfg.
    #                these will be used when running menu items
    #
    #  Inputs : input_in_hdlr - single character input through prompt
    #           opt_in_handler - double the option/selection to compare with
    #                            menu item's attribute 'how_to_enter'
    #                            (e.g. user input 'gg' to go to motherboard
    #                             submenu , python check if 'gg' is a valid
    #                             input)
    #           main_menu_dict_in_hdlr - current menu dictionary
    #           flag_for_handler - set flag to non-zero value to bypass capital
    #                              checking mechanism when do_all and do_grp
    #           flag_for_func - reserved extension flag
    #
    #  Outputs : RET_SHOW_MENU     - show current menu again
    #            RET_INVALID_INPUT - user input doesn't have related usage
    #            RET_NO_EXEC       - there are no related executalbe
    #            RET_TRUE          - run function successfully due to input
    #            RET_FALSE         - run function failed due to input
    #
    #--------------------------------------------------------------------------
    def run_handler(self, input_in_hdlr, opt_in_handler,\
    main_menu_dict_in_hdlr, flag_for_handler, flag_for_func) :
        test_name = None
        test_cmd = None
        mitem_name = '' # menu item name in body part

        if (flag_for_handler == 0) and (len(input_in_hdlr) == 1) :
            # toggle Diag flag, bypass if do_all or do_grp
            # ord(65)=A, ord(90)=Z
            if (65 <= ord(input_in_hdlr) and ord(input_in_hdlr) <= 90) :
                ret = self.toggle_flag(input_in_hdlr)
                for key,value in (self.flag.iteritems()) :
                    if (self.flag[key][1] == MACRO.ON_FLAG) :
                        self.flag_value = self.flag_value | self.flag[key][2]

                if (ret == RET_TRUE) :
                    return RET_SHOW_MENU
                else :
                    return RET_INVALID_INPUT

            for key,value in (self.flag.iteritems()) :
                if (self.flag[key][1] == MACRO.ON_FLAG) :
                    self.flag_value = self.flag_value | self.flag[key][2]
        if (self.member != collections.OrderedDict()) :
            user_selected_index = trim_double_chars(opt_in_handler)
            for  key,value  in main_menu_dict_in_hdlr.iteritems() :
                if (value[1] == user_selected_index) :
                    test_name = key[0]
                    test_cmd = value[2]

            if ((test_name == None) or (test_cmd == None)) :
                return RET_INVALID_INPUT

            ret = self.run_handler_function(test_name,test_cmd,\
            main_menu_dict_in_hdlr,input_in_hdlr,flag_for_handler,flag_for_func)
            if (RET_FALSE == ret) :
                return RET_FALSE
            if (RET_NO_EXEC == ret) :
                return RET_NO_EXEC
        else :
            print 'main dict is empty\n'
            return RET_FALSE

        self.submenu_dict = collections.OrderedDict()
        get_sub_memu_dict_for_do_grp = collections.OrderedDict()
        for key,value in (self.member.iteritems()) :
            if (value[3] == main_menu_dict_in_hdlr.values()[0][0] and \
            value[4] == opt_in_handler and (DO_ALL_FLAG in value[7])) :
                self.submenu_dict[key] = value

        # mitem has submenu
        if (self.submenu_dict != collections.OrderedDict()) :
            # get submenu dict
            tmp_submenu_dict = self.submenu_dict
            for key,value in (self.submenu_dict.iteritems()) :
                # Stop on error flag
                if (self.flag[MACRO.S_FLAG][1] == MACRO.ON_FLAG and \
                var.stoponerr_flag > 0) :
                    print "break"
                    break
                mitem_name = value[1]
                if (len(mitem_name) == 2) :
                    # support 'Xa'~'Xz' mitem_letter
                    how_to_enter = mitem_name + mitem_name[1]
                else :
                    how_to_enter = mitem_name * 2
                ret = self.run_handler(value[1], how_to_enter, \
                tmp_submenu_dict, 1, 0)

                if (RET_FALSE == ret) :
                    return RET_FALSE
                if (RET_NO_EXEC == ret) :
                    return RET_NO_EXEC
        else :
            # mitem has no submenu
            # no executable or submenu in $product.pcfg
            if (test_cmd == MACRO.ZERO_FLAG) :
                print "&&& %s item"%(test_name),
                if (self.slot_id == '') :
                    print "has",
                else :
                    print "in slot %s has"%(self.slot_id),
                print "no submenu and not associated with any executable &&&"


    #--------------------------------------------------------------------------
    #
    #  Function : run_handler_function
    #
    #  Description : this function do loop control for do all, do grp, and
    #                permutation test
    #
    #  Inputs : test_name - menu item name in pcfg
    #           test_cmd - menu item executable with arguments in pcfg
    #           main_menu_from_hdlr - current menu dictionary
    #           input_in_hdlr - single character input through prompt
    #           flag_for_handler - set flag to non-zero value to bypass capital
    #                              checking mechanism when do_all and do_grp
    #           flag_for_func - reserved extension flag
    #
    #  Outputs : RET_NO_EXEC - there are no related executalbe
    #            RET_TRUE    - run function successfully due to input
    #            RET_FALSE   - run function failed due to input
    #
    #--------------------------------------------------------------------------
    def run_handler_function(self,test_name,test_cmd, \
    main_menu_from_hdlr,input_in_hdlr,flag_for_handler,flag_for_func) :
        global g_con_intr
        mitem_name = '' # menu item name in body part

        # shell script or C executable
        exec_file = ''
        line = ''
        items = ''

        # only split first item of test_cmd
        items = [item.strip() for item in test_cmd.split(' ', 1)]
        exec_flag = ''.join(items)

        # ord(65)=A, ord(90)=Z
        if ( len(exec_flag) == 1 and 65 <= ord(exec_flag) and \
        ord(exec_flag) <= 90) :
            # toggle flag in alter diag flags menu
            ret = self.toggle_flag(exec_flag)
            for key,value in (self.flag.iteritems()) :
                if (self.flag[key][1] == MACRO.ON_FLAG) :
                    self.flag_value = self.flag_value | self.flag[key][2]

            if (MACRO.C_FLAG in self.flag.keys()) and \
            (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG) :
                pass
            else :
                return RET_FALSE

            if (ret == RET_TRUE) :
                return RET_TRUE
            else :
                return RET_FALSE

        elif (exec_flag == DO_ALL_CMD or exec_flag == DO_GRP_CMD) :
            permutation_test_dict = collections.OrderedDict()

            # get main menu in handler with do all flag
            if (exec_flag == DO_ALL_CMD) :
                item_with_doall_dict = collections.OrderedDict()

                # permutation test flag on
                if (MACRO.U_FLAG in self.flag.keys() and \
                self.flag[MACRO.U_FLAG][1] == MACRO.ON_FLAG) :

                    for key,value in (main_menu_from_hdlr.iteritems()) :
                        if (DO_ALL_FLAG in value[7]) :
                            # value[7]:menu item flag
                            item_with_doall_dict[key] = value

                    for key,value in (item_with_doall_dict.iteritems()) :
                        permutation_test_dict = collections.OrderedDict()
                        # store 1level test item to dict
                        permutation_test_dict[key] = value
                        permutation_test_value = value

                        for key,value in (item_with_doall_dict.iteritems()) :
                            # 1level test item, e.g. "a"a, "a"b, "a"c
                            if (len(permutation_test_value[1]) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = permutation_test_value[1] + \
                                permutation_test_value[1][1]
                            else :
                                how_to_enter = permutation_test_value[1] * 2

                            if (RET_FALSE == self.run_handler\
                            (permutation_test_value[1], how_to_enter,\
                            permutation_test_dict,1,1)) :
                                if (not((MACRO.C_FLAG in self.flag.keys())\
                                and (self.flag[MACRO.C_FLAG][1] == \
                                MACRO.ON_FLAG))) :
                                    menu_pr_err_accum()
                                return RET_FALSE
                            if (self.flag[MACRO.S_FLAG][1] == \
                            MACRO.ON_FLAG) and (var.stoponerr_flag > 0) :
                                break

                            # 2level test item, e.g. a"a", a"b", a"c"
                            mitem_name = value[1]
                            if (len(mitem_name) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = mitem_name + mitem_name[1]
                            else :
                                how_to_enter = mitem_name * 2

                            if (RET_FALSE == self.run_handler\
                            (value[1],how_to_enter,item_with_doall_dict,1,1)) :
                                if (not((MACRO.C_FLAG in self.flag.keys())\
                                and (self.flag[MACRO.C_FLAG][1] == \
                                MACRO.ON_FLAG))) :
                                    menu_pr_err_accum()
                                return RET_FALSE

                            if (self.flag[MACRO.S_FLAG][1] == \
                            MACRO.ON_FLAG) and (var.stoponerr_flag > 0) :
                                break
                else :
                    # normal do all
                    for key,value in (main_menu_from_hdlr.iteritems()) :
                        if (DO_ALL_FLAG in value[7]) :
                            item_with_doall_dict[key] = value
                            mitem_name = value[1]
                            if (len(mitem_name) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = mitem_name + mitem_name[1]
                            else :
                                how_to_enter = mitem_name * 2

                            if (RET_FALSE == self.run_handler\
                            (value[1],how_to_enter,item_with_doall_dict,1,1)) :
                                if (not((MACRO.C_FLAG in self.flag.keys())\
                                and (self.flag[MACRO.C_FLAG][1] == \
                                MACRO.ON_FLAG))) :
                                    menu_pr_err_accum()
                                return RET_FALSE
                            if (self.flag[MACRO.S_FLAG][1] == MACRO.\
                            ON_FLAG) and (var.stoponerr_flag > 0) :
                                break
                if (not((MACRO.C_FLAG in self.flag.keys()) and \
                (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG))) :
                    menu_pr_err_accum()

            elif (exec_flag == DO_GRP_CMD) :
                ret = []
                if (self.c_cnt == 0) :
                    # get main menu in handler for do grp
                    if (MACRO.C_FLAG in self.flag.keys()) and \
                    (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG) :
                        self.c_cnt = 1
                    item_with_dogrp_dict = collections.OrderedDict()
                    do_grp_opt_list = []

                    for key,value in (main_menu_from_hdlr.iteritems()) :
                        if (DO_ALL_FLAG in value[7]) :
                            item_with_dogrp_dict[key] = value

                    count_for_value = 0

                    for key,value in (item_with_dogrp_dict.iteritems()) :
                        if (count_for_value == 0) :
                            print '\n\n====DOGRP==',value[5],'====\n'
                            count_for_value = count_for_value + 1
                        if (value[6] in var.menu_keyname_list) or \
                        (value[6] == '$'+key[0]) :
                            print '', value[1], ":",  key[0], "*"
                        else :
                            print '', value[1], ":",  key[0]

                        do_grp_opt_list.append(value[1])

                    self.ret_user_input = self.query_user_for_do_grp\
                    (do_grp_opt_list)
                    if (self.ret_user_input == RET_FALSE) :
                        return RET_FALSE

                    for loop_cnt in range(0,len(self.ret_user_input),1) :
                        for key,value in (item_with_dogrp_dict.iteritems()) :
                            if (value[1] == self.ret_user_input\
                            [self.usr_input_list_cnt]) :
                                # store user's input to dict
                                self.usr_input_dogrp_dict[key] = value

                        self.usr_input_list_cnt = self.usr_input_list_cnt + 1
                    self.usr_input_list_cnt = 0

                # permutation test flag on
                if (MACRO.U_FLAG in self.flag.keys()) and \
                (self.flag[MACRO.U_FLAG][1] == MACRO.ON_FLAG) :

                    for key,value in self.usr_input_dogrp_dict.iteritems() :
                        permutation_test_dict = collections.OrderedDict()
                        # store 1level test item to dict
                        permutation_test_dict[key] = value
                        permutation_test_value = value

                        for key,value in self.usr_input_dogrp_dict.\
                        iteritems() :
                            # 1level test item, e.g. "a"a, "a"b, "a"c
                            if (len(permutation_test_value[1]) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = permutation_test_value[1] + \
                                permutation_test_value[1][1]
                            else :
                                how_to_enter = permutation_test_value[1] * 2

                            if (RET_FALSE == self.run_handler\
                            (permutation_test_value[1], how_to_enter,\
                            permutation_test_dict,1,1)) :
                                if (not((MACRO.C_FLAG in self.flag.keys())\
                                and (self.flag[MACRO.C_FLAG][1] == \
                                MACRO.ON_FLAG))) :
                                    menu_pr_err_accum()
                                return RET_FALSE
                            if (self.flag[MACRO.S_FLAG][1] == MACRO.\
                            ON_FLAG) and (var.stoponerr_flag > 0) :
                                break

                            # 2level test item, e.g. a"a", a"b", a"c"
                            mitem_name = value[1]
                            if (len(mitem_name) == 2) :
                                # support 'Xa'~'Xz' mitem_letter
                                how_to_enter = mitem_name + mitem_name[1]
                            else :
                                how_to_enter = mitem_name * 2
                            ret = self.run_handler (value[1],how_to_enter,\
                            self.usr_input_dogrp_dict,1,1)
                            if (RET_FALSE == ret) :
                                if (not((MACRO.C_FLAG in self.flag.keys())\
                                and (self.flag[MACRO.C_FLAG][1] == \
                                MACRO.ON_FLAG))) :
                                    menu_pr_err_accum()
                                return RET_FALSE
                            elif (RET_NO_EXEC == ret) :
                                continue

                            # Stop on error flag
                            if (self.flag[MACRO.S_FLAG][1] == MACRO.\
                            ON_FLAG) and (var.stoponerr_flag > 0) :
                                break
                else :
                    # normal do group
                    for key,value in self.usr_input_dogrp_dict.iteritems() :
                        mitem_name = value[1]
                        if (len(mitem_name) == 2) :
                            # support 'Xa'~'Xz' mitem_letter
                            how_to_enter = mitem_name + mitem_name[1]
                        else :
                            how_to_enter = mitem_name * 2

                        ret = self.run_handler\
                        (value[1],how_to_enter,self.usr_input_dogrp_dict,1,1)
                        if (RET_FALSE == ret) :
                            if (not((MACRO.C_FLAG in self.flag.keys())\
                            and (self.flag[MACRO.C_FLAG][1] == \
                            MACRO.ON_FLAG))) :
                                menu_pr_err_accum()
                            return RET_FALSE
                        elif (RET_NO_EXEC == ret) :
                            continue

                        if (self.flag[MACRO.S_FLAG][1] == MACRO.ON_FLAG)\
                        and (var.stoponerr_flag > 0) :
                            break

                if (not((MACRO.C_FLAG in self.flag.keys()) and \
                (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG))) :
                    menu_pr_err_accum()
        else :
            ret = self.run_executable (test_cmd,input_in_hdlr,flag_for_func)
            if (RET_FALSE == ret or var.stoponerr_flag == 1) :
                return RET_FALSE
            elif (RET_NO_EXEC == ret) :
                return RET_NO_EXEC


    #--------------------------------------------------------------------------
    #
    #  Function : run_executable
    #
    #  Description : this function executes menu item
    #
    #  Inputs : test_cmd - menu item executable with arguments in pcfg
    #           input_in_hdlr - single character input through prompt
    #           flag_for_func - reserved extension flag
    #
    #  Outputs : RET_TRUE    - run executable successfully due to input
    #            RET_FALSE   - run executable failed due to input
    #
    #--------------------------------------------------------------------------
    def run_executable(self,test_cmd,input_in_hdlr,flag_for_func) :
        global g_con_intr
        flag_changed_list = []
        exec_line = ''
        exec_file = ''
        line = ''
        items = ''

        # only split first item of test_cmd
        items = [item.strip() for item in test_cmd.split(' ',1)]

        exec_flag = ''.join(items)
        try :
            if (items[0] != MACRO.ZERO_FLAG) :
                # item has EXEC
                line = find_file(var.path_exec_path,items[0] )
            else :
                return RET_TRUE

            if (line == MACRO.RET_INT) :
                raise KeyboardInterrupt
            elif (line == '') :
                print "!!!_CAN'T find the executable %s" % (items[0])
                sys.exit()

            if (len(items) > 1) :
                # item has arguments
                exec_file = line + ' ' + ''.join(items[1])
            else :
                exec_file = line

        except KeyboardInterrupt :
            raise

        for key,value in self.default_flag_dict.iteritems():
            if self.default_flag_dict[key] != self.flag[key] :
                flag_changed_list.append(key)

        # one big image mode
        if (var.img_mode_str == ONE_IMAGE_MODE):
            # passing all flag
            if (var.prod_type_str == ROUTING_TYPE):
                if (''.join(flag_changed_list) != "") :
                    exec_file = exec_file + ' -' + ''.join(flag_changed_list)
        # individual image mode
        else :
            # add diag_flag to the last argument, pass_counter to second-last
            exec_file = exec_file + ' ' + str(self.pass_counter) + ' ' + \
            str(self.flag_value)

        try :
            # exec C through subshell
            ret_sub = subprocess.call(exec_file,shell=True)
            # returncode fall between 0~255
            if (ret_sub != 0) :
                print  "exception in exec, returncode=%s"%(ret_sub)
        except KeyboardInterrupt :
            # if user enter ctrl+c in continue mode
            g_con_intr = 1

        self.update_main_dict(input_in_hdlr)

        if (MACRO.S_FLAG in self.flag.keys()) and \
        (self.flag[MACRO.S_FLAG][1] == MACRO.ON_FLAG) :
            # stop on err when executing different EXEC
            if (os.path.exists(MACRO.SRC_PATH + MACRO.STOPONERR_FILE)) :
               print "Test stopped on error"
               return RET_FALSE

        if (g_con_intr == 1) :
            g_con_intr = 0
            # if user enter ctrl+c in continue mode
            if (not((MACRO.C_FLAG in self.flag.keys()) and \
            (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG))) :
                menu_pr_err_accum()
            raise

    #--------------------------------------------------------------------------
    #
    #  Function : update_main_dict
    #
    #  Description : this function updates module menu under main menu if
    #                module_id.tmp existed
    #
    #  Inputs : input_in_hdlr - single character input through prompt
    #
    #  Outputs : None
    #
    #--------------------------------------------------------------------------
    def update_main_dict(self,input_in_hdlr) :
        global g_diag_py_path_dict
        mod_update_flag = 1 # updating module menu to g_menu_dict
        pre_mod_id = 0  # store previous moduld menu id
        pre_mod_com_submenu_id = 0 # store preovious mod side common submenu id
        mod_symbol = '' # symbol for module
        user_selected_index = ''
        module_table = collections.OrderedDict()
        module_path = ''
        from_where = ''
        how_to_enter = ''
        menu_title = ''
        unused_dict_list = [] # store unused dict id for removing

        user_selected_index = input_in_hdlr

        if (os.path.exists(MACRO.SRC_PATH + SLOT_ID_FILE)) :
            fp = open(MACRO.SRC_PATH + SLOT_ID_FILE,'r')
            self.slot_id = fp.readline()
            fp.close()

        if (os.path.exists(MOD_ID_PATH)) :
            fp = open(MOD_ID_PATH,'r')
            self.module_id = fp.readline()
            print "find module_id = %s in module_id.tmp" % (self.module_id)
            fp.close()
            os.remove(MOD_ID_PATH)

            if (self.module_id not in g_diag_py_path_dict.keys()) :
                print "module id is not in $product_diag_export.txt"
            else :
                for item in g_diag_py_path_dict.keys() :
                    if (self.module_id in item) :
                        # module pcfg
                        module_path = ''.join(g_diag_py_path_dict[item])
                        # list to string
                        if (module_path == '') :
                            print "can't find module pcfg path "\
                            "in $product_diag_export.txt"
                            raise
                        var.module_dict = collections.OrderedDict()
                        pre_mod_com_submenu_id = var.com_submenu_mod_menu_id
                        pre_mod_id = var.mod_menu_start_id
                        module_table = parse_module_file(module_path)

                        if (module_table == collections.OrderedDict()) :
                            print "module_table table is empty!"
                            print "please check $product_diag_export.txt"
                            raise
                print ""
                for key,value in (self.member.iteritems()) :
                    # value[6] = submenu display,
                    if (value[6] == '$' + key[0]) and \
                    (value[1] == user_selected_index) :
                        mod_symbol = key[1]

                if (mod_symbol == '') :
                    print "module menu can't be added! please check if "\
                    "menu item name is equal to submenu name in $product.pcfg"
                    raise
                for key,value in (module_table.iteritems()) :
                    # value[3] = from where, value[4] = how to enter
                    if (value[3] == '' and value[4] == '') :
                        value[3] = mod_symbol

                        if (len(user_selected_index) == 2) :
                            # support 'Xa'~'Xz' mitem_letter
                            value[4] = user_selected_index + \
                            user_selected_index[1]
                        else :
                            value[4] = user_selected_index * 2

                        from_where = value[3]
                        how_to_enter = value[4]
                        menu_title = value[5]

                for key,value in (self.member.iteritems()) :
                    if (from_where == value[3]) and (how_to_enter == value[4])\
                    and (menu_title == value[5]):
                        # check if mod menu has existed in g_menu_dict or not,
                        # if yes, no need to update same menu again
                        mod_update_flag = 0

                # update module_table to g_menu_dict
                if (mod_update_flag == 1) :
                    var.mod_menu_start_id = var.mod_menu_start_id + 1

                    for key,value in (self.member.iteritems()) :
                        if (from_where == value[3]) and \
                        (how_to_enter == value[4]):
                            # remove previous same module menu
                            del self.member[key]
                        else :
                            if (key[1] not in unused_dict_list) :
                                unused_dict_list.append(key[1])

                    for key,value in (self.member.iteritems()) :
                        if (value[3] not in unused_dict_list) and \
                        (value[3] != '') :
                            # remove unused dict
                            if (key[1] in unused_dict_list) :
                                unused_dict_list.remove(key[1])
                            del self.member[key]

                    self.member.update(module_table)
                else :
                    var.mod_menu_start_id = pre_mod_id
                    var.com_submenu_mod_menu_id = pre_mod_com_submenu_id

    #--------------------------------------------------------------------------
    #
    #  Function : query_user_for_do_grp
    #
    #  Description : this function records user's selection in order
    #                for do group
    #
    #  Inputs : do_grp_opt_list - current menu items with DOALL flag
    #
    #  Outputs : RET_FALSE - if input is empty or no space between each item
    #            items - retrun input items in list
    #
    #--------------------------------------------------------------------------
    def query_user_for_do_grp(self , do_grp_opt_list) :
        query_string = 'enter the menu items to execute\n(in test order)[]'
        self.selected_input = ''
        self.selected_input = raw_input(query_string).strip()
        print_msg_once = 0

        # input is enter only
        if (self.selected_input == '') :
            return RET_FALSE

        try :
            items = [item for item in self.selected_input.split(' ')]
        except :
            return RET_FALSE

        for item in items :
            if (item not in do_grp_opt_list) and ( print_msg_once == 0 ) :
                print "illegal menu item..., please try again"
                print "make sure one space between each typed menu item"
                return RET_FALSE
        return items

    #--------------------------------------------------------------------------
    #
    #  Function : query_user
    #
    #  Description : this function shows Diag menu and prompt to query user
    #
    #  Inputs : None
    #
    #  Outputs : RET_TRUE - if valid input, print next menu and return true
    #            RET_QUIT - if user input esc and enter, print upper menu
    #            RET_INVALID_INPUT - if invalid input, print current menu again
    #
    #--------------------------------------------------------------------------
    def query_user(self) :
        global g_kb_intr
        self.selected_input = ''
        ret_exec = None
        self.pass_counter = 0
        self.flag_value = 0
        self.usr_input_dogrp_dict = collections.OrderedDict()
        self.c_cnt = 0
        self.ret_user_input = []
        self.usr_input_list_cnt = 0

        # remove tmp when show menu
        del_file_list = glob.glob(TMP_FILE)
        for del_file in del_file_list:
            if (del_file != SLOT_ID_FILE) :
                os.remove(del_file)

        ret = self.print_menu()
        g_kb_intr = 0
        self.menu_outside = 0
        var.stoponerr_flag = 0
        if (ret == RET_INVALID_INPUT or ret == RET_FALSE or \
        ret == RET_NO_SUBMENU) :
            if (ret != RET_NO_SUBMENU) :
                print_error_input()
            # chr(ASCII_RS) = ^^, pseudo input to go to error input process
            self.selected_input = chr(ASCII_RS)
            self.print_menu_again = 1

        else :
            self.show_flag()
            self.selected_input = \
            raw_input('enter ' + self.query_string + ' > ')

        # input is enter only
        if (self.selected_input == '') :
            self.print_menu_again = 1
            return RET_TRUE

        if (self.selected_input == PATH_STR) :
            self.print_menu_again = 1
            print "main_menu -> " + ' -> '.join(self.path_key_list)
            return RET_TRUE

        if (self.selected_input == ERR_LOG_STR) :
            self.print_menu_again = 1
            os.system(CAT_CMD + ERR_LOG_PATH)
            return RET_TRUE

        if (self.selected_input == CLR_ERR_STR) :
            self.print_menu_again = 1
            os.system(RM_CMD + ERR_LOG_PATH)
            return RET_TRUE

        if (self.selected_input == SHOW_DBG_STR) :
            self.print_menu_again = 1
            os.system(CAT_CMD + SHOW_DEBUG_PATH)
            return RET_TRUE

        if (self.selected_input == CLR_DBG_STR) :
            self.print_menu_again = 1
            os.system(RM_CMD + SHOW_DEBUG_PATH)
            return RET_TRUE

        try :
            if (ord(self.selected_input) == ASCII_ESC or \
            ord(self.selected_input) == ASCII_RS) :
                # ascii code of esc is 27
                if (self.last_menu.values()[0][3] > 0) :
                    self.update_state(RET_GO_BACK_TO_UPPER_MENU)
                else :
                    self.update_state(RET_QUIT)
                return RET_QUIT
        except :
            pass

        # manipulate input to temp value
        if (len(self.selected_input) == 0) :
            self.print_menu_again = 1
            return RET_TRUE
        elif (len(self.selected_input) == 1) :
            # now single letter input supported A~Z and a~z
            temp_selected_input = self.selected_input * 2
            # the table use doubled char
            self.task = RET_INVOKE_TEST

        elif (len(self.selected_input) == 2) :
            if (self.selected_input[0].isdigit()) :
                # support 'Xa'~'Xz' mitem_letter
                temp_selected_input = self.selected_input + \
                self.selected_input[1]
                self.task = RET_INVOKE_TEST
            else :
                temp_selected_input = self.selected_input
                self.task = RET_SHOW_MENU

                # if vm/sm/wic, run handler once to generate submenu by mod id
                user_selected_index = trim_double_chars(self.selected_input)
                for key,value in (self.get_main_menu_dict.iteritems()) :
                    # value[6] = submenu display,
                    if (value[6] == '$' + key[0] and \
                    value[1] == user_selected_index) :

                        for key,value in self.get_main_menu_dict.iteritems() :
                            # designated dict according to symbol
                            if (value[1] == user_selected_index) :
                                test_cmd = value[2]

                        if (test_cmd == None) :
                            print "CAN'T find executable"
                            return RET_INVALID_INPUT
                        self.menu_outside = 1
                        self.run_executable(test_cmd,user_selected_index,0)

        elif ((len(self.selected_input) == 3) and (self.selected_input[0]\
        .isdigit()) and (self.selected_input[1] == self.selected_input[2])) :
            # support 'Xa'~'Xz' mitem_letter
            temp_selected_input = self.selected_input
            self.task = RET_SHOW_MENU

            # if vm/sm/wic, run handler once to generate submenu by module id
            user_selected_index = trim_double_chars(self.selected_input)
            for key,value in (self.member.iteritems()) :
                # value[6] = submenu display,
                if (value[6] == '$' + key[0] and \
                value[1] == user_selected_index) :

                    for key,value in self.get_main_menu_dict.iteritems() :
                        # designated dict according to symbol
                        if (value[1] == user_selected_index) :
                            test_cmd = value[2]

                    if (test_cmd == None) :
                        print "CAN'T find executable"
                        return RET_INVALID_INPUT
                    self.menu_outside = 1
                    self.run_executable(test_cmd,user_selected_index,0)
        else :
            print_error_input()
            self.print_menu_again = 1
            return RET_TRUE
        self.update_state(self.task, temp_selected_input)
        if (self.task == RET_INVOKE_TEST) :
            sel_in = self.selected_input
            if (MACRO.C_FLAG in self.flag.keys()) and \
            (self.flag[MACRO.C_FLAG][1] == MACRO.ON_FLAG) :
                self.pass_counter = 0

                while 1 :
                    # Stop on error flag
                    if (MACRO.S_FLAG in self.flag.keys()) and \
                    (self.flag[MACRO.S_FLAG][1] == MACRO.ON_FLAG) and \
                    (var.stoponerr_flag > 0) :
                        break
                    self.pass_counter = self.pass_counter + 1
                    self.selected_input = sel_in
                    ret_exec = self.run_handler\
                    (self.selected_input,self.opt,self.get_main_menu_dict,0,0)

                    if (ret_exec == RET_SHOW_MENU) or \
                    (ret_exec == RET_INVALID_INPUT) or (ret_exec == RET_FALSE) :
                        break
            else :
                ret_exec = self.run_handler\
                (self.selected_input,self.opt,self.get_main_menu_dict,0,0)

            if (ret_exec == RET_FALSE) :
                print "handler terminated"

            if (ret_exec == RET_INVALID_INPUT) :
                print_error_input()

            self.print_menu_again = 1
            var.stoponerr_flag = 0

        else :
            ret = self.query_user()
            if (ret == RET_QUIT) :
                pass
        return RET_TRUE

#------------------------------------------------------------------------------
#
#  Function : register_menu
#
#  Description : this function register Diag menu to menu class(e.g. diag_menu)
#                from platform pcfg(e.g. o2.pcfg)
#
#  Inputs : d_menu - main menu class(e.g. diag_menu)
#
#  Outputs : RET_TRUE - if menu registered successfully, return true
#            RET_FALSE - if pcfg path can't be found, return false
#
#------------------------------------------------------------------------------
def register_menu(d_menu) :
    global g_usr_input_plat_path
    menu_table = collections.OrderedDict()
    menu_table = parse_menu_file(g_usr_input_plat_path)

    if (menu_table == collections.OrderedDict()) :
        print "py menu table is empty!"
        return RET_FALSE

    d_menu.add_menu(menu_table)
    d_menu.load_flags()

    return RET_TRUE

#------------------------------------------------------------------------------
#
#  Function : diag_main
#
#  Description : this function is the main entry of python menu, it parse diag
#                export file(e.g. o2_diag_export.txt) to get related pcfg path
#                and run python diag menu
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
def diag_main() :
    global g_usr_input_plat_path
    global g_kb_intr
    global g_diag_py_path_dict
    global g_export_file_path

    line = ''
    exec_file = ''

    thread.start_new_thread(notifier_for_stoponerr, ())

    try :
        if (len(sys.argv) > 1) :
            g_export_file_path = str(sys.argv[1])

        g_diag_py_path_dict = parse_path_file(g_export_file_path)

        if (g_diag_py_path_dict == None) :
            print "please check $product_diag_export.txt path"
            raise

        for item in g_diag_py_path_dict.keys() :

            if (item == PLAT_STR) :
                g_usr_input_plat_path = ''.join(g_diag_py_path_dict[item][0])
                # list to string

            if (item == WEL_STR) :
                var.welcome_str = ''.join(g_diag_py_path_dict[item])

            if (item == MACRO.BASIC_STR) :
                common_table_full_path = ''.join(g_diag_py_path_dict[item])

                if (common_table_full_path == '') :
                    print "please check basic.pcfg path"
                    raise
                if (os.path.exists(''.join(common_table_full_path))) :
                    fp = open(''.join(common_table_full_path), 'r')
                else :
                    print 'open file %s failed' % common_table_full_path
                    raise

                for line in fp.readlines() :
                    line = line.strip()
                    # ignore lines started with '#' and empty ones
                    if not len(line) or line.startswith('#') :
                        continue

                    if (line.startswith('$')) :
                        items = [item.strip() for item in line.split(':')]
                        # there is only one menu_keyname in basic.pcfg
                        var.basic_menu_keyname = items[0]
                fp.close()

            if (item == MACRO.FLAG_STR) :
                common_table_full_path = ''.join(g_diag_py_path_dict[item])

                if (common_table_full_path == '') :
                    print "please check diag_flag.pcfg path"
                    raise
                if (os.path.exists(''.join(common_table_full_path))) :
                    fp = open(''.join(common_table_full_path), 'r')
                else :
                    print 'open file %s failed' % common_table_full_path
                    raise

                for line in fp.readlines() :
                    line = line.strip()
                    # ignore lines started with '#' and empty ones
                    if not len(line) or line.startswith('#') :
                        continue

                    if (line.startswith('$')) :
                        items = [item.strip() for item in line.split(':')]
                        # there is only one menu_keyname in diag_flag.pcfg
                        var.flag_menu_keyname = items[0]
                fp.close()

        d_menu = diag_menu()
        ret = register_menu(d_menu)

        if (ret != RET_TRUE) :
            print "register menu fail"
            raise

        for item in g_diag_py_path_dict.keys() :
            # if $product.pcfg followed by an "optional" init EXEC in
            # $product_diag_export.txt
            try :
                if (len(g_diag_py_path_dict[item]) == 2) :
                    path = find_file\
                    (var.path_exec_path,g_diag_py_path_dict[item][1])
                    if (path == '') :
                        print "CAN'T find the platform init file!"
                        raise
                    exec_file = ''.join(path)
                    os.system(exec_file)
            except :
                print "please check init file in $diag_export.txt"
                pass

        print_welcome_msg()

        while 1:
            try :
                ret = d_menu.query_user()
                if (ret == RET_QUIT and d_menu.state == STATE_IS_INIT) :
                    print 'Goodbye.. Have a Nice Day!'
                    break
            except KeyboardInterrupt :
                # wait 100ms for executable to exit
                sleep(DELAY_TIME)
                g_kb_intr = 1

            except (EOFError, SystemExit) :
                if (os.path.exists(MACRO.SRC_PATH + MACRO.WARNCNT_FILE)) :
                    os.remove(MACRO.SRC_PATH + MACRO.WARNCNT_FILE)
                thread.exit()
                sys.exit()

    except :
        sys.exit()

#------------------------------------------------------------------------------
#
#  Function : notifier_for_stoponerr
#
#  Description : this function observes file modified events in specific path
#                as a notifier
#
#  Inputs : None
#
#  Outputs : None
#
#------------------------------------------------------------------------------
def notifier_for_stoponerr() :
    while 1:
        try :
            notifier.process_events()
            # check_events is blocking
            if (notifier.check_events()) :
                notifier.read_events()
        except :
            notifier.stop()
            break

if __name__ == "__main__":
    # if file is imported outside, __name__ will be filename "diag_manu"
    # remove *.pyc and *.tmp files
    del_file_list = glob.glob(PYC_FILE)
    for del_file in del_file_list:
        os.remove(del_file)
    del_file_list = glob.glob(TMP_FILE)
    for del_file in del_file_list:
        os.remove(del_file)
    # start a WatchManager
    wm = pyinotify.WatchManager()
    # monitor file in modify
    mask = pyinotify.IN_MODIFY
    # register file_mon() as notifier
    notifier = pyinotify.ThreadedNotifier(wm, file_mon())
    # register monitor path
    wdd = wm.add_watch(MACRO.SRC_PATH, mask, rec=False)
    diag_main()
    del_file_list = glob.glob(TMP_FILE)
    for del_file in del_file_list:
        os.remove(del_file)

'''
**********History**********
$Log: diag_menu.py,v $
Revision 1.2  2014/06/03 10:53:28  erwu2
python menu collapsed to main trunk

Revision 1.1.2.16  2014/04/29 11:40:39  erwu2
update python file structure

Revision 1.1.2.15  2014/04/10 06:24:04  erwu2
classify o2 and lebowski executable to obj folder

Revision 1.1.2.14  2014/03/26 03:32:35  erwu2
support image mode and product type

Revision 1.1.2.13  2014/03/07 10:29:39  erwu2
add common submenu comments

Revision 1.1.2.12  2014/02/26 08:43:07  erwu2
1. support basic util submenu. 2. reduce args in parse function

Revision 1.1.2.11  2014/02/20 08:33:18  erwu2
support Xa~Xz mitem_letter

Revision 1.1.2.10  2014/02/17 11:38:24  erwu2
support 1a~1z mitem_letter and global MACRO/var class

Revision 1.1.2.9  2014/02/13 11:17:48  erwu2
improve variable naming

Revision 1.1.2.8  2014/02/10 11:29:06  erwu2
add more comments to files

Revision 1.1.2.6  2014/01/23 09:13:23  erwu2
show slot num to executable

Revision 1.1.2.5  2014/01/21 10:45:19  erwu2
improve executable and submenu column definition

Revision 1.1.2.4  2014/01/20 05:53:35  erwu2
improve executable building

Revision 1.1.2.3  2014/01/16 11:15:54  erwu2
update python files

Revision 1.1.2.2  2013/12/19 10:25:19  erwu2
improve tftp dnld process

Revision 1.1.2.1  2013/12/09 06:20:31  erwu2
python menu for o2 example



$Endlog$
'''

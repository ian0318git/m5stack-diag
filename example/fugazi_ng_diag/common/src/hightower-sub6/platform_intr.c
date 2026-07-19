/* $Id: platform_intr.c,v 1.2 2021/06/02 02:56:24 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_intr.c,v $
 *********************************************************************
 *
 * platform_intr.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <list.h>
#include <time.h>

#include "defs.h"
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "hr_commn_util.h"
#include "platform_intr.h"

struct hr_intr {
    char   name[64]              ;   
    struct hr_intr *parent       ;

    void  *pdata                 ;
    int  (*status )(void *pdata) ;
    int  (*trigger)(void *pdata) ;
    int  (*enable )(void *pdata) ;
    int  (*disable)(void *pdata) ;
    int  (*clear  )(void *pdata) ;

    int    ch_num                ;
    int    ch_size               ;
    struct hr_intr **children    ;
};

static struct hr_intr *hr_intr_root = NULL;

static const char *hr_intr_names[] = {
    "TEMP",
    "CPLD",
    "PHY_WOL",
    NULL,
};
static int root_state = 0;
static int dummy_status (void *pdata) {return 0;}
static int dummy_trigger(void *pdata) {root_state = 1; return 0;}
static int dummy_enable (void *pdata) {return 0;}
static int dummy_disable(void *pdata) {return 0;}
static int dummy_clear  (void *pdata) {root_state = 0; return 0;}

static int _intr_check_name(const char *name)
{
    int i = 0;

    if (!name)
        return -(__LINE__);

    for(i = 0; hr_intr_names[i]; i++) {
        if (strcmp(name, hr_intr_names[i]) == 0)
            return 0;
    }
    return -(__LINE__);
}

static int _intr_trans_down(struct hr_intr *intr, int (*cb[])(struct hr_intr *), int ncb)
{
    int i = 0;
    int j = 0;

    for(j = 0; j < ncb; j++) {
        if (cb[j](intr) < 0)
            return -(__LINE__);
    }

    for(i = 0; i < intr->ch_num; i++) {
        for(j = 0; j < ncb; j++) {
            if (cb[j](intr->children[i]) < 0)
                return -(__LINE__);
        }
    }
    return 0;
}

/*static*/ int _intr_trans_up(struct hr_intr *intr, int (*cb[])(struct hr_intr *), int ncb)
{
    int j = 0;

    for(j = 0; j < ncb; j++) {
        if (cb[j](intr) < 0)
            return -(__LINE__);
    }

    if (intr->parent)
        for(j = 0; j < ncb; j++) {
            if (cb[j](intr->parent) < 0)
                return -(__LINE__);
        }
    return 0;
}

static struct hr_intr *_intr_lookup(struct hr_intr *intr, const char *name)
{
    int i = 0;
    struct hr_intr *p = NULL;

    if (!intr)
        return NULL;

    if (strcmp(intr->name, name) == 0)
        return intr;

    for(i = 0; i < intr->ch_num; i++) {
        p = _intr_lookup(intr->children[i], name);
        if (p)
            return p;
    }
    return NULL;
}

int hr_intr_lookup(struct hr_intr **intr, const char *name)
{
    *intr = _intr_lookup(hr_intr_root, name);
    return *intr ? 0 : -(__LINE__);
}

int hr_intr_set_parent(struct hr_intr *self, struct hr_intr *parent)
{
    ERR_RET_COND(self->parent != NULL, -(__LINE__), "Intr '%s' already has parent '%s'\n",
        self->name, self->parent->name);
    self->parent = parent;
    return 0;
}

int hr_intr_add_child(struct hr_intr *self, struct hr_intr *child)
{
    const int incr = 64;
    struct hr_intr **intrs = NULL;
    ERR_RET_COND(!self || !child, -(__LINE__), "Invalid argument.\n");
    ERR_RET_COND(self->ch_num > self->ch_size, -(__LINE__), "Something must be wrong.\n");

    if (self->ch_num == self->ch_size) {
        intrs = realloc(self->children, (self->ch_num + incr) * sizeof(struct hr_intr *));
        ERR_RET_COND(!intrs, -(__LINE__), "Alloc memory failed.\n");
        self->children = intrs;
        self->ch_size += incr;
    }

    self->children[self->ch_num] = child;
    self->ch_num += 1;

    return 0;
}

int hr_intr_create(struct hr_intr **intr,
    const char *name, void *pdata,
    int (*status )(void *pdata)  ,
    int (*trigger)(void *pdata)  ,
    int (*enable )(void *pdata)  ,
    int (*disable)(void *pdata)  ,
    int (*clear  )(void *pdata))
{
    ERR_RET_COND(!intr || !status || !trigger || !enable || !disable || !clear,
                 -(__LINE__), "Invalid argument.\n");

    ERR_RET_COND(_intr_check_name(name) < 0, -(__LINE__), "Invalid name:%s\n", name);

    ERR_RET_COND(!(*intr = malloc(sizeof(struct hr_intr))), -(__LINE__), "Alloc memory failed.\n");

    memset(*intr, 0, sizeof(**intr));
    strncpy((*intr)->name, name, sizeof((*intr)->name));
    (*intr)->pdata   = pdata  ;
    (*intr)->status  = status ;
    (*intr)->trigger = trigger;
    (*intr)->enable  = enable ;
    (*intr)->disable = disable;
    (*intr)->clear   = clear  ;
    return 0;
}

int hr_intr_remove(struct hr_intr *intr)
{
    return 0;
}

int hr_intr_init(void)
{
    if (hr_intr_root)
        return 0;

    return hr_intr_create(&hr_intr_root, "ROOT", NULL,
                          dummy_status ,
                          dummy_trigger,
                          dummy_enable ,
                          dummy_disable,
                          dummy_clear);
}

int hr_intr_deinit(void)
{
    if (hr_intr_root)
        return hr_intr_remove(hr_intr_root);
    return 0;
}


static int _intr_test_clr(struct hr_intr *intr)
{
    return (!intr) ? -(__LINE__) : intr->clear(intr->pdata);
}

static int _intr_test_dis(struct hr_intr *intr)
{
    return (!intr) ? -(__LINE__) : intr->disable(intr->pdata);
}

static int _intr_test_chk_set(struct hr_intr *intr)
{
    return (!intr) ? -(__LINE__) : (intr->status(intr->pdata) ? 0 : -(__LINE__));
}

static int _intr_test_chk_clr(struct hr_intr *intr)
{
    return (!intr) ? -(__LINE__) : (intr->status(intr->pdata) ? -(__LINE__) : 0);
}

static int _intr_test_one(struct hr_intr *intr)
{
    /*
        3.1 disable and clear all leaf intrs
        3.2 NOP
        3.3 I.trigger()
        3.4 if I.status() == SET, failed
            else goto 3.5
        3.5 I.enable()
        3.6 if I.status() == SET, failed
            else goto 3.7
        3.7 I.trigger()
        3.8 if I.status() != SET, failed
            else goto 3.9
        3.9 check if all parent status set
            Not, failed.
    */

    int ret = 0;
    int (*cb[16])(struct hr_intr *);
    memset(cb, 0, sizeof(cb));

    cb[0] = _intr_test_dis;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Disable all intr failed.\n");

    cb[0] = _intr_test_clr;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Clear all intr failed.\n");

    ERR_RET_COND(intr->trigger(intr->pdata)        < 0             , -(__LINE__) , "Trigger failed.\n");
    ERR_RET_COND((ret = intr->status(intr->pdata)) < 0 || ret == 1 , -(__LINE__) , "Invalid status.\n");
    ERR_RET_COND(intr->enable(intr->pdata)         < 0             , -(__LINE__) , "Enable failed.\n");
    ERR_RET_COND(intr->trigger(intr->pdata)        < 0             , -(__LINE__) , "Trigger failed.\n");
    ERR_RET_COND((ret = intr->status(intr->pdata)) < 0 || ret != 1 , -(__LINE__) , "Invalid status.\n");

    cb[0] = _intr_test_chk_set;
    ERR_RET_COND(_intr_trans_up(intr, cb, 1) < 0, -(__LINE__), "Parents status invalid.\n");

    return 0;
}

int hr_intr_test(void)
{
    /*
        1. disable and clear all leaf intrs
        2. check if all status clear
            Not, failed
            Yes, goto 3
        3. foreach I in all leaf intrs
            3.1 disable and clear all leaf intrs
            3.2 NOP
            3.3 I.trigger()
            3.4 if I.status() == SET, failed
                else goto 3.5
            3.5 I.enable()
            3.6 if I.status() == SET, failed
                else goto 3.7
            3.7 I.trigger()
            3.8 if I.status() != SET, failed
                else goto 3.9
            3.9 check if all parent status set
                Not, failed.
    */

    int (*cb[16])(struct hr_intr *);
    memset(cb, 0, sizeof(cb));

    cb[0] = _intr_test_dis;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Disable all intr failed.\n");

    cb[0] = _intr_test_clr;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Clear all intr failed.\n");

    cb[0] = _intr_test_chk_clr;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Check if all intr cleared failed.\n");

    cb[0] = _intr_test_one;
    ERR_RET_COND(_intr_trans_down(hr_intr_root, cb, 1) < 0, -(__LINE__), "Check if all intr cleared failed.\n");

    return 0;
}

/*********************************************************************
 * $Log: platform_intr.c,v $
 * Revision 1.2  2021/06/02 02:56:24  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.3  2020/12/09 01:52:01  alpeng
 * use C comment
 *
 * Revision 1.1.4.2  2020/11/25 07:35:59  alpeng
 *  clean up #if 0
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endplatform_intr.c$
 */


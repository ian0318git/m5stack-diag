/*------------------------------------------------------------------
 *
 * curie2ru_test.h - Curie2ru test definitions.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_TEST_H__
#define __CURIE2RU_TEST_H__

extern int curie2ru_diag_init(int argc, char *argv[]);
extern void curie2ru_diag_exit(void);

extern int curie2ru_bcm82757_test(int show_menu);
extern int curie2ru_bcm82752_test(int show_menu);

#endif /* __CURIE2RU_TEST_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_test.h,v $
Revision 1.1  2020/01/09 01:01:58  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

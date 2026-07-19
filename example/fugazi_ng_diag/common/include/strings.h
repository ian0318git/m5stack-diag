/* $Id: strings.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/strings.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#ifndef __STRINGS_H__
#define __STRINGS_H__
/*
** strings prototypes
*/
#ifndef LINUX_APP
extern char *strchr(char *sp, char c);
extern char *strcat(char *s1, char *s2);
extern int strcmp(char *s1, char *s2);
extern char *strcpy(char *s1, char *s2);
extern int strlen(char *s);
extern char *strncat(char *s1, char *s2, int n);
extern int strncmp(char *s1, char *s2, int n);
extern char *strncpy(char *s1, char *s2, int n);
extern char *strstr ( char *long_string, char *sub_string); /* find first occurance 
							     *of sub_string in long_string */
extern unsigned long strtoul (const char *s, char **ptr, int base);
extern int strcasecmp (const char *s1, const char *s2);
extern void bzero (void *dummy, int count);
extern int  bcopy(unsigned char *source, unsigned char *destation, int bytes);
#endif  /* linux_app */

#ifndef LINUX_APP
extern char *memcpy(char *dst, const char *src, int nbytes);
extern char *memset (char *ptr, int val, int len);
extern void puts(char *cptr);

#if defined(INTEL_ICC)
#pragma byte_order(push, littleendian)
extern void putchar(char c);
extern char getchar(void);
#pragma byte_order(pop)
#else
extern void putchar(char c);
extern char getchar(void);
#endif
#endif /* LINUX_APP */

#endif /* __STRINGS_H__ */

/******** History ******** 
$Log: strings.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

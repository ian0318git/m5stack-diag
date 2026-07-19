/* $Id: term.h,v 1.2 2012/03/28 00:38:13 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/term.h,v $
 *-----------------------------------------------------------------------------
 * File: term.h
 *
 * March. 2007, mcharon
 *
 * Copyright (c) 2002-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#define ansi                    printf( "\033<") 
#define apshome                 printf( "\033[?6l") 
#define arrowaplic              printf( "\033[?1h") 
#define arrownorm               printf( "\033[?1l") 
#define autorepoff              printf( "\033[?8l") 
#define autorepon               printf( "\033[?8h") 
#define bannerdown              printf( "\033#4") 
#define bannerup                printf( "\033#3") 
#define charinsmode             printf( "\033[4h") 
#define charrepmode             printf( "\033[4l") 
#define clearbol                printf( "\033[1K") 
#define clearbos                printf( "\033[1J") 
#define cleareol                printf( "\033[K") 
#define cleareos                printf( "\033[J") 
#define clearline               printf( "\033[2K") 
#define cleartab                printf( "\033[g") 
#define cleartabs               printf( "\033[3g") 
#define cls                     printf( "\033[2J") 
#define crnonwl                 printf( "\033[20l") 
#define croatian0             printf( "\033(Y") 
#define croatian1             printf( "\033)Y") 
#define crwithnwl               printf( "\033[20h") 
#define curmode(n)              printf( "\033[%dm",n) 
#define default0              printf( "\033(1") 
#define default1              printf( "\033)1") 
#define delchar(n)            printf( "\033[%dP",n) 
#define delline(n)            printf( "\033[%dM",n) 
#define devrequest              printf( "\033[c") 
#define doublwidth              printf( "\033#6") 
#define gotoxy(x,y)           printf( "\033[%d;%dH",x,y) 
#define graphic0                printf( "\033(0") 
#define graphic1                printf( "\033)0") 
#define greek0                  printf( "\033(6") 
#define greek1                  printf( "\033)6") 
#define hardscroll              printf( "\033[?4l") 
#define home                    printf( "\033[H") 
#define ink(n)                 printf( "\033[%1dm",n) 
#define insline(n)            printf( "\033[%dL",n) 
#define keypadaplic             printf( "\033=") 
#define keypadnumer             printf( "\033>") 
#define ledlight(n)           printf( "\033[%d;%dq",n,n) 
#define lockkbrd                printf( "\033[2h") 
#define movedown(n)           printf( "\033[%dB",n) 
#define moveleft(n)           printf( "\033[%dD   ",n) 
#define moveright(n)          printf( "\033[%dC",n) 
#define moveup(n)             printf( "\033[%dA",n) 
#define newline                 printf( "\033E") 
#define normalbg                printf( "\033[?5l") 
#define partner                 printf( "\033[?2h") 
#define position(x,y)         printf( "\033[%d,%df",x,y) 
#define relhome                 printf( "\033[?6h") 
#define repposn                 printf( "\033[6n") 
#define repstat                 printf( "\033[5n") 
#define resetterm               printf( "\033C") 
#define restore                 printf( "\0338") 
#define reversebg               printf( "\033[?5h") 
#define scroll                  printf( "\033[r") 
#define scrolldown              printf( "\033M") 
#define scrollup                printf( "\033D") 
#define scrollwin(x,y)        printf( "\033[%d;%dr",x,y) 
#define set130                  printf( "\033[?3h") 
#define set80                   printf( "\033[?3l") 
#define setG0(c)              printf( "\033(%c",c) 
#define setG1(c)              printf( "\033)%c",c) 
#define settab                  printf( "\033H") 
#define singlwidth              printf( "\033#5") 
#define snglsh2                 printf( "\033N") 
#define snglsh3                 printf( "\033O") 
#define softscroll              printf( "\033[?4h") 
#define store                   printf( "\0337") 
#define ukascii0              printf( "\033(A") 
#define ukascii1              printf( "\033)A") 
#define unlockkbrd              printf( "\033[2l") 
#define usascii0              printf( "\033(B") 
#define usascii1              printf( "\033)B") 
#define vt52                    printf( "\033[?2l") 
#define wrapoff                 printf( "\033[?7l") 
#define wrapon                  printf( "\033[?7h")



#define bel             printf("\007") 
#define esc             printf("\033") 
#define csi             printf("\033[") 
#define lscreen         printf("\033[?5h") 
#define dscreen         printf("\033[?5l") 
#define rev_vid         printf("\033[7m") 
#define blink           printf("\033[5m") 
#define under           printf("\033[4m") 
#define bold            printf("\033[1m") 
#define norm_vid        printf("\033[0m") 
#define wide_vid        printf("\033#6") 
#define high_vid2       printf("\033#4") 
#define high_vid1       printf("\033#3") 
#define graphic         printf("\033(0") 
#define no_graph        printf("\033(B") 
#define no_window       printf("\033[1;24r") 
#define no_att          printf("\033[0;22;24;25;27m") 
#define cup(row,col)    printf("\033[%d;%dH",(row),(col)) 
#define stbm(top,bot)   printf("\033[%d;%dr",(top),(bot)) 
#define ri              printf("\033M") 
#define el              printf("\033[K")
#define ed0             printf("\033[0J")
#define eee             printf("\033[2K") 
#define bar             graphic;printf("x");no_graph 
#define wipe(row,col)   cup(row,col);printf("\033[J") 
#define clr             wipe(0,0);norm_vid 

/*--------------------------------End of file-------------------------------*/
/******** History ********
$Log: term.h,v $
Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

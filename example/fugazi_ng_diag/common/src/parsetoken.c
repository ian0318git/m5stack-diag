/* $Id: parsetoken.c,v 1.2 2012/03/28 00:38:14 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/parsetoken.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Parse a command line token.
** A token can be of several types as defined in SH_TOKEN in the file sh.h.
** It can be a normal word with no special meaning to the shell or a shell
** directive.  Return the type of shell token parsed out of the command
** buffer.  If it is a normal word it is copied into the buffer pointed to
** by the supplied argv buffer pointer.
*/

#include "endians.h"
#include "types.h"
#include "sh.h"
#include "pcmap.h"
#include "nvsysvars.h"
#include "proto.h"

#include <stdio.h>
#include <string.h>
extern void setmore(void);
SH_TOKEN
parsetoken(struct shstuff *shp, SH_STATE state)
{
  register char c;
  register char *cptr;
  register char *startptr = shp->argvptr;  /* record starting point */

  while(1) {
    c = *shp->cmdptr++;  /* get a character from the command buffer */
#ifdef DEBUG
printf("parsetoken: *cmdptr = '%c'   state = ",c);
switch(state) {
 case NEUTRAL: puts("NEUTRAL"); break;
 case GRTRTHAN: puts("GRTRTHAN"); break;
 case INSQUOTE: puts("INSQUOTE"); break;
 case INDQUOTE: puts("INDQUOTE"); break;
 case INBRACES: puts("INBRACES"); break;
 case INSHVAR: puts("INSHVAR"); break;
 case INWORD: puts("INWORD"); break;
 case INCOMMENT: puts("INCOMMENT"); break;
}
putchar('\n');
#endif
    switch(state) {
    case NEUTRAL:  /* the beginning of a shell "word" */
      switch(c) {
      case ';':  /* end of command character */
	return(W_ENDCMD);
      case '&':  /* background command character */
	return(W_BKGRND);
      case '|':  /* pipe character */
	setmore();
	return(parsetoken(shp, INPIPELINE));
      case '<':  /* redirect input character */
	return(W_REDIN);
      case '\0':  /* end of line character */
	return(W_EOLN);
      case ' ':
      case '\t':  /* white space at beginning of word */
	continue;
      case '>':  /* redirect output character */
	return(parsetoken(shp,GRTRTHAN));
      case '"':  /* double quoted string (shell variables replacement) */
	parsetoken(shp,INDQUOTE);
couldbemore:
	state = INWORD;
	continue;
      case '\'':  /* single quoted string (no shell variable replacement) */
	parsetoken(shp,INSQUOTE);
	goto couldbemore;
      case '$':  /* start of shell variable */
	parsetoken(shp,INSHVAR);
	goto couldbemore;
      case '\\':  /* ignore any special meaning of the next character */
	--shp->cmdptr;  /* unget the char */
	return(parsetoken(shp,INWORD));
      case '#':  /* the start of a comment */
	return(parsetoken(shp,INCOMMENT));
      default:
	state = INWORD;
bufchar:
	*shp->argvptr++ = c;  /* put char in word buffer */
	continue;
      }
    case GRTRTHAN:
      if(c == '>') return(W_APPEND);
      --shp->cmdptr;  /* unget the char */
      return(W_REDOUT);
    case INSQUOTE:
      switch(c) {
      case '\'':  /* end of the quote */
termword:  /* null argv slot, reuse */
	if(shp->argvptr == startptr) return(W_VOID);
        *shp->argvptr = '\0';  /* terminate the word */
        return(W_REG);
      case '\0':  /* premature end of line */
	c = '\'';
quoterr:
	printf("monitor: missing closing %c\n",c);
	  longjmp(shp->shparserrjmp,1);
      case '\\':  /* "escape" the special meaning of the next char */ 
escapechar:
	c = *shp->cmdptr++;  /* just buffer this char without checking it */
	break;
      }
      goto bufchar;
    case INDQUOTE:
      switch(c) {
      case '\\':  /* "escape" the special meaning of the next char */ 
	goto escapechar;
      case '"':  /* end of double quoted string */
	goto termword;
      case '$':  /* shell variables get expanded */
	parsetoken(shp,INSHVAR);
	continue;
      case '\0':  /* premature end of line */
	c = '"';
	goto quoterr;
      }
      goto bufchar;
    case INBRACES:
      if(c == '}') goto vardone;
    case INSHVAR:
      if(c == '{' && state == INSHVAR && startptr == shp->argvptr)
	return(parsetoken(shp,INBRACES));  /* first char of INSHVAR */
      if(strchr("\t $#\\'\"",c)) {
	if(state == INBRACES) {
	  printf("monitor: illegal character '%c' in variable name\n",c);
	  longjmp(shp->shparserrjmp,1);
	}
	--shp->cmdptr;  /* unget the char */
vardone:
	*shp->argvptr = '\0';  /* terminate the variable arg */
	if((cptr = getvar((NVRAM)->evartbl,startptr))) {  /* var is valid */
	  strcpy(startptr,cptr);  /* replace var name with var value */
	  shp->argvptr = startptr + strlen(cptr);  /* point to NULL term */
	} else shp->argvptr = startptr;  /* var is void (return(W_VOID)?) */
	return(W_REG);
      }
      goto bufchar;
    case INWORD:
      switch(c) {
      case '\\':  /* "escape" the special meaning of the next char */ 
	goto escapechar;
      case '$':  /* shell variables can be a part of a word */
	parsetoken(shp,INSHVAR);
	continue;
      case '\'':  /* single quoted strings can be part of word */
	parsetoken(shp,INSQUOTE);
	continue;
      case '"':  /* double quoted strings can be part of word */
	parsetoken(shp,INDQUOTE);
	continue;
      default:
	if(strchr(";&|<> \t`#",c)) {  /* word delimiters */
	  --shp->cmdptr;  /* unget the char */
	  goto termword;
	}
	break;
      }
      goto bufchar;

	/*
	** Special hack for a kludgy "more" mechanism
	** Just eat chars to the end of the pipeline command and
	** throw them away.
	** Give some chars special treatment
	*/
    case INPIPELINE:
	if(c == '|') return(W_PIPE);
	if(c == ';') return(W_ENDCMD);
	/* fall through */

    case INCOMMENT:  /* do not buffer comment chars */
      if(c == '\0') return(W_EOLN);
      continue;
    }
  }
}

/******** History ******** 
$Log: parsetoken.c,v $
Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

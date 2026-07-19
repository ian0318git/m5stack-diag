/* $Id: linux_pstat.c,v 1.2 2012/03/28 00:38:14 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_pstat.c,v $
 *------------------------------------------------------------------
 *
 *
 * 5/2008
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/time.h> 
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "defs.h"
#include "signals.h"

static volatile char abort_flag;

void timer_handler (int signum)
{
    struct itimerval timer;

    /* stop timer */
    memset (&timer, 0, sizeof (timer));
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    
    abort_flag = 1;
}

int
pstat(unsigned int *location, int size, int eq, unsigned int mask,
      unsigned int cmpval, unsigned int msecs, void *retptr)

{
    struct sigaction sa;
    struct itimerval timer;

    unsigned long readval, masked_rdval;
    abort_flag = 0;
    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset (&sa, 0, sizeof (sa));
    memset (&timer, 0, sizeof (timer));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_ONESHOT; /* restore signal default after executing
                                 our handler */
    sigaction(SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after msec... */
    if (msecs >= 1000) {
        timer.it_value.tv_sec = msecs/1000;
        timer.it_value.tv_usec = (msecs%1000) * 1000;
    } else {
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = msecs * 1000;
    }
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    //    timer.it_interval.tv_usec = msecs * 1000;
    timer.it_interval.tv_usec = 0;// msecs * 1000;
    /* Start a virtual timer. It counts down whenever this process is
       executing. */
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /* Do busy work. */
    while(1) {
        switch(size) {
        case 1:
	    readval = *(unsigned char *)location;
	    break;
	case 2:
	    readval = *(unsigned short *)location;
	    break;
	default:
	    readval = *location;
	    break;
	}

	masked_rdval = readval & mask;
	if(eq) {
	    if(masked_rdval == cmpval) break;
	} else {
	    if(masked_rdval != cmpval) break;
	}
	if(abort_flag) {
            /* stop timer */
            memset (&timer, 0, sizeof (timer));
            setitimer(ITIMER_VIRTUAL, &timer, NULL);
            return(0);
        }
    }

    /* stop timer */
    memset (&timer, 0, sizeof (timer));
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /* if timer expired, get the value to return */
    if (retptr) {
	switch(size) {
	case 1:
	    *(unsigned char *)retptr = readval;
	    break;
	case 2:
	    *(unsigned short *)retptr = readval;
	    break;
	default:
	    *(unsigned long *)retptr = readval;
	    break;
	}
    }
    return (1);
}
int
pstat_le(unsigned int *location, int size, int eq, unsigned int mask,
      unsigned int cmpval, unsigned int msecs, void *retptr)

{
    return (pstat(location, size, eq, mask,
                  cmpval, msecs, retptr));
}






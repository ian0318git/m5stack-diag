/* $Id: bmcsyslog.h,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/bmcsyslog.h,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <syslog.h>

#ifndef BMCSYSLOG_H
#define BMCSYSLOG_H

#define DAEMON_SYSLOG_INIT(name) \
	openlog(name, LOG_PID | LOG_NDELAY, LOG_DAEMON)

#define SYSLOG_INIT(name) \
	openlog(name, LOG_PID | LOG_NDELAY, LOG_USER)

#define RAW_SYSLOG_INIT(name, facility) \
	openlog(name, LOG_PID | LOG_NDELAY, facility)

#define BMC_SLOG_COMMON(msg) __FILE__ ":%d:" msg, __LINE__
/* 
 * Software or hardware component is going down, there is nothing 
 * that can be done to repair the error.  Component is UNUSUABLE
 */
#define SLOG_FATAL(msg, args...) \
	syslog(LOG_EMERG, BMC_SLOG_COMMON(msg), ##args) 

/* 
 * Software or hardware component problem detected, this event 
 * should not be occurring.  Continued operation potentially degraded,
 * action must be taken immediately
 */
#define SLOG_ALERT(msg, args...) \
	syslog(LOG_ALERT, BMC_SLOG_COMMON(msg), ##args) 

/* 
 * A recoverable but unexpected software component error occurred, 
 * like a file open failing.  
 */
#define SLOG_CRIT(msg, args...) \
	syslog(LOG_CRIT, BMC_SLOG_COMMON(msg), ##args) 

/* 
 * Software or hardware component problem detected, this is an 
 * informational message, like a temp threshold being crossed.  
 * Action to correct being attempted and continued operation is 
 * not threatened.
 */
#define SLOG_ERR(msg, args...) \
	syslog(LOG_ERR, BMC_SLOG_COMMON(msg), ##args) 

/*
 * Software warning indicating out of bounds value or response.
 */
#define SLOG_WARN(msg, args...) \
	syslog(LOG_WARNING, BMC_SLOG_COMMON(msg), ##args) 

#define SLOG_WARN_EX(enable, msg, args...) \
	if (enable) { syslog(LOG_WARNING, BMC_SLOG_COMMON(msg), ##args); } 
/*
 * Normal but significant condition
 */
#define SLOG_NOTICE(msg, args...) \
	syslog(LOG_NOTICE, BMC_SLOG_COMMON(msg), ##args) 

/*
 * Information message - e.g. version and application name
 */
#define SLOG_INFO(msg, args...)

#define SLOG_INFO_EX(enable, msg, args...) \
  if ( 0 != enable ) { syslog(LOG_INFO, BMC_SLOG_COMMON(msg), ##args); }

// Do not over use this!!! I will have to kick butt if you do.
#define SLOG_INFO_REAL(msg, args...) \
	syslog(LOG_INFO, BMC_SLOG_COMMON(msg), ##args) 
/*
 * Debug level messages
 */
#define SLOG_DEBUG(msg, args...) 

#define SLOG_DEBUG_EX(enable, msg, args...) \
  if ( 0 != enable ) { syslog(LOG_DEBUG, BMC_SLOG_COMMON(msg), ##args); }

#define SLOG0(msg, args...) SLOG_FATAL(msg, ##args)
#define SLOG1(msg, args...) SLOG_ALERT(msg, ##args)
#define SLOG2(msg, args...) SLOG_CRIT(msg, ##args)
#define SLOG3(msg, args...) SLOG_ERR(msg, ##args)
#define SLOG4(msg, args...) SLOG_WARN(msg, ##args)
#define SLOG5(msg, args...) SLOG_NOTICE(msg, ##args)
#define SLOG6(msg, args...) SLOG_INFO(msg, ##args)
#define SLOG7(msg, args...) SLOG_DEBUG(msg, ##args)


#endif

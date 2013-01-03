/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*!
 * @file smf_trace.h
 * @brief Logging functions
 */

#ifndef _SMF_TRACE_H
#define _SMF_TRACE_H

/*!
 * @enum SMFTrace_T
 * @brief Possible log levels
 */
typedef enum {
    TRACE_EMERG = 1,
    TRACE_ALERT = 2,
    TRACE_CRIT = 4,
    TRACE_ERR = 8,
    TRACE_WARNING = 16,
    TRACE_NOTICE = 32,
    TRACE_INFO = 64,
    TRACE_DEBUG = 128,
    TRACE_LOOKUP = 256 // Logs at Debug Level
} SMFTrace_T;

/*!
 * @enum SMFTraceDest_T
 * @brief Trace destination
 */
typedef enum {
	TRACE_DEST_SYSLOG,
	TRACE_DEST_STDERR
} SMFTraceDest_T;

/*!
 * @def TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, fmt)
 * @brief Convenience macro for logging
 * @param level loglevel, see trace_t
 * @param fmt format string for log message
 * @param ... format string arguments
 */
#define TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, NULL, fmt)

/*!
 * @def STRACE(level, sid, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, sid, fmt)
 * @brief Log message with session id
 */
#define STRACE(level, sid, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, sid, fmt)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void trace(SMFTrace_T level, const char * module, const char * function, int line, const char *sid, const char *formatstring, ...);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
 * @brief Configures the detail-level of a trace-entry.
 *
 * @param debug If set to 1, then the function and line-number are logged in in addition
 */
void configure_debug(int debug);

/*!
 * @brief Configures the destination, where all the log-data are send to.
 *
 * @param dest The new logging-destination. The default is TRACE_DEST_SYSLOG.
 */
void configure_trace_destination(SMFTraceDest_T dest);

/*!
 * @def TRDEBUG(fmt, ...) TRACE(TRACE_DEBUG, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with debug log level
 */
#define TRDEBUG(fmt, ...) TRACE(TRACE_DEBUG, fmt, ##__VA_ARGS__)

/*!
 * @def TRINFO(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with info log level
 */
#define TRINFO(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)

 /*!
 * @def TRNOTICE(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with notice log level
 */
#define TRNOTICE(fmt, ...) TRACE(TRACE_NOTICE, fmt, ##__VA_ARGS__)

/*!
 * @def TRWARN(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with warning log level
 */
#define TRWARN(fmt, ...) TRACE(TRACE_WARNING, fmt, ##__VA_ARGS__)

/*!
 * @def TRERR(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with error log level
 */ 
#define TRERR(fmt, ...) TRACE(TRACE_ERR, fmt, ##__VA_ARGS__)

/*!
 * @def TRCRIT(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with critical log level
 */ 
#define TRCRIT(fmt, ...) TRACE(TRACE_CRIT, fmt, ##__VA_ARGS__)

/*!
 * @def TRALERT(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with alert log level
 */
#define TRALERT(fmt, ...) TRACE(TRACE_ALERT, fmt, ##__VA_ARGS__)

/*!
 * @def TREMERG(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
 * @brief Shortcut for logging with ermegency log level
 */
#define TREMERG(fmt, ...) TRACE(TRACE_EMERG, fmt, ##__VA_ARGS__)

#endif  /* _SMF_TRACE_H */


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

#ifndef _SMF_TRACE_H
#define _SMF_TRACE_H

/* Logging and debugging stuff */
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
 * @def TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, fmt)
 * @brief convenience macro for logging
 * @param level loglevel, see trace_t
 * @param fmt format string for log message
 * @param ... format string arguments
 */
#define TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, fmt)
void trace(SMFTrace_T level, const char * module, const char * function, int line, const char *formatstring, ...);

void configure_debug(int debug);

#define TRDEBUG(fmt, ...) TRACE(TRACE_DEBUG, fmt, ##__VA_ARGS__)
#define TRINFO(fmt, ...) TRACE(TRACE_INFO, fmt, ##__VA_ARGS__)
#define TRNOTICE(fmt, ...) TRACE(TRACE_NOTICE, fmt, ##__VA_ARGS__)
#define TRWARN(fmt, ...) TRACE(TRACE_WARNING, fmt, ##__VA_ARGS__)
#define TRERR(fmt, ...) TRACE(TRACE_ERR, fmt, ##__VA_ARGS__)
#define TRCRIT(fmt, ...) TRACE(TRACE_CRIT, fmt, ##__VA_ARGS__)
#define TRALERT(fmt, ...) TRACE(TRACE_ALERT, fmt, ##__VA_ARGS__)
#define TREMERG(fmt, ...) TRACE(TRACE_EMERG, fmt, ##__VA_ARGS__)

#endif  /* _SMF_TRACE_H */


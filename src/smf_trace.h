/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
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
#define	_SMF_TRACE_H

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
} Trace_T;

/** convenience macro for logging
 *
 * \param level loglevel, see trace_t
 * \param fmt format string for log message
 * \param ... format string arguments
 */
#define TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, fmt)
void trace(Trace_T level, const char * module, const char * function, int line, const char *formatstring, ...);


#endif	/* _SMF_TRACE_H */


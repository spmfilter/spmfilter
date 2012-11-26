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
#define _GNU_SOURCE
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#include "smf_core.h"
#include "smf_trace.h"
#include "smf_settings.h"

static int debug_flag = 0;
static SMFTraceDest_T debug_dest = TRACE_DEST_SYSLOG;

static const char * trace_to_text(SMFTrace_T level) {
	const char * const trace_text[] = {
		"EMERGENCY",
		"Alert",
		"Critical",
		"Error",
		"Warning",
		"Notice",
		"Info",
		"Debug",
		"Lookup"
	};
	return trace_text[ilogb((double) level)];
}

void configure_debug(int debug) {
	debug_flag = debug;
}

void configure_trace_destination(SMFTraceDest_T dest) {
	debug_dest = dest;
}

const char* format_string(SMFTrace_T level, const char *module, const char *function, int line,
                                 const char *sid, const char *formatstring,
                                 char* out, size_t size) {
	char debug_format[size];
	char sid_format[size];
    
	if (debug_flag == 1)
		snprintf(debug_format, size, " (%s:%d)", function, line);
	else
		debug_format[0] = '\0';
    
	if (sid != NULL)
		snprintf(sid_format, size, " [%s]", sid);
	else
		sid_format[0] = '\0';
    
	snprintf(out, size, "%s [%s]%s%s: %s\n",
		trace_to_text(level),
		module,
		debug_format,
		sid_format,
		formatstring);
    
	return out;
}

static void trace_syslog(SMFTrace_T level, const char *message) {
	int priority;

	// Convert our extended log levels (>128) to syslog levels
	switch (level) {
		case TRACE_EMERG:   priority = LOG_EMERG; break;
		case TRACE_ALERT:   priority = LOG_ALERT; break;
		case TRACE_CRIT:    priority = LOG_CRIT; break;
		case TRACE_ERR:     priority = LOG_ERR; break;
		case TRACE_WARNING: priority = LOG_WARNING;  break;
		case TRACE_NOTICE:  priority = LOG_NOTICE; break;
		case TRACE_INFO:    priority = LOG_INFO; break;
		default:            priority = LOG_DEBUG; break;
	}
	
	syslog(priority, message);
}

static void trace_stderr(const char *message) {
	fprintf(stderr, message);
}

void trace(SMFTrace_T level, const char *module, const char *function, int line, const char *sid, const char *formatstring, ...) {
	const size_t maxlen = 1024;
	va_list ap;
	char format[maxlen];
	char message[maxlen];
	size_t l;

	// Prepare the format-string
	format_string(level, module, function, line, sid, formatstring, format, maxlen);
    
	// Format the trace-message
	va_start(ap, formatstring);
	vsnprintf(message, maxlen, format, ap);
	va_end(ap);

	l = strlen(message);
	
	if (message[l] == '\n')
		message[l] = '\0';

	switch (debug_dest) {
		case TRACE_DEST_SYSLOG: trace_syslog(level, message); break;
		case TRACE_DEST_STDERR: trace_stderr(message); break;
		default:                fprintf(stderr, "Unsupported trace-destination: %i", debug_dest);
								abort();
								break;
	}
}

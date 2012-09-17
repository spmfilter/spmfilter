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

#define min(x,y) ((x)<=(y)?(x):(y))

static int debug_flag = 0;

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

void trace(SMFTrace_T level, const char *module, const char *function, int line, const char *sid, const char *formatstring, ...) {
	SMFTrace_T syslog_level;
	va_list ap;
	va_list cp;
	char *message = NULL;
	size_t l, maxlen=1024;
	char *out = NULL;

	/* Return now if we're not logging anything. */
	if (! level)
		return;

	va_start(ap, formatstring);
	va_copy(cp, ap);
	vasprintf(&message,formatstring,cp);
	va_end(cp);

	l = strlen(message);
	
	if (message[l] == '\n')
		message[l] = '\0';

	if (sid != NULL)
		asprintf(&out,"[%s] ",sid);
	else
		out = (char *)calloc(1,sizeof(char));

	if (level) {
		/* Convert our extended log levels (>128) to syslog levels */
		switch((int)ilogb((double) level)) {
			case 0:
				syslog_level = LOG_EMERG;
				break;
			case 1:
				syslog_level = LOG_ALERT;
				break;
			case 2:
				syslog_level = LOG_CRIT;
				break;
			case 3:
				syslog_level = LOG_ERR;
				break;
			case 4:
				syslog_level = LOG_WARNING;
				break;
			case 5:
				syslog_level = LOG_NOTICE;
				break;
			case 6:
				syslog_level = LOG_INFO;
				break;
			case 7:
				syslog_level = LOG_DEBUG;
				break;
			case 8:
				syslog_level = LOG_DEBUG;
				break;
			default:
				syslog_level = LOG_DEBUG;
				break;
		}
		size_t w = min(l,maxlen);
		message[w] = '\0';
		
		smf_core_strcat_printf(&out,"%s:[%s]",trace_to_text(level), module);
		if (debug_flag == 1) smf_core_strcat_printf(&out," %s(+%d)",function, line);

		smf_core_strcat_printf(&out, " %s",message);

		if ((level >= 128) && (debug_flag == 1)) 
   			syslog(syslog_level,out);
   		else if (level < 128)
			syslog(syslog_level, out);
	}
	free(out);
	free(message);
}

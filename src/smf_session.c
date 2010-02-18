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

#include <glib.h>

#include "smf_session.h"
#include "smf_trace.h"

#define THIS_MODULE "session"

SMFSession_T *session = NULL;

/** Initialize SMFSession_T structure
 *
 * \returns pointer to SMFSession_T type
 */
SMFSession_T *smf_session_get(void) {
	if (session == NULL) {
		TRACE(TRACE_DEBUG,"initialize session data");
		session = g_slice_new(SMFSession_T);
		session->helo = NULL;
		session->envelope_from = NULL;
		session->queue_file = NULL;
		session->envelope_to = NULL;
		session->xforward_addr = NULL;
	}
	
	return session;
}

/** Free SMFSession_T structure
 *
 * \param session SMFSession_T type
 */
void smf_session_free(void) {
	int i;
	TRACE(TRACE_DEBUG,"destroy session data");
	g_free(session->queue_file);
	g_free(session->helo);
	g_free(session->xforward_addr);

//	g_free(session->header->data);
//	g_slice_free(Header_T,session->header);

	if (session->envelope_from != NULL)
		g_free(session->envelope_from->addr);
	g_slice_free(SMFEmailAddress_T,session->envelope_from);
	for (i = 0; i < session->envelope_to_num; i++) {
		g_free(session->envelope_to[i]->addr);
		g_slice_free(SMFEmailAddress_T,session->envelope_to[i]);
	}
	g_free(session->envelope_to);
	g_slice_free(SMFSession_T,session);
	session = NULL;
}

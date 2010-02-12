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

#include "spmfilter.h"
#include "smf_mailconn.h"

#define THIS_MODULE "mailconn"

/** Initialize MailConn_T structure
 *
 * \returns pointer to MailConn_T type
 */
MailConn_T *mconn_new(void) {
	MailConn_T *mconn = NULL;
	TRACE(TRACE_DEBUG,"initialize session data");
	mconn = g_slice_new(MailConn_T);
	mconn->helo = NULL;
	mconn->from = NULL;
	mconn->queue_file = NULL;
	mconn->rcpts = NULL;
	mconn->xforward_addr = NULL;

	return mconn;
}

/** Free MailConn_T structure
 *
 * \param mconn MailConn_T type
 */
void mconn_free(MailConn_T *mconn) {
	int i;
	TRACE(TRACE_DEBUG,"destroy session data");
	g_free(mconn->queue_file);
	g_free(mconn->helo);
	g_free(mconn->xforward_addr);

	g_free(mconn->header->data);
	g_slice_free(Header_T,mconn->header);

	if (mconn->from != NULL)
		g_free(mconn->from->addr);
	g_slice_free(EmailAddress_T,mconn->from);
	for (i = 0; i < mconn->num_rcpts; i++) {
		g_free(mconn->rcpts[i]->addr);
		g_slice_free(EmailAddress_T,mconn->rcpts[i]);
	}
	g_free(mconn->rcpts);
	g_slice_free(MailConn_T,mconn);
}

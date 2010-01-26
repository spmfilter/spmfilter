#include <glib.h>

#include "spmfilter.h"

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
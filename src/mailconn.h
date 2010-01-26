#ifndef _MAILCONN_H
#define	_MAILCONN_H

/** Initialize MailConn_T structure
 *
 * \returns pointer to MailConn_T type
 */
MailConn_T *mconn_new(void);

/** Free MailConn_T structure
 *
 * \param mconn MailConn_T type
 */
void mconn_free(MailConn_T *mconn);

#endif	/* _MAILCONN_H */


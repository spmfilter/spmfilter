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

#ifndef _SMF_MAILCONN_H
#define	_SMF_MAILCONN_H

/** Retrieve MailConn_T structure
 *
 * \returns pointer to MailConn_T type
 */
MailConn_T *smf_mailconn_get(void);

/** Free MailConn_T structure
 *
 * \param mconn MailConn_T type
 */
void smf_mailconn_free(void);

#endif	/* _SMF_MAILCONN_H */


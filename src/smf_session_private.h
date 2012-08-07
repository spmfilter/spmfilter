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

#ifndef _SMF_SESSION_PRIVATE_H
#define	_SMF_SESSION_PRIVATE_H

#include "smf_session.h"

/*!
 * @fn SMFSession_T *smf_session_new(void)
 * @brief Create a new SMFSession_T object
 * @returns a newly allocated SMFSession_T object
 */
SMFSession_T *smf_session_new(void);

/*!
 * @fn void smf_session_free(SMFSession_T *session)
 * @brief Free a SMFSession_T object
 * @param session a SMFSession_T object
 */
void smf_session_free(SMFSession_T *session);

#endif	/* _SMF_SESSION_PRIVATE_H */
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

#ifndef _SMF_EMAIL_ADDRESS_H
#define	_SMF_EMAIL_ADDRESS_H

#include "spmfilter_config.h"
#include "smf_lookup.h"

typedef struct {
	char *addr;
	int is_local;
	SMFLookupResult_T *user_data;
} SMFEmailAddress_T;

/** Creates a new SMFEmailAddress_T object
 *
 * \returns an empty SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_new(void);

/** Free a SMFEmailAddress_T object
 *
 * \param ea a SMFEmailAddress_T object
 */
void smf_email_address_free(SMFEmailAddress_T *ea);

#endif	/* _SMF_EMAIL_ADDRESS_H */

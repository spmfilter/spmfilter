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

#include "smf_email_address.h"

#define THIS_MODULE "email_address"

/** Creates a new SMFEmailAddress_T object */
SMFEmailAddress_T *smf_email_address_new(void) {
	SMFEmailAddress_T *ea = NULL;
	
	ea = g_slice_new(SMFEmailAddress_T);
	ea->user_data = NULL;
	
	return ea;
}

/** Free SMFEmailAddress_T object */
void smf_email_address_free(SMFEmailAddress_T *ea) {
	if (ea != NULL) {
		if (ea->addr != NULL) {
			g_free(ea->addr);
			if (ea->user_data != NULL)
				smf_lookup_result_free(ea->user_data);
		}
		g_slice_free(SMFEmailAddress_T,ea);
	}
}

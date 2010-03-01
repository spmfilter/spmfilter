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

#ifndef _SMF_MESSAGE_PRIVATE_H
#define	_SMF_MESSAGE_PRIVATE_H

void smf_message_extract_addresses(GMimeObject *message);

typedef enum {
	HEADER_REMOVE = 0,
	HEADER_APPEND = 1,
	HEADER_PREPEND = 2,
	HEADER_SET = 3,
} SMFHeaderStatus_T;

typedef struct {
	SMFHeaderStatus_T status;
	char *name;
	char *value;
} SMFHeaderModification_T;

#endif	/* _SMF_MESSAGE_PRIVATE_H */


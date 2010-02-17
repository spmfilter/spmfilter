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

#ifndef _SMF_MESSAGE_H
#define	_SMF_MESSAGE_H

/* struct for messages send
 * via smtp_delivery */
typedef struct {
	/* message sender */
	char *from;

	/* pointer to message recipients */
	char **rcpts;

	/* number of recipients */
	int num_rcpts;

	/* path to message */
	char *message_file;

	/* SMTP auth user, if needed */
	char *auth_user;

	/* SMTP auth password, if needed */
	char *auth_pass;

	/* destination smtp server */
	char *nexthop;
} Message_T;


/** Write a message to disk
 *
 * \param new_path path for the new message file
 * \param queue_file path of the queue file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_write(char *new_path, char *queue_file);

/** Gets the value of the first header with the name requested.
 *
 * \param header_name name of the wanted header
 *
 * \returns value of header or NULL in case of error
 */
const char *smf_message_header_get(const char *header_name);

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_prepend(char *header_name, char *header_value);

/** Removed the specified header if it exists
 *
 * \param msg_path path to message or queue_file
 * \param header_name name of the header
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_mesage_remove_header(char *msg_path, char *header_name);

/** Deliver message
 *
 * \param msg_data Messate_T structure
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_deliver(Message_T *msg_data);


#endif	/* _SMF_MESSAGE_H */


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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>

#include "spmfilter_config.h"
#include "smf_session.h"
#include "smf_trace.h"
#include "smf_core.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_modules.h"
#include "smf_lookup.h"
#include "smf_settings.h"

#define THIS_MODULE "message"

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"

/** Copy the current message to disk
 *
 * \param path for the new message file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_copy_to_disk(char *path) {
	SMFSession_T *session = smf_session_get();
	GIOChannel *in;
	GMimeStream *out;
	int fd;
	gchar *line;
	GError *error = NULL;

	if (path == NULL)
		return -1;
	
	if ((fd = open(path,O_WRONLY|O_CREAT,S_IRWXU)) == -1) {
		TRACE(TRACE_ERR,"failed opening destination file");
		return -1;
	}

	out = g_mime_stream_fs_new(fd);

	if (smf_modules_flush_dirty(session) != 0)
		TRACE(TRACE_ERR,"message flush failed");

	if ((in = g_io_channel_new_file(session->queue_file,"r", &error)) == NULL) {
		TRACE(TRACE_ERR,"%s",error->message);
		g_error_free(error);
		close(fd);
		g_object_unref(out);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);

	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (g_mime_stream_write(out,line,strlen(line)) == -1) {
			TRACE(TRACE_ERR,"failed writing file");
			g_io_channel_shutdown(in,TRUE,NULL);
			g_io_channel_unref(in);
			close(fd);
			g_object_unref(out);
			g_free(line);
			g_remove(path);
			return -1;
		}
		g_free(line);
	}

	g_mime_stream_flush(out);
	close(fd);
	g_object_unref(out);
	g_io_channel_shutdown(in,TRUE,NULL);
	g_io_channel_unref(in);

	return 0;
}

/** Gets the value of the first header with the name requested.
 *
 * \param header_name name of the wanted header
 *
 * \returns requested header
 */
const char *smf_message_header_get(const char *header_name) {
	SMFSession_T *session = smf_session_get();
	const char *header_value = NULL;

	while (session->dirty_headers) {
		SMFHeaderModification_T *mod = (SMFHeaderModification_T *)((GSList *)session->dirty_headers)->data;
		if (g_ascii_strcasecmp(mod->name,header_name) == 0)
			return mod->value;
		session->dirty_headers = ((GSList *)session->dirty_headers)->next;
	}
#ifdef HAVE_GMIME24
	header_value = g_mime_header_list_get((GMimeHeaderList *)session->headers,header_name);
#else
	header_value = g_mime_header_get((GMimeHeader *)session->headers,header_name);
#endif
	return header_value;
}

/** Removed the specified header if it exists
 *
 * \param header_name name of the header
 * 
 * \returns 0 on success or -1 in case of error
 */
void smf_message_header_remove(char *header_name) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_REMOVE;
	header->name = g_strdup(header_name);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_remove((GMimeHeaderList *)session->headers,header_name);
#else
	g_mime_header_remove((GMimeHeader *)session->headers,header_name);
#endif
	return;
}

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_prepend(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_PREPEND;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_prepend((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	g_mime_header_prepend((GMimeHeader *)session->headers,header_name,header_value);
#endif
	return;
 }

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_append(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_APPEND;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	TRACE(TRACE_WARNING,"function not implemented in GMime < 2.4");
#endif
	return;
}

/** Set the value of the specified header. If value is NULL and the header,
 * name, had not been previously set, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * Note: If there are multiple headers with the specified field name,
 * the first instance of the header will be replaced and further instances
 * will be removed.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_set(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_SET;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_set((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	g_mime_header_set((GMimeHeader *)session->headers,header_name,header_value);
#endif
	return;
}

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \returns a string containing the header block.
 */
char *smf_message_header_to_string(void) {
	SMFSession_T *session = smf_session_get();
	char *header = NULL;

#ifdef HAVE_GMIME24
	header = g_mime_header_list_to_string((GMimeHeaderList *)session->headers);
#else
	header = g_mime_header_to_string((GMimeHeader *)session->headers);
#endif
	return header;
}

/** Calls func for each header name/value pair.
 *
 * \param func function to be called for each header.
 * \param user_data user data to be passed to the func.
 */
void smf_message_header_foreach(SMFHeaderForeachFunc func, void *user_data) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_foreach((GMimeHeaderList *)session->headers,func,user_data);
#else
	g_mime_header_foreach((GMimeHeader *)session->headers,func,user_data);
#endif
}

void smf_message_extract_addresses(GMimeObject *message) {
	SMFSession_T *session = smf_session_get();
	SMFSettings_T *settings = smf_settings_get();
	InternetAddressList *ia;
	InternetAddress *addr;
	int i;

	/* get the from field */
	session->message_from = g_slice_new(SMFEmailAddress_T);
	session->message_from->addr = smf_core_get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(GMIME_MESSAGE(message)),1);

	TRACE(TRACE_DEBUG,"session->message_from: %s",session->message_from->addr);

	session->message_from->is_local = smf_lookup_check_user(session->message_from->addr);
	TRACE(TRACE_DEBUG,"[%s] is local [%d]",
			session->message_from->addr,
			session->message_from->is_local);

	/* now check the to field */
	session->message_to_num = 0;

	session->message_to = malloc(sizeof(session->message_to[session->message_to_num]));

#ifdef HAVE_GMIME24
	/* g_mime_message_get_all_recipients() appeared in gmime 2.2.5 */
	ia = g_mime_message_get_all_recipients(GMIME_MESSAGE(message));
	if (ia != NULL) {
		for (i=0; i < internet_address_list_length(ia); i++) {
			addr = internet_address_list_get_address(ia,i);
			session->message_to[session->message_to_num] = malloc(
					sizeof(*session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			TRACE(TRACE_DEBUG,"session->message_to[%d]: %s",
					session->message_to_num,
					session->message_to[session->message_to_num]->addr);

			if (strcmp(settings->backend,"undef") != 0) {
				session->message_to[session->message_to_num]->is_local =
						smf_lookup_check_user(session->message_to[session->message_to_num]->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]",
						session->message_to[session->message_to_num]->addr,
						session->message_to[session->message_to_num]->is_local);
			}
			session->message_to_num++;
		}
	}
#else
	ia = (InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_TO);
	if (ia != NULL) {
		internet_address_list_concat(ia,
			(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_CC));
		internet_address_list_concat(ia,
			(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_BCC));
		while(ia) {
			addr = internet_address_list_get_address(ia);
			session->message_to = malloc(sizeof(session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num] = malloc(sizeof(*session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			TRACE(TRACE_DEBUG,"session->message_to[%d]: %s",
					session->message_to_num,
					session->message_to[session->message_to_num]->addr);

			if (strcmp(settings->backend,"undef") != 0) {
				session->message_to[session->message_to_num]->is_local =
						smf_lookup_check_user(session->message_to[session->message_to_num]->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]",
						session->message_to[session->message_to_num]->addr,
						session->message_to[session->message_to_num]->is_local);
			}
			session->message_to_num++;
			ia = internet_address_list_next(ia);
		}
	}
#endif

}

/** Decodes an rfc2047 encoded 'text' header.
 *
 * \param text header text to decode
 *
 * \returns a newly allocated UTF-8 string representing the the decoded header.
 */
char *smf_message_decode_text(const char *text) {
	return g_mime_utils_header_decode_text(text);
}

/** Encodes a 'text' header according to the rules in rfc2047.
 *
 * \param text text to encode
 *
 * \returns the encoded header. Useful for encoding headers like "Subject".
 */
char *smf_message_encode_text(const char *text) {
	return g_mime_utils_header_encode_text(text);
}

/** Generates a unique Message-Id.
 *
 * \returns a unique string in an addr-spec format suitable for use as a Message-Id.
 */
char *smf_message_generate_message_id(void) {
	char *mid = NULL;
	char hostname[256];

	gethostname(hostname,256);
	mid = g_strdup_printf("<%s>",g_mime_utils_generate_message_id(hostname));
	
	return mid;
}

/** Determines the best content encoding for the first len bytes of text.
 *
 * \param text text to encode
 * \param len text length
 *
 * \returns a SMFContentEncoding that is determined to be the best encoding
 *          type for the specified block of text. ("best" in this particular
 *          case means smallest output size)
 */
SMFContentEncoding smf_message_best_encoding(unsigned char *text, size_t len) {
	return (SMFContentEncoding) g_mime_utils_best_encoding(text,len);
}

/** Prepend text to subject
 *
 * \param text text to prepend
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_subject_prepend(char *text) {
	char *subject = (char *)smf_message_header_get("subject");
	if (subject == NULL)
		return -1;

	// TODO: check subject encoding

	smf_message_header_set("subject",g_strdup_printf("%s %s",text,subject));
	return 0;
}

/** Append text to subject
 *
 * \param text text to append
 *
 * \return 0 on success or -1 in case of error
 */
int smf_message_subject_append(char *test) {
	char *subject = (char *)smf_message_header_get("subject");

	smf_message_header_set("subject",g_strdup_printf("%s %s",subject, test));
	return 0;
}
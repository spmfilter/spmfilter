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
#include <gmime/gmime.h>
#include <string.h>

#include "spmfilter_config.h"
#include "smf_message.h"
#include "smf_mime.h"
#include "smf_trace.h"

#define THIS_MODULE "mime"

/** Creates a new SMFDataWrapper_T object around buffer
 *
 * \param buffer content for data wrapper
 * \param encoding contents encoding
 *
 * \returns a new SMFDataWrapper_T object
 */
SMFDataWrapper_T *smf_mime_data_wrapper_new(const char *buffer, SMFContentEncoding_T encoding) {
	GMimeStream *stream;
	SMFDataWrapper_T *wrapper;

	if (buffer == NULL)
		return NULL;

	wrapper = g_slice_new(SMFDataWrapper_T);
	stream = g_mime_stream_mem_new_with_buffer(buffer, strlen(buffer));

	wrapper->data = g_mime_data_wrapper_new_with_stream(stream, encoding);
	g_object_unref(stream);

	return wrapper;
}

/** Free SMFDataWrapper_T object
 *
 * \param wrapper SMFDataWrapper_T object
 */
void smf_mime_data_wrapper_unref(SMFDataWrapper_T *wrapper) {
	g_object_unref(wrapper->data);
	g_slice_free(SMFDataWrapper_T,wrapper);
}

/** Creates a new MIME Part with a sepcified type. If type and
 *  subtype is NULL, object will be created with a default
 *  content-type of text/plain.
 *
 * \param type content-type
 * \param subtype content-subtype
 *
 * \returns an empty MIME Part object with the specified content-type.
 */
SMFMimePart_T *smf_mime_part_new(const char *type, const char *subtype) {
	SMFMimePart_T *part;
	part = g_slice_new(SMFMimePart_T);
	if ((type == NULL) && (subtype == NULL)) {
		part->data = g_mime_part_new_with_type("text","plain");
	} else {
		part->data = g_mime_part_new_with_type(type,subtype);
	}

	return part;
}

/** Free SMFMimePart_T object
 *
 * \param part SMFMimePart_T object
 */
void smf_mime_part_unref(SMFMimePart_T *part) {
	g_object_unref(part->data);
	g_slice_free(SMFMimePart_T,part);
}

/** Set the content encoding for the specified mime part.
 *
 * \param part SMFMimePart_T
 * \param encoding SMFContentEncoding_T
 */
void smf_mime_part_set_encoding(SMFMimePart_T *part, SMFContentEncoding_T encoding) {
#ifdef HAVE_GMIME24
	g_mime_part_set_content_encoding((GMimePart *)part->data, encoding);
#else
	g_mime_part_set_content_encoding((GMimePart *)part->data, encoding);
#endif
}

/** Sets the disposition to disposition which may be one of SMD_DISPOSITION_ATTACHMENT
 *  or SMF_DISPOSITION_INLINE or, by your choice, any other string which would
 *  indicate how the MIME part should be displayed by the MUA.
 *
 * \param part SMFMimePart_T
 * \param disposition the content disposition
 */
void smf_mime_part_set_disposition(SMFMimePart_T *part, const char *disposition) {
#ifdef HAVE_GMIME24
	g_mime_object_set_disposition(GMIME_OBJECT(part->data), disposition);
#else
	g_mime_part_set_content_disposition((GMimePart *)part->data,disposition);
#endif
}

/** Sets the content object on the mime part.
 *
 * \param part a SMFMimePart_T object
 * \param content a SMFDataWrapper_T content obeject
 */
void smf_mime_set_content(SMFMimePart_T *part, SMFDataWrapper_T *content) {
	g_mime_part_set_content_object((GMimePart *)part->data,(GMimeDataWrapper *)content->data);
}

/** Gets the internal data-wrapper of the specified mime part, or NULL on error.
 *
 * \param part a SMFMimePart_T object
 *
 * \returns the data-wrapper for the mime part's contents.
 */
SMFDataWrapper_T *smf_mime_get_content(SMFMimePart_T *part) {
	SMFDataWrapper_T *content;
	content->data = g_mime_part_get_content_object((GMimePart *)part->data);

	return content;
}

/** Creates a new MIME multipart object with a content-type of multipart/subtype.
 *
 * \param subtype content-type subtype. If subtype is NULL, a multipart object
 *                with a default content-type of multipart/mixed will be created.
 * \returns a SMFMultiPart_T object
 */
SMFMultiPart_T *smf_mime_multipart_new(const char *subtype) {
	SMFMultiPart_T *multipart;

	multipart = g_slice_new(SMFMultiPart_T);
	if (subtype == NULL) {
		multipart->data = g_mime_multipart_new();
	} else {
		multipart->data = g_mime_multipart_new_with_subtype(subtype);
	}

	return multipart;
}

/** Free SMFMultiPart_T object
 *
 * \param multipart SMFMultiPart_T object
 */
void smf_mime_multipart_unref(SMFMultiPart_T *multipart) {
	g_object_unref(multipart->data);
	g_slice_free(SMFMultiPart_T,multipart);
}

/** Adds a mime part to the multipart.
 *
 * \param multipart a SMFMultiPart_T object
 * \param part a SMFMimePart_T object
 */
void smf_mime_multipart_add(SMFMultiPart_T *multipart, SMFMimePart_T *part) {
#ifdef HAVE_GMIME24
	g_mime_multipart_add((GMimeMultipart *)multipart->data,(GMimeObject *)part->data);
#else
	g_mime_multipart_add_part((GMimeMultipart *)multipart->data,(GMimeObject *)part->data);
#endif
}

/** Gets the index of part within multipart.
 *
 * \param multipart a SMFMultiPart_T object
 * \param part a SMFMimePart_T object
 *
 * \returns the index of part within multipart or -1 if not found.
 */
int smf_mime_multipart_index_of(SMFMultiPart_T *multipart, SMFMimePart_T *part) {
#ifdef HAVE_GMIME24
	return g_mime_multipart_index_of((GMimeMultipart *)multipart->data,(GMimeObject *)part->data);
#else
	return g_list_length(((GMimeMultipart *)multipart->data)->subparts);
#endif
}

/** Inserts the specified mime part into the multipart at the position index.
 *
 * \param multipart a SMFMultiPart_T object
 * \param part a SMFMimePart_T object
 * \param index position to insert the mime part
 */
void smf_mime_multipart_insert(SMFMultiPart_T *multipart, SMFMimePart_T *part, int index) {
#ifdef HAVE_GMIME24
	g_mime_multipart_insert((GMimeMultipart *)multipart->data,index,(GMimeObject *)part->data);
#else
	g_mime_multipart_add_part_at((GMimeMultipart *)multipart->data,(GMimeObject *)part->data,index);
#endif
}

/** Removes the specified mime part from the multipart.
 *
 * \param multipart a SMFMultiPart_T object
 * \param part a SMFMimePart_T object
 */
void smf_mime_multipart_remove(SMFMultiPart_T *multipart, SMFMimePart_T *part) {
#ifdef HAVE_GMIME24
	g_mime_multipart_remove((GMimeMultipart *)multipart->data,(GMimeObject *)part->data);
#else
	g_mime_multipart_remove_part((GMimeMultipart *)multipart->data,(GMimeObject *)part->data);
#endif
}

/** Removes the mime part at position index from the multipart.
 *
 * \param multipart a SMFMultiPart_T object
 * \param position of the mime part to remove
 *
 * \returns the mime part that was removed or NULL if the part was not contained within the multipart.
 */
SMFMimePart_T *smf_mime_multipart_remove_at(SMFMultiPart_T *multipart, int index) {
	SMFMimePart_T *part = NULL;

#ifdef HAVE_GMIME24
	part->data = g_mime_multipart_remove_at((GMimeMultipart *)multipart->data,index);
#else
	part->data = g_mime_multipart_remove_part_at((GMimeMultipart *)multipart->data,index);
#endif

	return part;
}

/** Sets boundary as the boundary on the multipart. If boundary is NULL,
 *  then a boundary will be auto-generated for you.
 *
 * \param multipart a SMFMultiPart_T object
 * \param boundary boundary or NULL to autogenerate one
 */
void smf_mime_multipart_set_boundary(SMFMultiPart_T *multipart, const char *boundary) {
	g_mime_multipart_set_boundary((GMimeMultipart *)multipart->data,boundary);
}

/** Gets the boundary on the multipart. If the internal boundary is NULL, then
 *  an auto-generated boundary will be set on the multipart and returned.
 *
 * \param multipart a SMFMultiPart_T object
 *
 * \returns the boundary on the multipart.
 */
const char *smf_mime_multipart_get_boundary(SMFMultiPart_T *multipart) {
	return g_mime_multipart_get_boundary((GMimeMultipart *)multipart->data);
}

/** Removes all subparts from multipart.
 *
 * \param multipart a SMFMultiPart_T object
 */
void smf_mime_multipart_clear(SMFMultiPart_T *multipart) {
	g_mime_multipart_clear((GMimeMultipart *)multipart->data);
}

/** Gets the number of mime parts contained within the multipart.
 *
 * \param multipart a SMFMultiPart_T object
 *
 * \returns the number of mime parts contained within the multipart.
 */
int smf_mime_muiltpart_get_count(SMFMultiPart_T *multipart) {
#ifdef HAVE_GMIME24
	return g_mime_multipart_get_count((GMimeMultipart *)multipart->data);
#else
	return g_mime_multipart_get_number((GMimeMultipart *)multipart->data);
#endif
}

#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>
#include <fcntl.h>

#include "spmfilter.h"

#define THIS_MODULE "message"

/** Parse a message file on disk and return a GMimeMessage object
 *
 * \param msg_path path to message file
 *
 * \returns GMimeMessage object
 */
// TODO: remove from spmfilter.h?
GMimeMessage *parse_message(char *msg_path) {
	GMimeMessage *message = NULL;
	GMimeParser *parser;
	GMimeStream *stream;
	int fd;

	if ((fd = g_open(msg_path, O_RDONLY)) == -1) {
		TRACE(TRACE_ERR, "cannot open message `%s': %s", msg_path, strerror(errno));
		return NULL;
	}

	stream = g_mime_stream_fs_new(dup(fd));
	parser = g_mime_parser_new_with_stream(stream);
	g_object_unref(stream);
	message = g_mime_parser_construct_message(parser);
	g_object_unref(parser);
	close(fd);
	return message;
}


/** write a message to disk
 *
 * \param new_path path for the new message file
 * \param queue_file path of the queue file
 *
 * \returns 0 on success or -1 in case of error
 */
int write_message(char *new_path, char *queue_file) {
	GMimeStream *stream;
	int fd;
	GMimeMessage *message = parse_message(queue_file);

	if ((fd = g_open(new_path, O_CREAT|O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH)) == -1) {
		TRACE(TRACE_ERR, "cannot open message `%s': %s", new_path, strerror(errno));
		return -1;
	}

	stream = g_mime_stream_fs_new(fd);
	if (g_mime_object_write_to_stream((GMimeObject *) message, stream) == -1) {
		return -1;
	}

	if (g_mime_stream_flush(stream) != 0) {
		return -1;
	}
	g_mime_stream_close(stream);
	g_object_unref(stream);
	close(fd);
	return 0;
}

/** Gets the value of the requested header if it exists or NULL otherwise.
 *
 * \param msg_path path to message or queue file
 * \param header_name name of the wanted header
 *
 * \returns requested header
 */
const char *get_header(char *msg_path, char *header_name) {
	GMimeMessage *message;
	const char *header_value = NULL;

	message = parse_message(msg_path);
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		header_value = g_mime_object_get_header(GMIME_OBJECT(message),header_name);
#else
		header_value = g_mime_message_get_header(message,header_name);
#endif
	}

	g_free(header_name);
	g_object_unref(message);
	return header_value;
}

/** Removed the specified header if it exists
 *
 * \param msg_path path to message or queue_file
 * \param header_name name of the header
 * 
 * \returns 0 on success or -1 in case of error
 */
int remove_header(char *msg_path, char *header_name) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);
	
	if (message!=NULL) {
		g_mime_object_remove_header((GMimeObject *)message,header_name);
		gen_queue_file(&tmp_file);
		
		if (write_message(tmp_file,msg_path) != 0)
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_object_unref(message);
	return 0;
}

/*
 * Adds an arbitrary header to the message, returns 0 on success.
 *
 * msg_path: path to message or queue file
 * header_name: name of the new header
 * header_value: value for the new header
 */
int add_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);

	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_append_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_add_header(message,header_name,header_value);
#endif
		gen_queue_file(&tmp_file);

		if (write_message(tmp_file,msg_path) != 0)
			return -1;
	} else
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_free(header_value);
	g_object_unref(message);
	return 0;
}

/*
 * Sets an arbitrary header, returns 0 on success.
 *
 * msg_path: path to message or queue file
 * header_name: name of the header
 * header_value: new value for the header
 */
int set_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);

	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_set_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_set_header(message,header_name,header_value);
#endif
		gen_queue_file(&tmp_file);

		if (write_message(tmp_file,msg_path) != 0)
			return -1;
	} else
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_free(header_value);
	g_object_unref(message);
	return 0;
}

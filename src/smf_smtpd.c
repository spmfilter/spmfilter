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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <glib.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "spmfilter_config.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_modules.h"
#include "smf_session.h"
#include "smf_smtp_codes.h"
#include "smf_core.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_message.h"
#include "smf_message_private.h"

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#define THIS_MODULE "smtpd"

#define CODE_221 "221 Goodbye. Please recommend us to others!\r\n"
#define CODE_250 "250 OK\r\n"
#define CODE_250_ACCEPTED "250 OK message accepted\r\n"
#define CODE_451 "451 Requested action aborted: local error in processing\r\n"
#define CODE_502 "502 Error: Command not recognized\r\n"
#define CODE_552 "552 Requested action aborted: local error in processing\r\n"

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_XFWD 2
#define ST_MAIL 3
#define ST_RCPT 4
#define ST_DATA 5
#define ST_QUIT 6

#define RE_MAIL_FROM "^MAIL FROM:?\\W*(?:.*<)?([^>]*)(?:>)?(?:\\W*SIZE=(\\d+))?"

/* copy headers from message object to own GMimeHeaderList */
static void copy_header_func(const char *name, const char *value, gpointer data) {
#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)data,
			g_strdup(name),g_strdup(value));
#else
	g_mime_header_add((GMimeHeader *)data,
			g_strdup(name),g_strdup(value));
#endif
}

/* dot-stuffing */
void stuffing(char chain[]) {
	int i, j;
	int found = 0;
	for (i=0,j=0; chain[i] != '\0'; i++) {
		if ((chain[i] != '.') || (found == 1)) {
			chain[j++]=chain[i];
		} else {
			found = 1;
		}
	}
	chain[j]='\0';
}

/* smtp answer with format string as arg */
void smtpd_string_reply(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout,format,ap);
	va_end(ap);
	fflush(stdout);
}

/** Write SMTP answer to stdout
 *
 * \param wanted smtp code
 */
void smtpd_code_reply(int code) {
	char *code_msg;
	/* we don't need to free code_msg, will be
	 * freed by smtp_code_free() */
	code_msg = smf_smtp_codes_get(code);
	if (code_msg!=NULL) {
		fprintf(stdout,"%d %s\r\n",code,code_msg);
	} else {
		switch(code) {
			case 221:
				fprintf(stdout,CODE_221);
				break;
			case 250:
				fprintf(stdout,CODE_250);
				break;
			case 451:
				fprintf(stdout,CODE_451);
				break;
			case 502:
				fprintf(stdout,CODE_502);
				break;
			case 552:
				fprintf(stdout,CODE_552);
				break;
			default:
				fprintf(stdout,CODE_451);
				break;
		}
	}
	fflush(stdout);
}

/* error handler used when building module queue
 * return 1 if processing should continue, else 0
 */
static int handle_q_error(void *args) {
	SMFSettings_T *settings = smf_settings_get();

	switch (settings->module_fail) {
		case 1: return(1);
		case 2: smtpd_code_reply(552);
				return(0);
		case 3: smtpd_code_reply(451);
				return(0);
	}

	return(0);
}

/* handle processing errors when running queue 
 *
 * return codes:
 * -1 = Error in processing, spmfilter will send 4xx Error to MTA
 * 0 = All ok, the next plugin will be started.
 * 1 = Further processing will be stopped. Email is not going
 *     to be delivered to nexthop!
 * 2 = Further processing will be stopped, no other plugin will
 *     be startet. spmfilter sends a 250 code
 */
static int handle_q_processing_error(int retval, void *args) {
	SMFSettings_T *settings = smf_settings_get();
	SMFSession_T *session = smf_session_get();

	if (retval == -1) {
		switch (settings->module_fail) {
			case 1: return(1);
			case 2: smtpd_code_reply(552);
					return(0);
			case 3: smtpd_code_reply(451);
					return(0);
		}
	} else if(retval == 1) {
		if (session->response_msg != NULL) {
			char *smtp_response;
			smtp_response = g_strdup_printf("250 %s\r\n",session->response_msg);
			smtpd_string_reply(smtp_response);
			free(smtp_response);
		} else
			smtpd_string_reply(CODE_250_ACCEPTED);
		return(1);
	} else if(retval == 2) {
		return(2);
	} else {
		if (session->response_msg != NULL) {
			char *smtp_response;
			smtp_response = g_strdup_printf("%d %s\r\n",retval,session->response_msg);
			smtpd_string_reply(smtp_response);
			free(smtp_response);
		} else
			smtpd_code_reply(retval);
		return(1);
	}

	/* if none of the above matched, halt processing, this is just
	 * for safety purposes
	 */
	TRACE(TRACE_DEBUG, "no conditional matched, will stop queue processing!");
	return(0);
}

/* handle nexthop delivery error */
static int handle_nexthop_error(void *args) {
	SMFSettings_T *settings = smf_settings_get();

	smtpd_string_reply(g_strdup_printf(
		"%d %s\r\n",
		settings->nexthop_fail_code,
		settings->nexthop_fail_msg)
	);

	return(0);
}

int load_modules(void) {
	int ret;
	ProcessQueue_T *q;
	SMFSession_T *session = smf_session_get();

	/* initialize the modules queue handler */
	q = smf_modules_pqueue_init(
		handle_q_error,
		handle_q_processing_error,
		handle_nexthop_error
	);

	if(q == NULL) {
		return(-1);
	}

	/* now tun the process queue */
	ret = smf_modules_process(q,session);
	free(q);

	if(ret == -1) {
		TRACE(TRACE_DEBUG, "smtp engine failed to process modules!");
		return(-1);
	} else if (ret == 1) {
		return(0);
	}

	if (session->response_msg != NULL) {
		char *smtp_response;
		smtp_response = g_strdup_printf("250 %s\r\n",session->response_msg);
		smtpd_string_reply(smtp_response);
		free(smtp_response);
	} else
		smtpd_string_reply(CODE_250_ACCEPTED);
	return(0);
}

void process_data(void) {
	GIOChannel *in;
	GMimeStream *out;
	gchar *line;
	gsize length;
	FILE *fd;
	GMimeParser *parser;
	GMimeMessage *message;
	char *message_id;
	SMFSession_T *session = smf_session_get();

	smf_core_gen_queue_file(&session->queue_file);
	if (session->queue_file == NULL) {
		TRACE(TRACE_ERR,"failed to create spool file!");
		smtpd_code_reply(552);
		return;
	}
		
	TRACE(TRACE_DEBUG,"using spool file: '%s'", session->queue_file);
		
	smtpd_string_reply("354 End data with <CR><LF>.<CR><LF>\r\n");
	
	/* start receiving data */
	in = g_io_channel_unix_new(dup(STDIN_FILENO));
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_close_on_unref(in,TRUE);

	if ((fd = fopen(session->queue_file,"wb+")) == NULL) {
		return;
	}
	
	out = g_mime_stream_file_new(fd);

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, ".\r\n")==0)||(g_ascii_strcasecmp(line, ".\n")==0)) break;
		if (g_ascii_strncasecmp(line,".",1)==0) stuffing(line);
		
		if (g_mime_stream_write(out,line,length) == -1) {
			smtpd_string_reply(CODE_451);
			g_object_unref(out);
			g_io_channel_unref(in);
			g_free(line);
			if (g_remove(session->queue_file) != 0)
				TRACE(TRACE_ERR,"failed to remove queue file");
			return;
		}
		session->msgbodysize+=length;
		g_free(line);
	}

	g_io_channel_unref(in);

	/* extract message headers */
	g_mime_stream_flush(out);
	g_mime_stream_seek(out,0,0);

	parser = g_mime_parser_new_with_stream(out);
	message = g_mime_parser_construct_message(parser);
#ifdef HAVE_GMIME24
	session->headers = (void *)g_mime_header_list_new();
	g_mime_header_list_foreach(GMIME_OBJECT(message)->headers, copy_header_func, session->headers);
#else
	session->headers = (void *)g_mime_header_new();
	g_mime_header_foreach(GMIME_OBJECT(message)->headers, copy_header_func, session->headers);
#endif
	
	smf_message_extract_addresses(GMIME_OBJECT(message));
	g_object_unref(parser);
	g_object_unref(message);
	g_object_unref(out);

	if (session->message_from->addr == NULL) {
		smf_session_header_append("From",g_strdup(session->envelope_from->addr));
		TRACE(TRACE_DEBUG,"adding [from] header to message");
	}

	if (session->message_to_num == 0) {
		smf_session_header_append("To",g_strdup("undisclosed-recipients:;"));
		TRACE(TRACE_DEBUG,"adding [to] header to message");
	}

	message_id = (char *)smf_session_header_get("message-id");

	if (message_id == NULL) {
		message_id = smf_message_generate_message_id();
		TRACE(TRACE_DEBUG,"no message id found, adding [%s]",message_id);
		smf_session_header_append("Message-ID",message_id);
	}

	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->msgbodysize);

	load_modules();
	
	if (g_remove(session->queue_file) != 0)
 		TRACE(TRACE_ERR,"failed to remove queue file");
	TRACE(TRACE_DEBUG,"removing spool file %s",session->queue_file);
	return;
}

int load(void) {
	char hostname[256];
	GIOChannel *in;
	char *line;
	int state=ST_INIT;
	SMFSettings_T *settings = smf_settings_get();
	SMFSession_T *session = smf_session_get();
	const char *requested_size = NULL;
	const char *mail_from_addr = NULL;
#if (GLIB2_VERSION >= 21400)
	GRegex *re = NULL;
	GMatchInfo *match_info = NULL;
#else
	pcre *re;
	int ovector[30];
	int rc, erroffset;
	const char *error;
#endif

	gethostname(hostname,256);

	smtpd_string_reply("220 %s spmfilter\r\n",hostname);
	session->envelope_to_num = 0;
	in = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(in, NULL, NULL);
	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_strstrip(line);
		TRACE(TRACE_DEBUG,"client smtp dialog: '%s'",line);
		
		if (g_ascii_strncasecmp(line,"quit",4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'quit' received"); 
			smtpd_code_reply(221);
			state = ST_QUIT;
			break;
		} else if (g_ascii_strncasecmp(line, "helo", 4)==0) {
			/* An EHLO command MAY be issued by a client later in the session.
			 * If it is issued after the session begins, the SMTP server MUST
			 * clear all buffers and reset the state exactly as if a RSET
			 * command had been issued.
			 */
			if (state != ST_INIT) {
				smf_session_free();
				/* reinit session */
				session = smf_session_get();
			}
			TRACE(TRACE_DEBUG,"SMTP: 'helo' received");
			session->helo = smf_core_get_substring("^HELO\\s(.*)$",line, 1);
			TRACE(TRACE_DEBUG,"HELO: %s",session->helo);
			if (session->helo != NULL) {
				if (strcmp(session->helo,"") == 0)  {
					smtpd_string_reply("501 Syntax: HELO hostname\r\n");
				} else {
					TRACE(TRACE_DEBUG,"session->helo: %s",session->helo);
					smtpd_string_reply("250 %s\r\n",hostname);
					state = ST_HELO;
				}
			} else {
				smtpd_string_reply("501 Syntax: HELO hostname\r\n");
			}
		} else if (g_ascii_strncasecmp(line, "ehlo", 4)==0) {
			/* Same here....clear all buffers, if ehlo command is
			 * received later...
			 */
			if (state != ST_INIT) {
				smf_session_free();
				/* reinit session */
				session = smf_session_get();
			}
			TRACE(TRACE_DEBUG,"SMTP: 'ehlo' received");
			session->helo = smf_core_get_substring("^EHLO\\s(.*)$",line,1);
			if (session->helo != NULL) {
				if (strcmp(session->helo,"") == 0) {
					smtpd_string_reply("501 Syntax: EHLO hostname\r\n");
				} else {
					TRACE(TRACE_DEBUG,"session->helo: %s",session->helo);
					smtpd_string_reply("250-%s\r\n250-XFORWARD NAME ADDR PROTO HELO SOURCE\r\n250 SIZE\r\n",hostname);
					state = ST_HELO;
				}
			} else {
				smtpd_string_reply("501 Syntax: HELO hostname\r\n");
			}
		} else if (g_ascii_strncasecmp(line,"xforward name",13)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'xforward name' received");
			session->xforward_addr = smf_core_get_substring("^XFORWARD NAME=.* ADDR=(.*)$",line,1);
			TRACE(TRACE_DEBUG,"session->xforward_addr: %s",session->xforward_addr);
			smtpd_code_reply(250);
			state = ST_XFWD;
		} else if (g_ascii_strncasecmp(line, "xforward proto", 13)==0) {
			smtpd_code_reply(250);
			state = ST_XFWD;
		} else if (g_ascii_strncasecmp(line, "mail from:", 10)==0) {
			/* The MAIL command begins a mail transaction. Once started, 
			 * a mail transaction consists of a transaction beginning command, 
			 * one or more RCPT commands, and a DATA command, in that order. 
			 * A mail transaction may be aborted by the RSET (or a new EHLO) 
			 * command. There may be zero or more transactions in a session. 
			 * MAIL MUST NOT be sent if a mail transaction is already open, 
			 * e.g., it should be sent only if no mail transaction had been 
			 * started in the session, or if the previous one successfully 
			 * concluded with a successful DATA command, or if the previous 
			 * one was aborted with a RSET.
			 */
			TRACE(TRACE_DEBUG,"SMTP: 'mail from' received");
			if (state == ST_MAIL) {
				/* we already got the mail command */
				smtpd_string_reply("503 Error: nested MAIL command\r\n");
			} else {
				session->envelope_from = g_slice_new(SMFEmailAddress_T);
#if (GLIB2_VERSION >= 21400)
				re = g_regex_new(RE_MAIL_FROM, G_REGEX_CASELESS, 0, NULL);
				g_regex_match(re, line, 0, &match_info);
				if(g_match_info_matches(match_info)) {
					mail_from_addr = g_match_info_fetch(match_info, 1);
					if (mail_from_addr != NULL) {
						session->envelope_from->addr = g_strdup(mail_from_addr);
						free((char *)mail_from_addr);
					}
					if (settings->max_size != 0 )
						requested_size = g_match_info_fetch(match_info, 2);
				} else {
					smtpd_string_reply(CODE_552);
					g_slice_free(SMFEmailAddress_T,session->envelope_from);
					session->envelope_from = NULL;
				}
				g_match_info_free(match_info);
				g_regex_unref(re);
#else
				re = pcre_compile(RE_MAIL_FROM,
						PCRE_CASELESS, &error, &erroffset, NULL);
				if(re != NULL) {
					rc = pcre_exec(re, NULL, line, strlen(line), 0, 0, ovector, 30);
					if (rc > 0) {
						pcre_get_substring(line,ovector,rc,1,&mail_from_addr);
						if (mail_from_addr != NULL) {
							session->envelope_from->addr = g_strdup(mail_from_addr);
							free((char *)mail_from_addr);
						}
						if (settings->max_size != 0 )
							pcre_get_substring(line,ovector,rc,2,&requested_size);
					} else{
						smtpd_string_reply(CODE_552);
						g_slice_free(SMFEmailAddress_T,session->envelope_from);
						session->envelope_from = NULL;
					}
				} else {
					smtpd_string_reply(CODE_552);
					g_slice_free(SMFEmailAddress_T,session->envelope_from);
					session->envelope_from = NULL;
				}
#endif

				if (settings->max_size != 0) {
					if (requested_size != NULL) {
						unsigned long l;
						l = (unsigned long) strtol(requested_size,NULL,10);
						if (l > settings->max_size) {
							smtpd_string_reply("552 Message size limit exceeded\r\n");
							g_slice_free(SMFEmailAddress_T,session->envelope_from);
							session->envelope_from = NULL;
							continue;
						}
					}
				}

				if (session->envelope_from->addr != NULL){
					TRACE(TRACE_DEBUG,"session->envelope_from: %s",session->envelope_from->addr);
					if (strcmp(session->envelope_from->addr,"") == 0) {
						/* check for emtpy string */
						smtpd_string_reply("501 Syntax: MAIL FROM:<address>\r\n");
						g_slice_free(SMFEmailAddress_T,session->envelope_from);
						session->envelope_from = NULL;
					} else {
						if (settings->backend != NULL) {
								smf_lookup_check_user(session->envelope_from);
								TRACE(TRACE_DEBUG,"[%s] is local [%d]", session->envelope_from->addr,session->envelope_from->is_local);
						} else 
							session->envelope_from->user_data = NULL;

						smtpd_code_reply(250);
						state = ST_MAIL;
					}
				} else {
					smtpd_string_reply("501 Syntax: MAIL FROM:<address>\r\n");
					g_slice_free(SMFEmailAddress_T,session->envelope_from);
					session->envelope_from = NULL;
				}
			}
		} else if (g_ascii_strncasecmp(line, "rcpt to:", 8)==0) {
			if ((state != ST_MAIL) && (state != ST_RCPT)) {
				/* someone wants to break smtp rules... */
				smtpd_string_reply("503 Error: need MAIL command\r\n");
			} else {
				TRACE(TRACE_DEBUG,"SMTP: 'rcpt to' received");

				/* reallocate memory to make room for additional recipients */
				session->envelope_to = g_realloc(
					session->envelope_to,
					sizeof(SMFEmailAddress_T) * (session->envelope_to_num + 1)
				);

				/* allocate resources for the individual recipient */
				session->envelope_to[session->envelope_to_num] = g_slice_new(SMFEmailAddress_T);
				session->envelope_to[session->envelope_to_num]->addr = smf_core_get_substring("^RCPT TO:?\\W*(?:.*<)?([^>]*)(?:>)?", line, 1);
				if (session->envelope_to[session->envelope_to_num] != NULL) {
					if (strcmp(session->envelope_to[session->envelope_to_num]->addr,"") == 0) {
						/* empty rcpt to? */
						smtpd_string_reply("501 Syntax: RCPT TO:<address>\r\n");
						g_slice_free(SMFEmailAddress_T,session->envelope_to[session->envelope_to_num]);
					} else {
						TRACE(TRACE_DEBUG,"session->envelope_to[%d]: %s",session->envelope_to_num, session->envelope_to[session->envelope_to_num]->addr);
						if (settings->backend != NULL) {
							smf_lookup_check_user(session->envelope_to[session->envelope_to_num]);
							TRACE(TRACE_DEBUG,"[%s] is local [%d]", 
									session->envelope_to[session->envelope_to_num]->addr,
									session->envelope_to[session->envelope_to_num]->is_local);
						} else
							session->envelope_to[session->envelope_to_num]->user_data = NULL;
						smtpd_code_reply(250);
						session->envelope_to_num++;
						state = ST_RCPT;
					}
				} else {
					smtpd_string_reply("501 Syntax: RCPT TO:<address>\r\n");
					g_slice_free(SMFEmailAddress_T,session->envelope_to[session->envelope_to_num]);
				}
			}
		} else if (g_ascii_strncasecmp(line,"data", 4)==0) {
			if ((state != ST_RCPT) && (state != ST_MAIL)) {
				/* someone wants to break smtp rules... */
				smtpd_string_reply("503 Error: need RCPT command\r\n");
			} else if ((state != ST_RCPT) && (state == ST_MAIL)) {
				/* we got the mail command but no rcpt to */
				smtpd_string_reply("554 Error: no valid recipients\r\n");
			} else {
				state = ST_DATA;
				TRACE(TRACE_DEBUG,"SMTP: 'data' received");
				process_data();
			}
		} else if (g_ascii_strncasecmp(line,"rset", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'rset' received");
			smf_session_free();
			/* reinit session */
			session = smf_session_get();
			smtpd_code_reply(250);
			state = ST_INIT;
		} else if (g_ascii_strncasecmp(line, "noop", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'noop' received");
			smtpd_code_reply(250);
		} else if (g_ascii_strcasecmp(line,"")!=0){
			TRACE(TRACE_DEBUG,"SMTP: command not recognized");
			smtpd_code_reply(502);
		} else {
			TRACE(TRACE_DEBUG,"SMTP: got empty line");
			smtpd_string_reply("500 Error: bad syntax\r\n");
		}
		g_free(line);
	} 
	
	g_io_channel_shutdown(in,TRUE,NULL);
	g_io_channel_unref(in);

	smf_session_free();
	
	return 0;
}

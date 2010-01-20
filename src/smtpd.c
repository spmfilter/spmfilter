#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>
#include <gmime/gmime.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "spmfilter.h"
#include "smtpd.h"

#define THIS_MODULE "smtpd"

MAILCONN *mconn;

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
void smtp_string_reply(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout,format,ap);
	va_end(ap);
	fflush(stdout);
}

/* smtp answer with smtp code as arg */
void smtp_code_reply(int code) {
	char *code_msg;
	char *str_code;
	SETTINGS *settings = get_settings();
	str_code = g_strdup_printf("%d",code);
	code_msg = smtp_code_get(settings->smtp_codes,code);
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
			case 500:
				fprintf(stdout,CODE_500);
				break;
			case 552:
				fprintf(stdout,CODE_552);
				break;
		}
	}
	fflush(stdout);
	g_free(str_code);
}

void load_modules(void) {
	GModule *module;
	LoadMod load_module;
	int i, ret;
	MESSAGE *msg = NULL;
	SETTINGS *settings = get_settings();

	for(i = 0; settings->modules[i] != NULL; i++) {
		gchar *path;	
		TRACE(TRACE_DEBUG,"loading module %s",settings->modules[i]);
		
		if (g_str_has_prefix(settings->modules[i],"lib")) {
#ifdef __APPLE__
			path = g_module_build_path(LIB_DIR,g_strdup_printf("%s.dylib",g_strstrip(settings->modules[i])));
#else
			path = g_module_build_path(LIB_DIR,g_strstrip(settings->modules[i]));
#endif
		} else {
#ifdef __APPLE__
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s.dylib",g_strstrip(settings->modules[i])));
#else
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",g_strstrip(settings->modules[i])));
#endif
		}
		module = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!module) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		}

		if (!g_module_symbol(module, "load", (gpointer *)&load_module)) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		}

		/* module return codes:
		 * -1 = Error in processing, spmfilter will send 4xx Error to MTA
		 * 0 = All ok, the next plugin will be started.
		 * 1 = Further processing will be stopped. Email is not going
		 *     to be delivered to nexthop!
		 * 2 = Further processing will be stopped, no other plugin will 
		 *     be startet. spmfilter sends a 250 code
		 */
		ret = load_module(mconn);
		TRACE(TRACE_DEBUG,"module return code [%d]", ret);
		if (ret == -1) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		} else if (ret == 1) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			smtp_string_reply(CODE_250_ACCEPTED);
			return;
		} else if (ret == 2) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			smtp_string_reply(CODE_250_ACCEPTED);
			break;
		} else if (ret >= 400) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
				
			smtp_code_reply(ret);
			return;
		}

		if (!g_module_close(module))
			TRACE(TRACE_ERR,"%s", g_module_error());
		
		g_free(path);
	}

	if (settings->nexthop != NULL ) {
		msg = g_slice_new(MESSAGE);
		msg->from = g_strdup(mconn->from->addr);
		msg->rcpts = g_malloc(sizeof(msg->rcpts[mconn->num_rcpts]));
		for (i = 0; i < mconn->num_rcpts; i++) {
			msg->rcpts[i] = g_strdup(mconn->rcpts[i]->addr);
		}

		msg->num_rcpts = mconn->num_rcpts;
		msg->message_file = g_strdup(mconn->queue_file);
		msg->nexthop = g_strup(settings->nexthop);
		if (smtp_delivery(msg) != 0) {
			TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
			smtp_string_reply(g_strdup_printf("%d %s\r\n",settings->nexthop_fail_code,settings->nexthop_fail_msg));
			return;
		}
		for (i = 0; i < mconn->num_rcpts; i++) {
			g_free(msg->rcpts[i]);
		}
		g_free(msg->from);
		g_free(msg->message_file);
		g_free(msg->nexthop);
		g_slice_free(MESSAGE,msg);
	}
	
	smtp_string_reply(CODE_250_ACCEPTED);
	return;
}

void process_data(void) {
	GIOChannel *in, *out;
	gchar *line;
	gsize length;
	GError *error = NULL;

	gen_queue_file(&mconn->queue_file);
	if (mconn->queue_file == NULL) {
		TRACE(TRACE_ERR,"failed to create spool file!");
		smtp_code_reply(552);
		return;
	}
		
	TRACE(TRACE_DEBUG,"using spool file: '%s'", mconn->queue_file);
		
	smtp_string_reply("354 End data with <CR><LF>.<CR><LF>\r\n");
	
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(mconn->queue_file,"w", &error)) == NULL) {
		smtp_string_reply("552 %s\r\n",error->message);
		g_error_free(error);
		return;
	}
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_encoding(out, NULL, NULL);

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, ".\r\n")==0)||(g_ascii_strcasecmp(line, ".\n")==0)) break;
		if (g_ascii_strncasecmp(line,".",1)==0) stuffing(line);
		
		if (g_io_channel_write_chars(out, line, -1, &length, &error) != G_IO_STATUS_NORMAL) {
			smtp_string_reply("452 %s\r\n",error->message);
			g_io_channel_unref(out);
			g_io_channel_shutdown(out,TRUE,NULL);
			g_io_channel_unref(in);
			g_free(line);
			remove(mconn->queue_file);
			g_error_free(error);
			return;
		}
		mconn->msgbodysize+=strlen(line);
		g_free(line);
	}
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(out);
	g_io_channel_unref(in);


	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)mconn->msgbodysize);

	load_modules();
	
	remove(mconn->queue_file);
	TRACE(TRACE_DEBUG,"removing spool file %s",mconn->queue_file);
	return;
}

int load(void) {
	char hostname[256];
	GIOChannel *in;
	char *line;
	int i;

	mconn = g_slice_new(MAILCONN);
	mconn->helo = NULL;
	mconn->from = NULL;
	mconn->queue_file = NULL;
	mconn->rcpts = NULL;
	mconn->xforward_addr = NULL;

	gethostname(hostname,256);


	smtp_string_reply("220 %s spmfilter\r\n",hostname);
	mconn->num_rcpts = 0;
	in = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(in, NULL, NULL);
	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_strstrip(line);
		TRACE(TRACE_DEBUG,"client smtp dialog: '%s'",line);
		
		if (g_ascii_strncasecmp(line,"quit",4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'quit' received"); 
			smtp_code_reply(221);
			break;
		} else if (g_ascii_strncasecmp(line, "helo", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'helo' received");
			mconn->helo = get_substring("^HELO\\s(.*)$",line, 1);
			TRACE(TRACE_DEBUG,"mconn->helo: %s",mconn->helo);
			smtp_string_reply("250 %s\r\n",hostname);
		} else if (g_ascii_strncasecmp(line, "ehlo", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'ehlo' received");
			mconn->helo = get_substring("^EHLO\\s(.*)$",line,1);
			TRACE(TRACE_DEBUG,"mconn->helo: %s",mconn->helo);
			smtp_string_reply("250-%s\r\n250-XFORWARD NAME ADDR PROTO HELO SOURCE\r\n250 DSN\r\n",hostname);
		} else if (g_ascii_strncasecmp(line,"xforward name",13)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'xforward name' received");
			mconn->xforward_addr = get_substring("^XFORWARD NAME=.* ADDR=(.*)$",line,1);
			TRACE(TRACE_DEBUG,"mconn->xforward_addr: %s",mconn->xforward_addr);
			smtp_code_reply(250);
		} else if (g_ascii_strncasecmp(line, "xforward proto", 13)==0) {
			smtp_code_reply(250);
		} else if (g_ascii_strncasecmp(line, "mail from:", 10)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'mail from' received");
			mconn->from = g_slice_new(EMLADDR);
			mconn->from->addr = get_substring("^MAIL FROM:(?:.*<)?([^>]*)(?:>)?", line, 1);
#ifdef HAVE_ZDB
			if (settings->sql_user_query != NULL) {
				mconn->from->is_local = sql_user_exists(mconn->from->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->from->addr,mconn->from->is_local);
			}
#endif
			smtp_code_reply(250);
			TRACE(TRACE_DEBUG,"mconn->from: %s",mconn->from->addr);
		} else if (g_ascii_strncasecmp(line, "rcpt to:", 8)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'rcpt to' received");
			mconn->rcpts = g_malloc(sizeof(mconn->rcpts[mconn->num_rcpts]));
			mconn->rcpts[mconn->num_rcpts] = g_malloc(sizeof(*mconn->rcpts[mconn->num_rcpts]));
			mconn->rcpts[mconn->num_rcpts]->addr = get_substring("^RCPT TO:(?:.*<)?([^>]*)(?:>)?", line, 1);
#ifdef HAVE_ZDB
			if (settings->sql_user_query != NULL) {
				mconn->rcpts[mconn->num_rcpts]->is_local = sql_user_exists(mconn->rcpts[mconn->num_rcpts]->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->rcpts[mconn->num_rcpts]->addr,mconn->rcpts[mconn->num_rcpts]->is_local);
			}
#endif
			smtp_code_reply(250);
			TRACE(TRACE_DEBUG,"mconn->rcpts[%d]: %s",mconn->num_rcpts, mconn->rcpts[mconn->num_rcpts]->addr);
			mconn->num_rcpts++;
		} else if (g_ascii_strncasecmp(line,"data", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'data' received");
			process_data();
		} else if (g_ascii_strncasecmp(line,"rset", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'rset' received");
			g_slice_free(MAILCONN,mconn);
			mconn = g_slice_new (MAILCONN);
			smtp_code_reply(250);
		} else if (g_ascii_strncasecmp(line, "noop", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'noop' received");
			smtp_code_reply(250);
		} else if (g_ascii_strcasecmp(line,"")!=0){
			TRACE(TRACE_DEBUG,"SMTP: wtf?!");
			smtp_code_reply(500);
		} else {
			break;
		}
		g_free(line);
	} 
	
	g_io_channel_shutdown(in,TRUE,NULL);
	g_io_channel_unref(in);

	g_free(mconn->queue_file);
	g_free(mconn->helo);
	g_free(mconn->xforward_addr);

	if (mconn->from != NULL)
		g_free(mconn->from->addr);
	g_slice_free(EMLADDR,mconn->from);
	for (i = 0; i < mconn->num_rcpts; i++) {
		g_free(mconn->rcpts[i]->addr);
		g_slice_free(EMLADDR,mconn->rcpts[i]);
	}
	g_slice_free(MAILCONN,mconn);

	return 0;
}

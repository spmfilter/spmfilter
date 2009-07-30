#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <glib.h>
#include <gmodule.h>
#include <gmime/gmime.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "spmfilter.h"
#include "smtpd.h"

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

/* smtp answer */
void smtp_chat_reply(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout,format,ap);
	va_end(ap);
	fflush(stdout);
}

void load_modules(SETTINGS *settings, MAILCONN *mconn) {
	GModule *module;
	LoadMod load_module;
	int i, ret;
	MESSAGE *msg = NULL;
	
	for(i = 0; settings->modules[i] != NULL; i++) {
		gchar *path;
		if (settings->debug)
			syslog(LOG_DEBUG,"loading module %s",settings->modules[i]);
		
		if (g_str_has_prefix(settings->modules[i],"lib")) {
			path = g_module_build_path(LIB_DIR,settings->modules[i]);
		} else {
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",settings->modules[i]));
		}
		module = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!module) {
			g_free(path);
			syslog(LOG_ERR,"%s\n", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_chat_reply(CODE_552);
						return;
				case 3: smtp_chat_reply(CODE_451);
						return;
			}
			return;
		}

		if (!g_module_symbol(module, "load", (gpointer *)&load_module)) {
			g_free(path);
			syslog(LOG_ERR,"%s\n", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_chat_reply(CODE_552);
						return;
				case 3: smtp_chat_reply(CODE_451);
						return;
			}
			return;
		}

		/* module return codes:
		 * -1 = Error in processing, spmfilter will send 4xx Error to MTA
		 * 0 = All ok, the next plugin will be started.
		 * 1 = Further processing will be stopped, no other plugin will 
		 *     be startet. spmfilter sends a 250 code
		 * 2 = Further processing will be stopped. Email is not going
		 *     to be delivered to nexthop!
		 */
		ret = load_module(settings,mconn); 
		if (ret == -1) {
			if (!g_module_close(module))
				syslog(LOG_ERR,"%s\n", g_module_error());
			g_free(path);
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_chat_reply(CODE_552);
						return;
				case 3: smtp_chat_reply(CODE_451);
						return;
			}
			return;
		} else if (ret == 1) {
			if (!g_module_close(module))
				syslog(LOG_ERR,"%s\n", g_module_error());
			g_free(path);
			smtp_chat_reply(CODE_250_ACCEPTED);
			break;
		} else if (ret == 2) {
			if (!g_module_close(module))
				syslog(LOG_ERR,"%s\n", g_module_error());
			g_free(path);
			smtp_chat_reply(CODE_250_ACCEPTED);
			return;
		}

		if (!g_module_close(module))
			syslog(LOG_ERR,"%s\n", g_module_error());
		
		g_free(path);
	}
	
	if (mconn->nexthop != NULL ) {
		msg = g_slice_new(MESSAGE);
		msg->from = g_strdup(mconn->from);
		msg->rcpt = mconn->rcpt;
		msg->message_file = g_strdup(mconn->queue_file);
		msg->nexthop = g_strup(mconn->nexthop);
		if (smtp_delivery(settings, msg) != 0) {
			syslog(LOG_ERR,"delivery to %s failed!",mconn->nexthop);
			smtp_chat_reply(g_strdup_printf("%d - %s\r\n",mconn->nexthop_fail_code,mconn->nexthop_fail_msg));
			return;
		}
		g_slice_free(MESSAGE,msg);
	}
	
	smtp_chat_reply(CODE_250_ACCEPTED);
	return;
}

void process_data(SETTINGS *settings, MAILCONN *mconn) {
	char *tempname;
	GIOChannel *in, *out;
	GMimeStream *gmin;
	GMimeMessage *message;
	GMimeParser *parser;
	gchar *line;
	gsize length;
	GError *error = NULL;
	HL *d;
	InternetAddressList *ia;
	InternetAddress *addr;
	
	/* create spooling file */
	tempname = g_strdup_printf("%s/spmfilter.XXXXXX",QUEUE_DIR);
	if (g_mkstemp(tempname) == -1) {
		g_free(tempname);
		syslog(LOG_ERR,"Can't create spooling file");
		smtp_chat_reply(CODE_451);
		return;
	}
	mconn->queue_file = g_strdup(tempname);
	
	if (settings->debug)
		syslog(LOG_DEBUG,"using spool file: '%s'", mconn->queue_file);
		
	smtp_chat_reply("354 End data with <CR><LF>.<CR><LF>\r\n");
	
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(mconn->queue_file,"w", &error)) == NULL) {
		smtp_chat_reply("552 - %s\r\n",error->message);
		return;
	}
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_encoding(out, NULL, NULL);

	/* initialize GMime */
	g_mime_init (0);
	gmin = g_mime_stream_mem_new();
	
	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, ".\r\n")==0)||(g_ascii_strcasecmp(line, ".\n")==0)) break;
		if (g_ascii_strncasecmp(line,".",1)==0) stuffing(line);
		
		if (g_io_channel_write_chars(out, line, -1, &length, &error) != G_IO_STATUS_NORMAL) {
			smtp_chat_reply("452 - %s\r\n",error->message);
			g_io_channel_shutdown(out,TRUE,NULL);
			g_io_channel_unref(in);
			g_io_channel_unref(out);
			g_free(line);
			remove(mconn->queue_file);
			return;
		}
		mconn->msgbodysize+=strlen(line);
		g_mime_stream_write_string(gmin,line);
		g_free(line);
	}
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(in);
	g_io_channel_unref(out);

	if (settings->debug)
		syslog(LOG_DEBUG,"data complete, message size: %d", (u_int32_t)mconn->msgbodysize);
		
	g_mime_stream_seek(gmin,0,0);
	parser = g_mime_parser_new_with_stream (gmin);
	g_object_unref(gmin);
	
	/* check header */
	d = g_slice_new(HL);
	d->mconn = mconn;
	d->settings = settings;
	d->message = message;
	g_hash_table_foreach(mconn->header_checks,header_check,d);
	g_slice_free(HL,d);
		
	load_modules(settings,mconn);
	
	remove(mconn->queue_file);
	if(settings->debug)
		syslog(LOG_DEBUG,"removing spool file %s",mconn->queue_file);
	return;
}

int load(SETTINGS *settings,MAILCONN *mconn) {
	int state=ST_INIT;
	char hostname[256];
	char line[512];
	
	gethostname(hostname,256);

	smtp_chat_reply("220 %s spmfilter\r\n",hostname);
	
	do {
		memset(line, 0, sizeof(line));
		fgets(line, sizeof(line), stdin);
		
		g_strstrip(line);
		if (settings->debug) 
			syslog(LOG_DEBUG,"client smtp dialog: '%s'",line);
		
		if (g_ascii_strncasecmp(line,"quit",4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'quit' received"); 
			smtp_chat_reply(CODE_221);
			break;
		} else if (g_ascii_strncasecmp(line, "helo", 4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'helo' received");
			mconn->helo = get_substring("^HELO\\s(.*)$",line, 1);
			if (settings->debug)
				syslog(LOG_DEBUG,"mconn->helo: %s",mconn->helo);
			smtp_chat_reply("250 %s\r\n",hostname);
			state = ST_HELO;
		} else if (g_ascii_strncasecmp(line, "ehlo", 4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'ehlo' received");
			mconn->helo = get_substring("^EHLO\\s(.*)$",line,1);
			if (settings->debug)
				syslog(LOG_DEBUG,"mconn->helo: %s",mconn->helo);
			smtp_chat_reply("250-%s\r\n250-XFORWARD NAME ADDR PROTO HELO SOURCE\r\n250 DSN\r\n",hostname);
			state = ST_HELO;
		} else if (g_ascii_strncasecmp(line,"xforward name",13)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'xforward name' received");
			mconn->xforward_addr = get_substring("^XFORWARD NAME=.* ADDR=(.*)$",line,1);
			if (settings->debug)
				syslog(LOG_DEBUG,"mconn->xforward_addr: %s",mconn->xforward_addr);
			smtp_chat_reply(CODE_250);
		} else if (g_ascii_strncasecmp(line, "xforward proto", 13)==0) {
			smtp_chat_reply(CODE_250);
		} else if (g_ascii_strncasecmp(line, "mail from:", 10)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'mail from' received");
			state = ST_MAIL;
			mconn->from = get_substring("^MAIL FROM:(?:.*<)?([^>]*)(?:>)?", line, 1);
			smtp_chat_reply(CODE_250);
			if (settings->debug)
				syslog(LOG_DEBUG,"mconn->from: %s",mconn->from);
		} else if (g_ascii_strncasecmp(line, "rcpt to:", 8)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'rcpt to' received");
			state = ST_RCPT;
			mconn->rcpt = g_slist_append(mconn->rcpt,get_substring("^RCPT TO:(?:.*<)?([^>]*)(?:>)?", line, 1));
			smtp_chat_reply(CODE_250);
			if (settings->debug)
				syslog(LOG_DEBUG,"mconn->rcpt[%d]: %s",
					g_slist_length(mconn->rcpt)-1,
					g_slist_nth_data(mconn->rcpt,g_slist_length(mconn->rcpt)-1));
		} else if (g_ascii_strncasecmp(line,"data", 4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'data' received");
			state = ST_DATA;
			process_data(settings,mconn);
		} else if (g_ascii_strncasecmp(line,"rset", 4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'rset' received");
			g_slice_free(MAILCONN,mconn);
			mconn = g_slice_new (MAILCONN);
			state = ST_INIT;
			smtp_chat_reply(CODE_250);
		} else if (g_ascii_strncasecmp(line, "noop", 4)==0) {
			if (settings->debug)
				syslog(LOG_DEBUG,"SMTP: 'noop' received");
			smtp_chat_reply(CODE_250);
		} else if (g_ascii_strcasecmp(line,"")!=0){
			if(settings->debug)
				syslog(LOG_DEBUG,"got wtf");
			smtp_chat_reply(CODE_500);
		} else {
			break;
		}
	} while (1);
	
	return 0;
}

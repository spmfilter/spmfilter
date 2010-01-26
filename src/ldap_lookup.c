#include <glib.h>
#include <ldap.h>
#include <stdarg.h>

#include "spmfilter.h"

#define THIS_MODULE "ldap_lookup"

LDAP *ld = NULL;

int get_scope(void) {
	Settings_T *settings = get_settings();

	if (g_ascii_strcasecmp(settings->ldap_scope,"subtree") == 0)
		return LDAP_SCOPE_SUBTREE;
	else if (g_ascii_strcasecmp(settings->ldap_scope,"onelevel") == 0)
		return LDAP_SCOPE_ONELEVEL;
	else if (g_ascii_strcasecmp(settings->ldap_scope,"base") == 0)
		return LDAP_SCOPE_BASE;
	else
		return LDAP_SCOPE_SUBTREE;
}

int ldap_connect(void) {
	int ret, err;
	int version;
	char *uri;
	Settings_T *settings = get_settings();

	TRACE(TRACE_LOOKUP, "connecting to ldap server on [%s]",settings->ldap_host);
	if (settings->ldap_uri) {
		if ((ret = ldap_initialize(&ld, settings->ldap_uri) != LDAP_SUCCESS)) 
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
	} else {
		uri = g_strdup_printf("ldap://%s:%d",settings->ldap_host,settings->ldap_port);
		if ((ret = ldap_initialize(&ld, uri)) != LDAP_SUCCESS) 
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);

		g_free(uri);
	}
	version = LDAP_VERSION3;
	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);

	if (settings->ldap_referrals) {
		ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_ON);
		TRACE(TRACE_LOOKUP, "set ldap referrals to on");
	} else {
		ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_OFF);
		TRACE(TRACE_LOOKUP, "set ldap referrals to off");
	}
	
	TRACE(TRACE_LOOKUP, "binding to ldap server as [%s] / [xxxxxxxx]",  settings->ldap_binddn);
	
	if ((err = ldap_bind_s(ld, settings->ldap_binddn, settings->ldap_bindpw, LDAP_AUTH_SIMPLE))) {
		TRACE(TRACE_ERR, "ldap_bind_s failed: %s",  ldap_err2string(err));
		return -1;
	}
	TRACE(TRACE_LOOKUP, "successfully bound to ldap server" );
	
	return 0;
}

LDAP *ldap_con_get(void) {
	if (!ld) {
		ldap_connect();
	}
	return ld;
}

void ldap_disconnect(void) {
	LDAP *c = ldap_con_get();
	if (c != NULL) {
		ldap_unbind_s(c);
		TRACE(TRACE_LOOKUP, "unbind ldap server");
	}
}

LDAPMessage *ldap_query(const char *q, ...) {
	va_list ap, cp;
	char *query;
	LDAPMessage *msg = NULL;
	LDAP *c = ldap_con_get();
	Settings_T *settings = get_settings();


	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
	
	TRACE(TRACE_LOOKUP,"[%p] [%s]",ld,query);
	
	if (ldap_search_s(c,settings->ldap_base,LDAP_SCOPE_SUBTREE,query,NULL,0,&msg) != LDAP_SUCCESS)
		TRACE(TRACE_ERR,"[%p] query [%s] failed",ld, query);
	
	if(ldap_count_entries(c,msg) <= 0)
		TRACE(TRACE_LOOKUP,"[%p] nothing found",ld);
	
	return msg;
}

/** Check if given user exists in ldap directory
 *
 * \param addr email adress of user
 *
 * \return 1 if the user exists, otherwise 0
 */
int ldap_user_exists(char *addr) {
	char *query;
	LDAP *c = ldap_con_get();
	LDAPMessage *msg = NULL;
	Settings_T *settings = get_settings();

	if (expand_query(settings->ldap_user_query, addr, &query) <= 0) {
		TRACE(TRACE_ERR,"failed to expand ldap query");
		return -1;
	}
	TRACE(TRACE_LOOKUP,"[%p] [%s]",ld,query);

	if (ldap_search_s(c,settings->ldap_base,get_scope(),query,NULL,0,&msg) != LDAP_SUCCESS) {
		TRACE(TRACE_ERR,"[%p] query [%s] failed",c, query);
		ldap_msgfree(msg);
		return -1;
	}

	if(ldap_count_entries(c,msg) > 0) {
		TRACE(TRACE_LOOKUP, "found user [%s]",addr);
		ldap_msgfree(msg);
		return 1;
	} else {
		TRACE(TRACE_LOOKUP, "user [%s] does not exist", addr);
		ldap_msgfree(msg);
		return 0;
	}
}

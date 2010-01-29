#include <glib.h>
#include <ldap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "spmfilter.h"
#include "lookup_result.h"

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

/** Get random ldap host
 *
 * \returns hostname of ldap host
 */
char *ldap_get_rand_host(void) {
	Settings_T *settings = get_settings();
	TRACE(TRACE_DEBUG,"trying to get random ldap server");
	srand(time(NULL));
	return settings->ldap_host[rand() % settings->ldap_num_hosts];
}

/** Try to get a failover connection to other server
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_failover_connect(void) {
	int i, ret, err;
	char *uri;
	Settings_T *settings = get_settings();

	TRACE(TRACE_WARNING,"trying ldap failover connection");
	for (i=0; i < settings->ldap_num_hosts; i++) {
		uri = g_strdup_printf("ldap://%s:%d",settings->ldap_host[i],settings->ldap_port);
		if ((ret = ldap_initialize(&ld, uri)) == LDAP_SUCCESS) {
			TRACE(TRACE_DEBUG,"ldap_initialize() to failover host [%s] successful ", settings->ldap_host[i]);
			TRACE(TRACE_LOOKUP, "binding to ldap server as [%s] / [xxxxxxxx]",  settings->ldap_binddn);
			if ((err = ldap_bind_s(ld, settings->ldap_binddn, settings->ldap_bindpw, LDAP_AUTH_SIMPLE))) {
				TRACE(TRACE_ERR, "ldap_bind_s on failover host [%s] failed: %s", settings->ldap_host[i], ldap_err2string(err));
				continue;
			}
			TRACE(TRACE_LOOKUP, "successfully bound to failover host [%s]", settings->ldap_host[i]);
			g_free(uri);
			return 0;
		} else {
			TRACE(TRACE_ERR, "ldap_initialize() to failover host [%s] returned [%d]", settings->ldap_host[i], ret);
		}
		g_free(uri);
	}

	return -1;
}

/** Connect to LDAP server
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_connect(void) {
	int ret, err;
	int version;
	char *uri;
	char *host;
	Settings_T *settings = get_settings();

	if (settings->ldap_uri) {
		if ((ret = ldap_initialize(&ld, settings->ldap_uri) != LDAP_SUCCESS)) 
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
	} else {
		if (g_ascii_strcasecmp(settings->backend_connection,"balance") == 0) {
			host = ldap_get_rand_host();
		} else
			host = settings->ldap_host[0];
		
		TRACE(TRACE_LOOKUP, "connecting to ldap server on [%s]",host);
		uri = g_strdup_printf("ldap://%s:%d",host,settings->ldap_port);
		if ((ret = ldap_initialize(&ld, uri)) != LDAP_SUCCESS) {
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
		}
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
	
	
	if (ldap_failover_connect() != 0) {
		TRACE(TRACE_ERR,"failover connection failed");
		return -1;
	} else
		return 0;
}

/** Get active LDAP connection, if no connection
 *  is available reconnect to LDAP server.
 *
 * \returns pointer to LDAP connection
 */
LDAP *ldap_con_get(void) {
	if (!ld) {
		ldap_connect();
	}
	return ld;
}

/** Disconnect from LDAP server */
void ldap_disconnect(void) {
	LDAP *c = ldap_con_get();
	if (c != NULL) {
		ldap_unbind_s(c);
		TRACE(TRACE_LOOKUP, "unbind ldap server");
	}
}

LookupResult_T *ldap_query(const char *q, ...) {
	LookupResult_T *result = lookup_result_new();
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
	
	if (ldap_search_s(c,settings->ldap_base,get_scope(),query,NULL,0,&msg) != LDAP_SUCCESS)
		TRACE(TRACE_ERR,"[%p] query [%s] failed",ld, query);
	
	if(ldap_count_entries(c,msg) <= 0) {
		TRACE(TRACE_LOOKUP,"[%p] nothing found",ld);
		return NULL;
	} 

	return result;
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

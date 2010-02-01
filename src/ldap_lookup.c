#include <glib.h>
#include <lber.h>
#include <ldap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "spmfilter.h"
#include "lookup.h"

#define THIS_MODULE "ldap_lookup"

LDAP *ld = NULL;

// TODO: add number of vals to LookupElement_T, there can be more values for one attribute

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

/** Generate LDAP connectio uri
 *
 * \param host to bind
 *
 * \returns new allocated uri
 */
char *ldap_get_uri(char *host) {
	char *uri;
	Settings_T *settings = get_settings();

	uri = g_strdup_printf("ldap://%s:%d",host,settings->ldap_port);

	return uri;
}

/** Bind to LDAP Server
 *
 * \param uri connection uri
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_bind(char *uri) {
	int ret, err;
	struct berval *cred;
	Settings_T *settings = get_settings();

	cred = malloc(sizeof(struct berval));
	cred->bv_len = strlen(settings->ldap_bindpw);
	cred->bv_val = settings->ldap_bindpw;

	if ((ret = ldap_initialize(&ld, uri)) == LDAP_SUCCESS) {
		TRACE(TRACE_DEBUG,"ldap_initialize() to host [%s] successful ", uri);
		TRACE(TRACE_LOOKUP, "binding to ldap server as [%s] / [xxxxxxxx]",  settings->ldap_binddn);
		if ((err = ldap_sasl_bind_s(ld,settings->ldap_binddn,LDAP_SASL_SIMPLE,cred,NULL,NULL,NULL))) {
			TRACE(TRACE_ERR, "ldap_sasl_bind_s on host [%s] failed: %s", uri, ldap_err2string(err));
			return -1;
		}
		TRACE(TRACE_LOOKUP, "successfully bound to host [%s]", uri);
		free(cred);
		g_free(uri);
		return 0;
	} 

	free(cred);
	g_free(uri);
	return -1;
}

/** Try to get a failover connection to other server
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_failover_connect(void) {
	int i;
	char *uri;
	Settings_T *settings = get_settings();
	
	for (i=0; i < settings->ldap_num_hosts; i++) {
		uri = ldap_get_uri(settings->ldap_host[i]);
		if (ldap_bind(uri) != 0) {
			continue;
		} else {
			return 0;
		}
	}

	return -1;
}

/** Connect to LDAP server
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_connect(void) {
	int ret;
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
		
		uri = ldap_get_uri(host);
		if ((ret = ldap_initialize(&ld, uri)) != LDAP_SUCCESS) {
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
		}
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

	if (ldap_bind(uri) != 0) {
		if (ldap_failover_connect() != 0) {
			TRACE(TRACE_ERR,"failover connection failed");
			return -1;
		}
	}

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
		ldap_unbind_ext_s(c,NULL,NULL);
		TRACE(TRACE_LOOKUP, "unbind ldap server");
	}
}

LookupResult_T *ldap_query(const char *q, ...) {
	LookupResult_T *result = lookup_result_new();
	va_list ap, cp;
	char *query;
	LDAPMessage *msg = NULL;
	LDAPMessage *entry = NULL;
	char *attr;
	struct berval **bvals;
	BerElement *ptr;
	LDAP *c = ldap_con_get();
	Settings_T *settings = get_settings();


	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
	
	TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
	
	if (ldap_search_ext_s(c,settings->ldap_base,get_scope(),query,NULL,0,NULL, NULL, NULL, 0, &msg) != LDAP_SUCCESS)
		TRACE(TRACE_ERR,"[%p] query [%s] failed",ld, query);
	
	if(ldap_count_entries(c,msg) <= 0) {
		TRACE(TRACE_LOOKUP,"[%p] nothing found",c);
		return NULL;
	} else
		TRACE(TRACE_LOOKUP,"[%p] found [%d] entries", c, ldap_count_entries(c,msg));

	for (entry = ldap_first_entry(c, msg); entry != NULL; entry = ldap_next_entry(c,entry)) {
		LookupElement_T *e = lookup_element_new();

		for(attr = ldap_first_attribute(c, msg, &ptr); attr != NULL;
				attr = ldap_next_attribute(c, msg, ptr)) {

			bvals = ldap_get_values_len(c, entry, attr);

			TRACE(TRACE_DEBUG,"VALUE: %s",(char *)((struct berval)*bvals[0]).bv_val); //<- TESTSHIT
			//values = ldap_get_values(c,entry,attr);
			TRACE(TRACE_DEBUG,"found attribute [%s] entry [%p]", attr, entry);
			lookup_element_insert(e,attr,bvals);
//			ldap_value_free(values);
			free(attr);
		}
		result = lookup_result_append(result,e);
	}
	ber_free(ptr,0);
	ldap_msgfree(msg);
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

	/* TESTCODE */
	LookupResult_T *r = NULL;

	r = ldap_query("(mail=%s)",addr);
	for (r = lookup_result_first(r); r; r = lookup_result_next(r)) {
		TRACE(TRACE_DEBUG,"Mail: %s\n",(char *)lookup_get_element(r,"mail"));
		r = lookup_result_next(r);
	}
	/* END TESTCODe */

	if (ldap_search_ext_s(c,settings->ldap_base,get_scope(),query,NULL,0,NULL, NULL, NULL, 0, &msg) != LDAP_SUCCESS) {
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

/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Aymeric MOIZARD jack@atosc.org
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>
#include <stdlib.h>
#include <stdio.h>

/* allocate a new url structure */
/* OUTPUT: osip_uri_t *url | structure to save results.   */
/* OUTPUT: err_t *err | structure to store error.   */
/* return -1 on error */
int osip_uri_init(osip_uri_t * url)
{
	url->scheme[0]='\0';
	url->username[0]='\0';
    url->password[0]='\0';
    url->host[0]='\0';
    url->port=0;
    url->string[0]='\0';
   	osip_list_init(&(url->url_params));

	osip_list_init(&(url->url_headers));
	return OSIP_SUCCESS;
}

/* examples:
   sip:j.doe@big.com;maddr=239.255.255.1;ttl=15
   sip:j.doe@big.com
   sip:j.doe:secret@big.com;transport=tcp
   sip:j.doe@big.com?subject=project
   sip:+1-212-555-1212:1234@gateway.com;user=phone
   sip:1212@gateway.com
   sip:alice@10.1.2.3
   sip:alice@example.com
   sip:alice@registrar.com;method=REGISTER

   NOT EQUIVALENT:
   SIP:JUSER@ExAmPlE.CoM;Transport=udp
   sip:juser@ExAmPlE.CoM;Transport=UDP
*/

/* this method search for the separator and   */
/* return it only if it is located before the */
/* second separator. */
char *next_separator(const char *ch, int separator_osip_to_find,
					 int before_separator)
{
	char *ind;
	char *tmp;

	ind = strchr(ch, separator_osip_to_find);
	if (ind == NULL)
		return NULL;

	tmp = NULL;
	if (before_separator != 0)
		tmp = strchr(ch, before_separator);

	if (tmp != NULL) {
		if (ind < tmp)
			return ind;
	} else
		return ind;

	return NULL;
}

/* parse the sip url.                                */
/* INPUT : char *buf | url to be parsed.*/
/* OUTPUT: osip_uri_t *url | structure to save results.   */
/* OUTPUT: err_t *err | structure to store error.   */
/* return -1 on error */
int osip_uri_parse(osip_uri_t * url, const char *buf)
{
	char *username;
	char *password;
	char *host;
	const char *port;
	const char *params;
	const char *headers;
	const char *tmp;

	/* basic tests */
	if (buf == NULL || buf[0] == '\0')
		return OSIP_BADPARAMETER;

	tmp = strchr(buf, ':');
	if (tmp == NULL)
		return OSIP_SYNTAXERROR;

	if (tmp - buf < 2)
		return OSIP_SYNTAXERROR;
	osip_strncpy(url->scheme, buf, tmp - buf);	

	if (strchr(url->scheme, ' ') != NULL) {
		return OSIP_SYNTAXERROR;
	}

	if (strlen(url->scheme) < 3 || (0 != osip_strncasecmp(url->scheme, "sip", 3)
									&& 0 != osip_strncasecmp(url->scheme, "sips", 4))) {	/* Is not a sipurl ! */
		size_t i = strlen(tmp + 1);

		if (i < 2)
			return OSIP_SYNTAXERROR;
		osip_strncpy(url->string, tmp + 1, i);
		return OSIP_SUCCESS;
	}

	/*  law number 1:
	   if ('?' exists && is_located_after '@')
	   or   if ('?' exists && '@' is not there -no username-)
	   =====>  HEADER_PARAM EXIST
	   =====>  start at index(?)
	   =====>  end at the end of url
	 */

	/* find the beginning of host */
	username = strchr(buf, ':');
	/* if ':' does not exist, the url is not valid */
	if (username == NULL)
		return OSIP_SYNTAXERROR;

	host = strchr(buf, '@');

	if (host == NULL)
		host = username;
	else if (username[1] == '@')	/* username is empty */
		host = username + 1;
	else
		/* username exists */
	{
		password = next_separator(username + 1, ':', '@');
		if (password == NULL)
			password = host;
		else
			/* password exists */
		{
			if (host - password < 2)
				return OSIP_SYNTAXERROR;
			osip_strncpy(url->password, password + 1, host - password - 1);
			__osip_uri_unescape(url->password);
		}
		if (password - username < 2)
			return OSIP_SYNTAXERROR;
		{
			osip_strncpy(url->username, username + 1, password - username - 1);
			__osip_uri_unescape(url->username);
		}
	}


	/* search for header after host */
	headers = strchr(host, '?');

	if (headers == NULL)
		headers = buf + strlen(buf);
	else
		/* headers exist */
		osip_uri_parse_headers(url, headers);


	/* search for params after host */
	params = strchr(host, ';');	/* search for params after host */
	if (params == NULL)
		params = headers;
	else
		/* params exist */
	{
		char *tmpbuf;

		if (headers - params + 1 < 2)
			return OSIP_SYNTAXERROR;
		tmpbuf = osip_malloc(headers - params + 1);
		if (tmpbuf == NULL)
			return OSIP_NOMEM;
		tmpbuf = osip_strncpy(tmpbuf, params, headers - params);
		osip_uri_parse_params(url, tmpbuf);
		osip_free(tmpbuf);
	}

	port = params - 1;
	while (port > host && *port != ']' && *port != ':')
		port--;
	if (*port == ':') {
		if (host == port)
			port = params;
		else {
			char tmp[4];
			if ((params - port < 2) || (params - port > 8))
			    return OSIP_SYNTAXERROR;    /* error cases */
            osip_clrncpy(tmp, port + 1, params - port - 1);
            url->port = osip_atoi(tmp);
		}
	} else
		port = params;
	/* adjust port for ipv6 address */
	tmp = port;
	while (tmp > host && *tmp != ']')
		tmp--;
	if (*tmp == ']') {
		port = tmp;
		while (host < port && *host != '[')
			host++;
		if (host >= port)
			return OSIP_SYNTAXERROR;
	}

	if (port - host < 2)
		return OSIP_SYNTAXERROR;
	osip_clrncpy(url->host, host + 1, port - host - 1);

	return OSIP_SUCCESS;
}

void osip_uri_set_scheme(osip_uri_t * url, char *scheme)
{
	if (url == NULL)
		return;
	osip_strncpy(url->scheme, scheme, strlen(url->scheme)-1);
}

char *osip_uri_get_scheme(osip_uri_t * url)
{
	if (url == NULL)
		return NULL;
	return url->scheme;
}

void osip_uri_set_username(osip_uri_t * url, char *username)
{
	if (url == NULL)
		return;
    osip_strncpy(url->username, username, strlen(url->username)-1);
}

char *osip_uri_get_username(osip_uri_t * url)
{
	if (url == NULL)
		return NULL;
	return url->username;
}

void osip_uri_set_password(osip_uri_t * url, char *password)
{
	if (url == NULL)
		return;
    osip_strncpy(url->password, password, strlen(url->password)-1);
}

char *osip_uri_get_password(osip_uri_t * url)
{
	if (url == NULL)
		return NULL;
	return url->password;
}

void osip_uri_set_host(osip_uri_t * url, char *host)
{
	if (url == NULL)
		return;
    osip_strncpy(url->host, host, strlen(url->host)-1);
}

char *osip_uri_get_host(osip_uri_t * url)
{
	if (url == NULL)
		return NULL;
	return url->host;
}

void osip_uri_set_port(osip_uri_t * url, char *port)
{
	if (url == NULL)
		return;
	if(port != NULL)
	    url->port = atoi(port);
}

int osip_uri_get_port(osip_uri_t * url)
{
	if (url == NULL)
		return 0;
	return url->port;
}

int osip_uri_parse_headers(osip_uri_t * url, const char *headers)
{
	char *and;
	char *equal;

	/* find '=' wich is the separator for one header */
	/* find ';' wich is the separator for multiple headers */

	equal = strchr(headers, '=');
	and = strchr(headers + 1, '&');

	if (equal == NULL)			/* each header MUST have a value */
		return OSIP_SYNTAXERROR;

	do {
		char *hname;
		char *hvalue;

		hname = (char *) osip_malloc(equal - headers);
		if (hname == NULL)
			return OSIP_NOMEM;
		osip_strncpy(hname, headers + 1, equal - headers - 1);
		__osip_uri_unescape(hname);

		if (and != NULL) {
			if (and - equal < 2) {
				osip_free(hname);
				return OSIP_SYNTAXERROR;
			}
			hvalue = (char *) osip_malloc(and - equal);
			if (hvalue == NULL) {
				osip_free(hname);
				return OSIP_NOMEM;
			}
			osip_strncpy(hvalue, equal + 1, and - equal - 1);
			__osip_uri_unescape(hvalue);
		} else {				/* this is for the last header (no and...) */
			if (headers + strlen(headers) - equal + 1 < 2) {
				osip_free(hname);
				return OSIP_SYNTAXERROR;
			}
			hvalue = (char *) osip_malloc(headers + strlen(headers) - equal + 1);
			if (hvalue == NULL) {
				osip_free(hname);
				return OSIP_NOMEM;
			}
			osip_strncpy(hvalue, equal + 1, headers + strlen(headers) - equal);
			__osip_uri_unescape(hvalue);
		}

		osip_uri_uheader_add(url, hname, hvalue);

		if (and == NULL)		/* we just set the last header */
			equal = NULL;
		else {					/* continue on next header */

			headers = and;
			equal = strchr(headers, '=');
			and = strchr(headers + 1, '&');
			if (equal == NULL)	/* each header MUST have a value */
				return OSIP_SYNTAXERROR;
		}
	}
	while (equal != NULL);
	return OSIP_SUCCESS;
}

int osip_uri_parse_params(osip_uri_t * url, const char *params)
{
	char *pname;
	char *pvalue;

	const char *comma;
	const char *equal;

	/* find '=' wich is the separator for one param */
	/* find ';' wich is the separator for multiple params */

	equal = next_separator(params + 1, '=', ';');
	comma = strchr(params + 1, ';');

	while (comma != NULL) {
		if (equal == NULL) {
			equal = comma;
			pvalue = NULL;
		} else {
			if (comma - equal < 2)
				return OSIP_SYNTAXERROR;
			pvalue = (char *) osip_malloc(comma - equal);
			if (pvalue == NULL)
				return OSIP_NOMEM;
			osip_strncpy(pvalue, equal + 1, comma - equal - 1);
			__osip_uri_unescape(pvalue);
		}

		if (equal - params < 2) {
			osip_free(pvalue);
			return OSIP_SYNTAXERROR;
		}
		pname = (char *) osip_malloc(equal - params);
		if (pname == NULL) {
			osip_free(pvalue);
			return OSIP_NOMEM;
		}
		osip_strncpy(pname, params + 1, equal - params - 1);
		__osip_uri_unescape(pname);

		osip_uri_uparam_add(url, pname, pvalue);

		params = comma;
		equal = next_separator(params + 1, '=', ';');
		comma = strchr(params + 1, ';');
	}

	/* this is the last header (comma==NULL) */
	comma = params + strlen(params);

	if (equal == NULL) {
		equal = comma;			/* at the end */
		pvalue = NULL;
	} else {
		if (comma - equal < 2)
			return OSIP_SYNTAXERROR;
		pvalue = (char *) osip_malloc(comma - equal);
		if (pvalue == NULL)
			return OSIP_NOMEM;
		osip_strncpy(pvalue, equal + 1, comma - equal - 1);
		__osip_uri_unescape(pvalue);
	}

	if (equal - params < 2) {
		osip_free(pvalue);
		return OSIP_SYNTAXERROR;
	}
	pname = (char *) osip_malloc(equal - params);
	if (pname == NULL) {
		osip_free(pvalue);
		return OSIP_NOMEM;
	}
	osip_strncpy(pname, params + 1, equal - params - 1);
	__osip_uri_unescape(pname);

	osip_uri_uparam_add(url, pname, pvalue);

	return OSIP_SUCCESS;
}

int osip_uri_to_str(const osip_uri_t * url, char **dest)
{
	char *buf;
	size_t len;
	size_t plen;
	char *tmp;
	const char *scheme;

	*dest = NULL;
	if (url == NULL)
		return OSIP_BADPARAMETER;
	if (url->host[0] == '\0' && url->string[0] == '\0')
		return OSIP_BADPARAMETER;
	if (url->scheme[0] == '\0' && url->string[0] != '\0')
		return OSIP_BADPARAMETER;
	if (url->string[0] == '\0' && url->scheme[0] == '\0')
		scheme = "sip";			/* default is sipurl */
	else
		scheme = url->scheme;

	if (url->string[0] != '\0') 
    {
		buf = (char *) osip_malloc(strlen(scheme) + strlen(url->string) + 3);
		if (buf == NULL)
			return OSIP_NOMEM;
		*dest = buf;
		sprintf(buf, "%s:", scheme);
		buf = buf + strlen(scheme) + 1;
		sprintf(buf, "%s", url->string);
		buf = buf + strlen(url->string);
		return OSIP_SUCCESS;
	}

	len = strlen(scheme) + 1 + strlen(url->host) + 5;
	if (url->username != NULL)
		len = len + (strlen(url->username) * 3) + 1;	/* count escaped char */
	if (url->password != NULL)
		len = len + (strlen(url->password) * 3) + 1;

	buf = (char *) osip_malloc(len);
	if (buf == NULL)
		return OSIP_NOMEM;
	tmp = buf;

	sprintf(tmp, "%s:", scheme);
	tmp = tmp + strlen(tmp);

	if (url->username[0] != '\0') 
    {
		char *tmp2 = __osip_uri_escape_userinfo(url->username);

		sprintf(tmp, "%s", tmp2);
		osip_free(tmp2);
		tmp = tmp + strlen(tmp);
	}

	if ((url->password[0] != '\0') && (url->username[0] != '\0')) 	/* be sure that when a password is given, a username is also given */
	{
		char *tmp2 = __osip_uri_escape_password(url->password);

		sprintf(tmp, ":%s", tmp2);
		osip_free(tmp2);
		tmp = tmp + strlen(tmp);
	}

	if (url->username[0] != '\0') 	/* we add a '@' only when username is present... */
	{
		sprintf(tmp, "@");
		tmp++;
	}
	if (strchr(url->host, ':') != NULL) {
		sprintf(tmp, "[%s]", url->host);
		tmp = tmp + strlen(tmp);
	} else {
		sprintf(tmp, "%s", url->host);
		tmp = tmp + strlen(tmp);
	}

	if (url->port > 0)     
    {
		sprintf(tmp, ":%d", url->port);
		tmp = tmp + strlen(tmp);
	}
	{
		int pos = 0;
		osip_uri_param_t *u_param;

		while (!osip_list_eol(&url->url_params, pos)) {
			char *tmp1;
			char *tmp2 = NULL;

			u_param = (osip_uri_param_t *) osip_list_get(&url->url_params, pos);

			tmp1 = __osip_uri_escape_uri_param(u_param->gname);
			if (u_param->gvalue == NULL)
				plen = strlen(tmp1) + 2;
			else {
				tmp2 = __osip_uri_escape_uri_param(u_param->gvalue);
				plen = strlen(tmp1) + strlen(tmp2) + 3;
			}
			len = len + plen;
			buf = (char *) osip_realloc(buf, len);
			tmp = buf;
			tmp = tmp + strlen(tmp);
			if (u_param->gvalue == NULL)
				sprintf(tmp, ";%s", tmp1);
			else {
				sprintf(tmp, ";%s=%s", tmp1, tmp2);
				osip_free(tmp2);
			}
			osip_free(tmp1);
			pos++;
		}
	}

	{
		int pos = 0;
		osip_uri_header_t *u_header;

		while (!osip_list_eol(&url->url_headers, pos)) {
			char *tmp1;
			char *tmp2;

			u_header = (osip_uri_header_t *) osip_list_get(&url->url_headers, pos);
			tmp1 = __osip_uri_escape_header_param(u_header->gname);

			if (tmp1 == NULL) {
				osip_free(buf);
				return OSIP_SYNTAXERROR;
			}

			tmp2 = __osip_uri_escape_header_param(u_header->gvalue);

			if (tmp2 == NULL) {
				osip_free(tmp1);
				osip_free(buf);
				return OSIP_SYNTAXERROR;
			}
			plen = strlen(tmp1) + strlen(tmp2) + 4;

			len = len + plen;
			buf = (char *) osip_realloc(buf, len);
			tmp = buf;
			tmp = tmp + strlen(tmp);
			if (pos == 0)
				snprintf(tmp, len - (tmp-buf), "?%s=%s", tmp1, tmp2);
			else
				snprintf(tmp, len - (tmp-buf), "&%s=%s", tmp1, tmp2);
			osip_free(tmp1);
			osip_free(tmp2);
			pos++;
		}
	}

	*dest = buf;
	return OSIP_SUCCESS;
}


void osip_uri_free(osip_uri_t * url)
{
	int pos = 0;

	if (url == NULL)
		return;
	url->scheme[0]='\0';
	url->username[0]='\0';
    url->password[0]='\0';
    url->host[0]='\0';
    url->port=0;
    url->string[0]='\0';

	osip_uri_param_freelist(&url->url_params);

	{
	    osip_uri_header_t *u_header;

	    while (!osip_list_eol(&url->url_headers, pos)) {
	        u_header = (osip_uri_header_t *) osip_list_get(&url->url_headers, pos);
	        osip_list_remove(&url->url_headers, pos);
	        osip_uri_header_free(u_header);
	    }
	}
}

int osip_uri_clone(const osip_uri_t * url, osip_uri_t * dest)
{
	int i;
	osip_uri_t ur;
	if (url == NULL)
		return OSIP_BADPARAMETER;
	if (url->host == NULL && url->string == NULL)
		return OSIP_BADPARAMETER;

	i = osip_uri_init(&ur);
	if (i != 0)					/* allocation failed */
		return i;
    if (url->scheme[0] != '\0')
		osip_strncpy(ur.scheme, url->scheme, schemeSize-1);
    if (url->username[0] != '\0')
		osip_strncpy(ur.username, url->username, usernameSize-1);
    if (url->password[0] != '\0')
		osip_strncpy(ur.password, url->password, passwordSize-1);
    if (url->host[0] != '\0')
	    osip_strncpy(ur.host, url->host, hostSize-1);
    if (url->port > 0)
	    ur.port = url->port;
    if (url->string[0] != '\0')
	    osip_strncpy(ur.string, url->string, stringSize-1);

    i = osip_list_clone(&url->url_params, &ur.url_params,
						(int (*)(void *, void **)) &osip_uri_param_clone);
	if (i != 0) {
		osip_uri_free(&ur);
		return i;
	}
	i = osip_list_clone(&url->url_headers, &ur.url_headers,
						(int (*)(void *, void **)) &osip_uri_param_clone);
	if (i != 0) {
		osip_uri_free(&ur);
		return i;
	}

    memcpy(dest, &ur, sizeof(osip_uri_t));
	return OSIP_SUCCESS;
}

int osip_uri_param_init(osip_uri_param_t ** url_param)
{
	*url_param = (osip_uri_param_t *) osip_malloc(sizeof(osip_uri_param_t));
	if (*url_param == NULL)
		return OSIP_NOMEM;
	(*url_param)->gname = NULL;
	(*url_param)->gvalue = NULL;
	return OSIP_SUCCESS;
}

void osip_uri_param_free(osip_uri_param_t * url_param)
{
	osip_free(url_param->gname);
	osip_free(url_param->gvalue);
	osip_free(url_param);
}

int osip_uri_param_set(osip_uri_param_t * url_param, char *pname, char *pvalue)
{
	url_param->gname = pname;
	/* not needed for url, but for all other generic params */
	osip_clrspace(url_param->gname);
	url_param->gvalue = pvalue;
	if (url_param->gvalue != NULL)
		osip_clrspace(url_param->gvalue);
	return OSIP_SUCCESS;
}

int osip_uri_param_add(osip_list_t * url_params, char *pname, char *pvalue)
{
	int i;
	osip_uri_param_t *url_param;

	i = osip_uri_param_init(&url_param);
	if (i != 0)
		return i;
	i = osip_uri_param_set(url_param, pname, pvalue);
	if (i != 0) {
		osip_uri_param_free(url_param);
		return i;
	}
	osip_list_add(url_params, url_param, -1);
	return OSIP_SUCCESS;
}

void osip_uri_param_freelist(osip_list_t * params)
{
	osip_uri_param_t *u_param;

	while (!osip_list_eol(params, 0)) {
		u_param = (osip_uri_param_t *) osip_list_get(params, 0);
		osip_list_remove(params, 0);
		osip_uri_param_free(u_param);
	}
}

int
osip_uri_param_get_byname(osip_list_t * params, char *pname,
						  osip_uri_param_t ** url_param)
{
	int pos = 0;
	size_t pname_len;
	osip_uri_param_t *u_param;

	*url_param = NULL;
	if (pname == NULL)
		return OSIP_BADPARAMETER;
	pname_len = strlen(pname);
	if (pname_len <= 0)
		return OSIP_BADPARAMETER;
	while (!osip_list_eol(params, pos)) {
		size_t len;

		u_param = (osip_uri_param_t *) osip_list_get(params, pos);
		len = strlen(u_param->gname);
		if (pname_len == len
			&& osip_strncasecmp(u_param->gname, pname, strlen(pname)) == 0) {
			*url_param = u_param;
			return OSIP_SUCCESS;
		}
		pos++;
	}
	return OSIP_UNDEFINED_ERROR;
}

int osip_uri_param_clone(const osip_uri_param_t * uparam, osip_uri_param_t ** dest)
{
	int i;
	osip_uri_param_t *up;

	*dest = NULL;
	if (uparam == NULL)
		return OSIP_BADPARAMETER;
	if (uparam->gname == NULL)
		return OSIP_BADPARAMETER;	/* name is mandatory */

	i = osip_uri_param_init(&up);
	if (i != 0)					/* allocation failed */
		return i;
	up->gname = osip_strdup(uparam->gname);
	if (uparam->gvalue != NULL)
		up->gvalue = osip_strdup(uparam->gvalue);
	else
		up->gvalue = NULL;
	*dest = up;
	return OSIP_SUCCESS;
}


#define _ALPHANUM_ "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\0"
#define _RESERVED_ ";/?:@&=+$\0"
#define _MARK_ "-_.!~*'()\0"

#define _MARK__USER_UNRESERVED_ "-_.!~*'()&=+$,;?/\0"
#define _MARK__PWORD_UNRESERVED_ "-_.!~*'()&=+$,\0"
#define _MARK__URI_PARAM_UNRESERVED_ "-_.!~*'()[]/:&+$\0"
#define _MARK__HEADER_PARAM_UNRESERVED_ "-_.!~*'()[]/?:+$\0"

#define osip_is_alphanum(in) (  \
       (in >= 'a' && in <= 'z') || \
       (in >= 'A' && in <= 'Z') || \
       (in >= '0' && in <= '9'))

char *__osip_uri_escape_nonascii_and_nondef(const char *string, const char *def)
{
	size_t alloc = strlen(string) + 1;
	size_t length;
	char *ns = (char *) osip_malloc(alloc);
	unsigned char in;
	size_t newlen = alloc;
	int index = 0;
	const char *tmp;
	int i;

	if (ns == NULL)
		return NULL;

	length = alloc - 1;
	while (length--) {
		in = *string;

		i = 0;
		tmp = NULL;
		if (osip_is_alphanum(in))
			tmp = string;
		else {
			for (; def[i] != '\0' && def[i] != in; i++) {
			}
			if (def[i] != '\0')
				tmp = string;
		}
		if (tmp == NULL) {
			/* encode it */
			newlen += 2;		/* the size grows with two, since this'll become a %XX */
			if (newlen > alloc) {
				alloc *= 2;
				ns = osip_realloc(ns, alloc);
				if (!ns)
					return NULL;
			}
			sprintf(&ns[index], "%%%02X", in);
			index += 3;
		} else {
			/* just copy this */
			ns[index++] = in;
		}
		string++;
	}
	ns[index] = 0;				/* terminate it */
	return ns;
}

/* user =  *( unreserved / escaped / user-unreserved ) */
const char *userinfo_def = /* implied _ALPHANUM_ */ _MARK__USER_UNRESERVED_;
char *__osip_uri_escape_userinfo(const char *string)
{
	return __osip_uri_escape_nonascii_and_nondef(string, userinfo_def);
}

/* user =  *( unreserved / escaped / user-unreserved ) */
const char *password_def = _MARK__PWORD_UNRESERVED_;
char *__osip_uri_escape_password(const char *string)
{
	return __osip_uri_escape_nonascii_and_nondef(string, password_def);
}

const char *uri_param_def = _MARK__URI_PARAM_UNRESERVED_;
char *__osip_uri_escape_uri_param(char *string)
{
	return __osip_uri_escape_nonascii_and_nondef(string, uri_param_def);
}

const char *header_param_def = _MARK__HEADER_PARAM_UNRESERVED_;
char *__osip_uri_escape_header_param(char *string)
{
	return __osip_uri_escape_nonascii_and_nondef(string, header_param_def);
}

void __osip_uri_unescape(char *string)
{
	size_t alloc = strlen(string) + 1;
	unsigned char in;
	int index = 0;
	unsigned int hex;
	char *ptr;

	ptr = string;
	while (--alloc > 0) {
		in = *ptr;
		if ('%' == in) {
			/* encoded part */
			if (alloc > 2 && sscanf(ptr + 1, "%02X", &hex) == 1) {
				in = (unsigned char) hex;
				if (*(ptr + 2) &&
					((*(ptr + 2) >= '0' && *(ptr + 2) <= '9') ||
					 (*(ptr + 2) >= 'a' && *(ptr + 2) <= 'f') ||
					 (*(ptr + 2) >= 'A' && *(ptr + 2) <= 'F'))) {
					alloc -= 2;
					ptr += 2;
				} else {
					alloc -= 1;
					ptr += 1;
				}
			} else {
				break;
			}
		}

		string[index++] = in;
		ptr++;
	}
	string[index] = 0;			/* terminate it */
}


/* RFC3261 16.5 
 */
int osip_uri_to_str_canonical(const osip_uri_t * url, char **dest)
{
	int result;

	*dest = NULL;
	result = osip_uri_to_str(url, dest);
	if (result == 0) {
		/*
		   tmp = strchr(*dest, ";");
		   if (tmp !=NULL) {
		   buf=strndup(*dest, tmp-(*dest));
		   osip_free(*dest);
		   *dest=buf;
		   }
		 */
		__osip_uri_unescape(*dest);
	}
	return result;
}

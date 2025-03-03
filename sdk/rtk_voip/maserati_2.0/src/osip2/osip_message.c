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


#include <stdio.h>
#include <stdlib.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>

/* enable logging of memory accesses */

const char *osip_protocol_version = "SIP/2.0";


int osip_message_init(osip_message_t ** sip)
{
	*sip = (osip_message_t *) osip_malloc(sizeof(osip_message_t));
	if (*sip == NULL)
		return OSIP_NOMEM;
	memset(*sip, 0, sizeof(osip_message_t));

#ifndef MINISIZE
	osip_list_init(&(*sip)->accepts);
	osip_list_init(&(*sip)->accept_encodings);

	osip_list_init(&(*sip)->accept_languages);
	osip_list_init(&(*sip)->alert_infos);
	osip_list_init(&(*sip)->allows);
	osip_list_init(&(*sip)->authentication_infos);
#endif
	osip_list_init(&(*sip)->authorizations);
	(*sip)->call_id = NULL;
#ifndef MINISIZE
	osip_list_init(&(*sip)->call_infos);
#endif
	osip_list_init(&(*sip)->contacts);

#ifndef MINISIZE
	osip_list_init(&(*sip)->content_encodings);
#endif
	(*sip)->content_length = NULL;
	(*sip)->content_type = NULL;
	(*sip)->cseq = NULL;
#ifndef MINISIZE
	osip_list_init(&(*sip)->error_infos);
#endif
	(*sip)->from = NULL;
	(*sip)->mime_version = NULL;
	osip_list_init(&(*sip)->proxy_authenticates);
#ifndef MINISIZE
	osip_list_init(&(*sip)->proxy_authentication_infos);
#endif
	osip_list_init(&(*sip)->proxy_authorizations);
	osip_list_init(&(*sip)->record_routes);
	osip_list_init(&(*sip)->routes);
	osip_list_init(&(*sip)->service_routes);
	(*sip)->to = NULL;
	osip_list_init(&(*sip)->vias);
	osip_list_init(&(*sip)->www_authenticates);

	osip_list_init(&(*sip)->bodies);

	osip_list_init(&(*sip)->headers);

	(*sip)->message_property = 3;
	(*sip)->message = NULL;		/* buffer to avoid calling osip_message_to_str many times (for retransmission) */
	(*sip)->message_length = 0;

	(*sip)->application_data = NULL;
	return OSIP_SUCCESS;		/* ok */
}


void osip_message_set_reason_phrase(osip_message_t * sip, char *reason)
{
	sip->reason_phrase = reason;
}

void osip_message_set_status_code(osip_message_t * sip, int status_code)
{
	sip->status_code = status_code;
}

void osip_message_set_method(osip_message_t * sip, char *sip_method)
{
	sip->sip_method = sip_method;
}

void osip_message_set_version(osip_message_t * sip, char *sip_version)
{
	sip->sip_version = sip_version;
}

void osip_message_set_uri(osip_message_t * sip, osip_uri_t * url)
{
	memcpy(&sip->req_uri, url, sizeof(osip_uri_t));
}

void osip_message_set_outboundproxy(osip_message_t * sip, char * proxy_addr,int proxy_port)
{

	if(proxy_port!=0){
		osip_strncpy(sip->outbound_proxy, proxy_addr, strlen(proxy_addr));
		sip->outbound_port = proxy_port;
	}

}


void osip_message_free(osip_message_t * sip)
{
	if (sip == NULL)
		return;

	osip_free(sip->sip_method);
	osip_free(sip->sip_version);
	if (&sip->req_uri != NULL)
		osip_uri_free(&sip->req_uri);
	osip_free(sip->reason_phrase);

#ifndef MINISIZE
	osip_list_special_free(&sip->accepts, (void (*)(void *)) &osip_accept_free);
#endif
	osip_list_special_free(&sip->authorizations,
						   (void (*)(void *)) &osip_authorization_free);
	if (sip->call_id != NULL)
		osip_call_id_free(sip->call_id);
#ifndef MINISIZE
	osip_list_special_free(&sip->accept_encodings,
						   (void (*)(void *)) &osip_accept_encoding_free);
	osip_list_special_free(&sip->accept_languages,
						   (void (*)(void *)) &osip_accept_language_free);
	osip_list_special_free(&sip->alert_infos,
						   (void (*)(void *)) &osip_alert_info_free);
	osip_list_special_free(&sip->allows, (void (*)(void *)) &osip_allow_free);
	osip_list_special_free(&sip->authentication_infos,
						   (void (*)(void *)) &osip_authentication_info_free);
	osip_list_special_free(&sip->call_infos,
						   (void (*)(void *)) &osip_call_info_free);
	osip_list_special_free(&sip->content_encodings,
						   (void (*)(void *)) &osip_content_encoding_free);
	osip_list_special_free(&sip->error_infos,
						   (void (*)(void *)) &osip_error_info_free);
	osip_list_special_free(&sip->proxy_authentication_infos, (void (*)(void *))
						   &osip_proxy_authentication_info_free);
#endif
	osip_list_special_free(&sip->contacts, (void (*)(void *)) &osip_contact_free);
	if (sip->content_length != NULL)
		osip_content_length_free(sip->content_length);
	if (sip->content_type != NULL)
		osip_content_type_free(sip->content_type);
	if (sip->cseq != NULL)
		osip_cseq_free(sip->cseq);
	if (sip->from != NULL)
		osip_from_free(sip->from);
	if (sip->mime_version != NULL)
		osip_mime_version_free(sip->mime_version);
	osip_list_special_free(&sip->proxy_authenticates,
						   (void (*)(void *)) &osip_proxy_authenticate_free);
	osip_list_special_free(&sip->proxy_authorizations,
						   (void (*)(void *)) &osip_proxy_authorization_free);
	osip_list_special_free(&sip->record_routes,
						   (void (*)(void *)) &osip_record_route_free);
	osip_list_special_free(&sip->routes, (void (*)(void *)) &osip_route_free);
	if (sip->to != NULL)
		osip_to_free(sip->to);
	osip_list_special_free(&sip->vias, (void (*)(void *)) &osip_via_free);
	osip_list_special_free(&sip->www_authenticates,
						   (void (*)(void *)) &osip_www_authenticate_free);
	osip_list_special_free(&sip->headers, (void (*)(void *)) &osip_header_free);
	osip_list_special_free(&sip->bodies, (void (*)(void *)) &osip_body_free);
	osip_list_special_free(&sip->service_routes, (void (*)(void *))
						   &osip_service_routes_free);
	osip_free(sip->message);
	osip_free(sip);
}


int osip_message_clone(const osip_message_t * sip, osip_message_t ** dest)
{
	osip_message_t *copy;
	int pos = 0;
	int i;

	*dest = NULL;
	if (sip == NULL)
		return OSIP_BADPARAMETER;

	i = osip_message_init(&copy);
	if (i != 0)
		return i;

	copy->sip_method = osip_strdup(sip->sip_method);
	if (sip->sip_method != NULL && copy->sip_method == NULL) {
		osip_message_free(copy);
		return OSIP_NOMEM;
	}
	copy->sip_version = osip_strdup(sip->sip_version);
	if (sip->sip_version != NULL && copy->sip_version == NULL) {
		osip_message_free(copy);
		return OSIP_NOMEM;
	}
	copy->status_code = sip->status_code;
	copy->reason_phrase = osip_strdup(sip->reason_phrase);
	if (sip->reason_phrase != NULL && copy->reason_phrase == NULL) {
		osip_message_free(copy);
		return OSIP_NOMEM;
	}
	if (&sip->req_uri != NULL) { 
		i = osip_uri_clone(&sip->req_uri, &(copy->req_uri));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
#ifndef MINISIZE
	{
		osip_accept_t *accept;
		osip_accept_t *accept2;

		pos = 0;
		while (!osip_list_eol(&sip->accepts, pos)) {
			accept = (osip_accept_t *) osip_list_get(&sip->accepts, pos);
			i = osip_accept_clone(accept, &accept2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->accepts, accept2, -1);	/* insert as last element */
			pos++;
		}
	}
	{
		osip_accept_encoding_t *accept_encoding;
		osip_accept_encoding_t *accept_encoding2;

		pos = 0;
		while (!osip_list_eol(&sip->accept_encodings, pos)) {
			accept_encoding =
				(osip_accept_encoding_t *) osip_list_get(&sip->accept_encodings,
														 pos);
			i = osip_accept_encoding_clone(accept_encoding, &accept_encoding2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->accept_encodings, accept_encoding2, -1);
			pos++;
		}
	}
	{
		osip_accept_language_t *accept_language;
		osip_accept_language_t *accept_language2;

		pos = 0;
		while (!osip_list_eol(&sip->accept_languages, pos)) {
			accept_language =
				(osip_accept_language_t *) osip_list_get(&sip->accept_languages,
														 pos);
			i = osip_accept_language_clone(accept_language, &accept_language2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->accept_languages, accept_language2, -1);
			pos++;
		}
	}
	{
		osip_alert_info_t *alert_info;
		osip_alert_info_t *alert_info2;

		pos = 0;
		while (!osip_list_eol(&sip->alert_infos, pos)) {
			alert_info =
				(osip_alert_info_t *) osip_list_get(&sip->alert_infos, pos);
			i = osip_alert_info_clone(alert_info, &alert_info2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->alert_infos, alert_info2, -1);
			pos++;
		}
	}
	{
		osip_allow_t *allow;
		osip_allow_t *allow2;

		pos = 0;
		while (!osip_list_eol(&sip->allows, pos)) {
			allow = (osip_allow_t *) osip_list_get(&sip->allows, pos);
			i = osip_allow_clone(allow, &allow2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->allows, allow2, -1);
			pos++;
		}
	}
	{
		osip_authentication_info_t *authentication_info;
		osip_authentication_info_t *authentication_info2;

		pos = 0;
		while (!osip_list_eol(&sip->authentication_infos, pos)) {
			authentication_info = (osip_authentication_info_t *)
				osip_list_get(&sip->authentication_infos, pos);
			i = osip_authentication_info_clone(authentication_info,
											   &authentication_info2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->authentication_infos, authentication_info2, -1);
			pos++;
		}
	}
	{
		osip_call_info_t *call_info;
		osip_call_info_t *call_info2;

		pos = 0;
		while (!osip_list_eol(&sip->call_infos, pos)) {
			call_info = (osip_call_info_t *) osip_list_get(&sip->call_infos, pos);
			i = osip_call_info_clone(call_info, &call_info2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->call_infos, call_info2, -1);
			pos++;
		}
	}
	{
		osip_content_encoding_t *content_encoding;
		osip_content_encoding_t *content_encoding2;

		pos = 0;
		while (!osip_list_eol(&sip->content_encodings, pos)) {
			content_encoding =
				(osip_content_encoding_t *) osip_list_get(&sip->content_encodings,
														  pos);
			i = osip_content_encoding_clone(content_encoding, &content_encoding2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->content_encodings, content_encoding2, -1);
			pos++;
		}
	}
	{
		osip_error_info_t *error_info;
		osip_error_info_t *error_info2;

		pos = 0;
		while (!osip_list_eol(&sip->error_infos, pos)) {
			error_info =
				(osip_error_info_t *) osip_list_get(&sip->error_infos, pos);
			i = osip_error_info_clone(error_info, &error_info2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->error_infos, error_info2, -1);
			pos++;
		}
	}
	{
		osip_proxy_authentication_info_t *proxy_authentication_info;
		osip_proxy_authentication_info_t *proxy_authentication_info2;

		pos = 0;
		while (!osip_list_eol(&sip->proxy_authentication_infos, pos)) {
			proxy_authentication_info = (osip_proxy_authentication_info_t *)
				osip_list_get(&sip->proxy_authentication_infos, pos);
			i = osip_proxy_authentication_info_clone(proxy_authentication_info,
													 &proxy_authentication_info2);
			if (i != 0) {
				osip_message_free(copy);
				return i;
			}
			osip_list_add(&copy->proxy_authentication_infos,
						  proxy_authentication_info2, -1);
			pos++;
		}
	}
#endif
	i = osip_list_clone(&sip->authorizations, &copy->authorizations,
						(int (*)(void *, void **)) &osip_authorization_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	if (sip->call_id != NULL) {
		i = osip_call_id_clone(sip->call_id, &(copy->call_id));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	i = osip_list_clone(&sip->contacts, &copy->contacts,
						(int (*)(void *, void **)) &osip_contact_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}

	i = osip_list_clone(&sip->service_routes, &copy->service_routes,
						(int (*)(void *, void **)) &osip_service_route_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}

	if (sip->content_length != NULL) {
		i = osip_content_length_clone(sip->content_length,
									  &(copy->content_length));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	if (sip->content_type != NULL) {
		i = osip_content_type_clone(sip->content_type, &(copy->content_type));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	if (sip->cseq != NULL) {
		i = osip_cseq_clone(sip->cseq, &(copy->cseq));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	if (sip->from != NULL) {
		i = osip_from_clone(sip->from, &(copy->from));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	if (sip->mime_version != NULL) {
		i = osip_mime_version_clone(sip->mime_version, &(copy->mime_version));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	i = osip_list_clone(&sip->proxy_authenticates, &copy->proxy_authenticates,
						(int (*)(void *, void **)) &osip_proxy_authenticate_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	i = osip_list_clone(&sip->proxy_authorizations, &copy->proxy_authorizations,
						(int (*)(void *, void **))
						&osip_proxy_authorization_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	if(sip->replaces!=NULL){
		i = osip_replaces_clone(sip->replaces, &(copy->replaces));
		if (i != 0) {
		osip_message_free(copy);
		return i;
		}
	}
	i = osip_list_clone(&sip->record_routes, &copy->record_routes,
						(int (*)(void *, void **)) &osip_record_route_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	i = osip_list_clone(&sip->routes, &copy->routes,
						(int (*)(void *, void **)) &osip_route_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	if (sip->to != NULL) {
		i = osip_to_clone(sip->to, &(copy->to));
		if (i != 0) {
			osip_message_free(copy);
			return i;
		}
	}
	i = osip_list_clone(&sip->vias, &copy->vias,
						(int (*)(void *, void **)) &osip_via_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	i = osip_list_clone(&sip->www_authenticates, &copy->www_authenticates,
						(int (*)(void *, void **)) &osip_www_authenticate_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	i = osip_list_clone(&sip->headers, &copy->headers,
						(int (*)(void *, void **)) &osip_header_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}
	i = osip_list_clone(&sip->bodies, &copy->bodies,
						(int (*)(void *, void **)) &osip_body_clone);
	if (i != 0) {
		osip_message_free(copy);
		return i;
	}

	copy->message_length = sip->message_length;
	copy->message = osip_strdup(sip->message);
	if (copy->message == NULL && sip->message != NULL) {
		osip_message_free(copy);
		return OSIP_NOMEM;
	}
	copy->message_property = sip->message_property;
	strcpy(copy->orig_number,sip->orig_number);
	*dest = copy;
	return OSIP_SUCCESS;
}

int
osip_message_get_knownheaderlist(osip_list_t * header_list, int pos, void **dest)
{
	*dest = NULL;
	if (osip_list_size(header_list) <= pos)
		return OSIP_UNDEFINED_ERROR;	/* does not exist */
	*dest = (void *) osip_list_get(header_list, pos);
	return pos;
}

/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2002,2003,2004,2005,2006,2007  Aymeric MOIZARD  - jack@atosc.org
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#ifndef MINISIZE

#include "eXosip2.h"

extern eXosip_t eXosip;

int
_eXosip_insubscription_transaction_find(int tid, eXosip_notify_t ** jn,
										eXosip_dialog_t ** jd,
										osip_transaction_t ** tr)
{
	for (*jn = eXosip.j_notifies; *jn != NULL; *jn = (*jn)->next) {
		if ((*jn)->n_inc_tr != NULL && (*jn)->n_inc_tr->transactionid == tid) {
			*tr = (*jn)->n_inc_tr;
			*jd = (*jn)->n_dialogs;
			return OSIP_SUCCESS;
		}
		if ((*jn)->n_out_tr != NULL && (*jn)->n_out_tr->transactionid == tid) {
			*tr = (*jn)->n_out_tr;
			*jd = (*jn)->n_dialogs;
			return OSIP_SUCCESS;
		}
		for (*jd = (*jn)->n_dialogs; *jd != NULL; *jd = (*jd)->next) {
			osip_transaction_t *transaction;
			int pos = 0;

			while (!osip_list_eol((*jd)->d_inc_trs, pos)) {
				transaction =
					(osip_transaction_t *) osip_list_get((*jd)->d_inc_trs, pos);
				if (transaction != NULL && transaction->transactionid == tid) {
					*tr = transaction;
					return OSIP_SUCCESS;
				}
				pos++;
			}

			pos = 0;
			while (!osip_list_eol((*jd)->d_out_trs, pos)) {
				transaction =
					(osip_transaction_t *) osip_list_get((*jd)->d_out_trs, pos);
				if (transaction != NULL && transaction->transactionid == tid) {
					*tr = transaction;
					return OSIP_SUCCESS;
				}
				pos++;
			}
		}
	}
	*jd = NULL;
	*jn = NULL;
	return OSIP_NOTFOUND;
}

int eXosip_insubscription_remove(int did)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;

	if (did <= 0)
		return OSIP_BADPARAMETER;

	if (did > 0) {
		eXosip_notify_dialog_find(did, &jn, &jd);
	}
	if (jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		return OSIP_NOTFOUND;
	}
	REMOVE_ELEMENT(eXosip.j_notifies, jn);
	eXosip_notify_free(jn);
	return OSIP_SUCCESS;
}

int
eXosip_insubscription_build_answer(int tid, int status, osip_message_t ** answer)
{
	int i = -1;
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;
	osip_transaction_t *tr = NULL;

	*answer = NULL;

	if (tid <= 0)
		return OSIP_BADPARAMETER;

	if (tid > 0) {
		_eXosip_insubscription_transaction_find(tid, &jn, &jd, &tr);
	}
	if (tr == NULL || jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		return OSIP_NOTFOUND;
	}

	if (status < 101 || status > 699) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: wrong status code (101<status<699)\n"));
		return OSIP_BADPARAMETER;
	}

		i = _eXosip_build_response_default(answer, jd->d_dialog, status,
										   tr->orig_request);

	if (i != 0) {
		OSIP_TRACE(osip_trace(__FILE__, __LINE__, OSIP_ERROR, NULL,
							  "ERROR: Could not create response for %s\n",
							  tr->orig_request->sip_method));
		return i;
	}

	if (status >= 200 && status <= 299)
		_eXosip_notify_add_expires_in_2XX_for_subscribe(jn, *answer);

	if (status < 300)
		i = complete_answer_that_establish_a_dialog(*answer, tr->orig_request);

	return i;
}

int eXosip_insubscription_send_answer(int tid, int status, osip_message_t * answer)
{
	int i = -1;
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;
	osip_transaction_t *tr = NULL;
	osip_event_t *evt_answer;

	if (tid <= 0)
		return OSIP_BADPARAMETER;

	if (tid > 0) {
		_eXosip_insubscription_transaction_find(tid, &jn, &jd, &tr);
	}
	if (jd == NULL || tr == NULL || tr->orig_request == NULL
		|| tr->orig_request->sip_method == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		osip_message_free(answer);
		return OSIP_NOTFOUND;
	}

	if (answer == NULL) {
		if (0 == osip_strcasecmp(tr->orig_request->sip_method, "SUBSCRIBE")) {
			if (status >= 200 && status <= 299) {
				OSIP_TRACE(osip_trace
						   (__FILE__, __LINE__, OSIP_ERROR, NULL,
							"eXosip: Wrong parameter?\n"));
				return OSIP_BADPARAMETER;
			}
		}
	}

	/* is the transaction already answered? */
	if (tr->state == NIST_COMPLETED || tr->state == NIST_TERMINATED) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: transaction already answered\n"));
		osip_message_free(answer);
		return OSIP_WRONG_STATE;
	}

	if (answer == NULL) {
		if (0 == osip_strcasecmp(tr->orig_request->sip_method, "SUBSCRIBE")) {
			if (status < 200)
				i = _eXosip_insubscription_answer_1xx(jn, jd, status);
			else
				i = _eXosip_insubscription_answer_3456xx(jn, jd, status);
			if (i != 0) {
				OSIP_TRACE(osip_trace
						   (__FILE__, __LINE__, OSIP_ERROR, NULL,
							"eXosip: cannot send response!\n"));
				return i;
			}
		} else {
			/* TODO */
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: a response must be given!\n"));
			return OSIP_BADPARAMETER;
		}
		return OSIP_SUCCESS;
	} else {
		i = 0;
	}

	if (0 == osip_strcasecmp(tr->orig_request->sip_method, "SUBSCRIBE")) {
		if (MSG_IS_STATUS_1XX(answer)) {
		} else if (MSG_IS_STATUS_2XX(answer)) {
			eXosip_dialog_set_200ok(jd, answer);
			osip_dialog_set_state(jd->d_dialog, DIALOG_CONFIRMED);
		} else if (answer->status_code >= 300 && answer->status_code <= 699) {
			i = 0;
		} else {
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: wrong status code (101<status<699)\n"));
			osip_message_free(answer);
			return OSIP_BADPARAMETER;
		}
	}

	evt_answer = osip_new_outgoing_sipmessage(answer);
	evt_answer->transactionid = tr->transactionid;

	osip_transaction_add_event(tr, evt_answer);
	eXosip_update();
	__eXosip_wakeup();
	return OSIP_SUCCESS;
}

int
eXosip_insubscription_build_notify(int did, int subscription_status,
								   int subscription_reason,
								   osip_message_t ** request)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;

	char subscription_state[50];
	char *tmp;
	time_t now = time(NULL);

	int i;

	*request = NULL;

	if (did <= 0)
		return OSIP_BADPARAMETER;

	if (did > 0) {
		eXosip_notify_dialog_find(did, &jn, &jd);
	}
	if (jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		return OSIP_NOTFOUND;
	}

	i = eXosip_insubscription_build_request(did, "NOTIFY", request);
	if (i != 0) {
		return i;
	}
#ifndef SUPPORT_MSN
	if (subscription_status == EXOSIP_SUBCRSTATE_PENDING)
		osip_strncpy(subscription_state, "pending;expires=", 16);
	else if (subscription_status == EXOSIP_SUBCRSTATE_ACTIVE)
		osip_strncpy(subscription_state, "active;expires=", 15);
	else if (subscription_status == EXOSIP_SUBCRSTATE_TERMINATED) {
		if (subscription_reason == DEACTIVATED)
			osip_strncpy(subscription_state, "terminated;reason=deactivated", 29);
		else if (subscription_reason == PROBATION)
			osip_strncpy(subscription_state, "terminated;reason=probation", 27);
		else if (subscription_reason == REJECTED)
			osip_strncpy(subscription_state, "terminated;reason=rejected", 26);
		else if (subscription_reason == TIMEOUT)
			osip_strncpy(subscription_state, "terminated;reason=timeout", 25);
		else if (subscription_reason == GIVEUP)
			osip_strncpy(subscription_state, "terminated;reason=giveup", 24);
		else if (subscription_reason == NORESOURCE)
			osip_strncpy(subscription_state, "terminated;reason=noresource", 28);
		else
			osip_strncpy(subscription_state, "terminated;reason=noresource", 28);
	} else
		osip_strncpy(subscription_state, "pending;expires=", 16);

	tmp = subscription_state + strlen(subscription_state);
	if (subscription_status != EXOSIP_SUBCRSTATE_TERMINATED)
		snprintf(tmp, 50 - (tmp - subscription_state), "%li", jn->n_ss_expires - now);
	osip_message_set_header(*request, "Subscription-State", subscription_state);
#endif

	return OSIP_SUCCESS;
}

int
eXosip_insubscription_build_request(int did, const char *method,
									osip_message_t ** request)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;

	osip_transaction_t *transaction;
	char *transport;
	int i;

	*request = NULL;
	if (method == NULL || method[0] == '\0')
		return OSIP_BADPARAMETER;

	if (did <= 0)
		return OSIP_BADPARAMETER;

	if (did > 0) {
		eXosip_notify_dialog_find(did, &jn, &jd);
	}
	if (jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		return OSIP_NOTFOUND;
	}

	transaction = NULL;
	transaction = eXosip_find_last_out_notify(jn, jd);
	if (transaction != NULL) {
		if (transaction->state != NICT_TERMINATED &&
			transaction->state != NIST_TERMINATED &&
			transaction->state != NICT_COMPLETED &&
			transaction->state != NIST_COMPLETED)
			return OSIP_WRONG_STATE;
	}

	transport = NULL;
	if (transaction == NULL)
		transaction = jn->n_inc_tr;

	if (transaction != NULL && transaction->orig_request != NULL)
		transport = _eXosip_transport_protocol(transaction->orig_request);

	transaction = NULL;

	if (transport == NULL)
		i = _eXosip_build_request_within_dialog(request, method, jd->d_dialog,
												"UDP");
	else
		i = _eXosip_build_request_within_dialog(request, method, jd->d_dialog,
												transport);
	if (i != 0)
		return i;

	return OSIP_SUCCESS;
}

int eXosip_insubscription_send_request(int did, osip_message_t * request)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;

	osip_transaction_t *transaction;
	osip_event_t *sipevent;
	int i;

	if (request == NULL)
		return OSIP_BADPARAMETER;

	if (did <= 0) {
		osip_message_free(request);
		return OSIP_BADPARAMETER;
	}

	if (did > 0) {
		eXosip_notify_dialog_find(did, &jn, &jd);
	}
	if (jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		osip_message_free(request);
		return OSIP_NOTFOUND;
	}

	transaction = NULL;
	transaction = eXosip_find_last_out_notify(jn, jd);
	if (transaction != NULL) {
		if (transaction->state != NICT_TERMINATED &&
			transaction->state != NIST_TERMINATED &&
			transaction->state != NICT_COMPLETED &&
			transaction->state != NIST_COMPLETED) {
			osip_message_free(request);
			return OSIP_WRONG_STATE;
		}
		transaction = NULL;
	}

	i = _eXosip_transaction_init(&transaction, NICT, eXosip.j_osip, request);
	if (i != 0) {
		osip_message_free(request);
		return i;
	}

	osip_list_add(jd->d_out_trs, transaction, 0);

	sipevent = osip_new_outgoing_sipmessage(request);
	sipevent->transactionid = transaction->transactionid;

	osip_transaction_set_your_instance(transaction,
									   __eXosip_new_jinfo(NULL, jd, NULL, jn));
	osip_transaction_add_event(transaction, sipevent);
	__eXosip_wakeup();
	return OSIP_SUCCESS;
}

int
_eXosip_insubscription_send_request_with_credential(eXosip_notify_t * jn,
													eXosip_dialog_t * jd,
													osip_transaction_t * out_tr)
{
	osip_transaction_t *tr = NULL;
	osip_message_t *msg = NULL;
	osip_event_t *sipevent;

	int cseq;
	osip_via_t *via;
	int i;

	if (jn == NULL)
		return OSIP_BADPARAMETER;
	if (jd != NULL) {
		if (jd->d_out_trs == NULL)
			return OSIP_BADPARAMETER;
	}

	if (out_tr == NULL) {
		out_tr = eXosip_find_last_out_notify(jn, jd);
	}

	if (out_tr == NULL
		|| out_tr->orig_request == NULL || out_tr->last_response == NULL)
		return OSIP_NOTFOUND;

	i = osip_message_clone(out_tr->orig_request, &msg);
	if (i != 0) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: could not clone msg for authentication\n"));
		return i;
	}

	via = (osip_via_t *) osip_list_get(&msg->vias, 0);
	if (via == NULL || msg->cseq == NULL || msg->cseq->number == NULL) {
		osip_message_free(msg);
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: missing via or cseq header\n"));
		return OSIP_SYNTAXERROR;
	}

	/* increment cseq */
	cseq = atoi(msg->cseq->number);
	osip_free(msg->cseq->number);
	msg->cseq->number = strdup_printf("%i", cseq + 1);
	if (msg->cseq->number == NULL) {
		osip_message_free(msg);
		return OSIP_NOMEM;
	}

	if (jd != NULL && jd->d_dialog != NULL) {
		jd->d_dialog->local_cseq++;
	}

	i = eXosip_update_top_via(msg);
	if (i != 0) {
		osip_message_free(msg);
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: unsupported protocol\n"));
		return i;
	}

	if (out_tr->last_response->status_code == 401
		|| out_tr->last_response->status_code == 407)
		eXosip_add_authentication_information(msg, out_tr->last_response);
	else
		eXosip_add_authentication_information(msg, NULL);

	osip_message_force_update(msg);

	i = _eXosip_transaction_init(&tr, NICT, eXosip.j_osip, msg);

	if (i != 0) {
		osip_message_free(msg);
		return i;
	}

	/* add the new tr for the current dialog */
	osip_list_add(jd->d_out_trs, tr, 0);

	sipevent = osip_new_outgoing_sipmessage(msg);

	osip_transaction_set_your_instance(tr, __eXosip_new_jinfo(NULL, jd, NULL, jn));
	osip_transaction_add_event(tr, sipevent);

	eXosip_update();			/* fixed? */
	__eXosip_wakeup();
	return OSIP_SUCCESS;
}

int
_eXosip_insubscription_auto_send_notify(int did,
										int subscription_status,
										int subscription_reason)
{
	osip_message_t *notify;
	int i;
	char xml[4096];
	char *entity;
	eXosip_call_t *jc;
	eXosip_dialog_t *jd;

	if (did <= 0)
		return OSIP_BADPARAMETER;

	i = eXosip_insubscription_build_notify(did, subscription_status,
										   subscription_reason, &notify);
	if (i != 0) {
		return i;
	}

	/* build dialog xml state */
	memset(xml, 0, sizeof(xml));

	i = osip_uri_to_str(&notify->from->url, &entity);
	if (i != 0 || entity == NULL) {
		osip_message_free(notify);
		return i;
	}
	snprintf(xml, sizeof(xml),
			 "<?xml version=\"1.0\"?>" "\r\n"
			 "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\"" "\r\n"
			 "	version=\"2\" state=\"full\"" "\r\n"
			 "	entity=\"%s\">" "\r\n", entity);
	osip_free(entity);

	/* loop over all jc/jd */
	for (jc = eXosip.j_calls; jc != NULL; jc = jc->next) {
		for (jd = jc->c_dialogs; jd != NULL; jd = jd->next) {
			if (jd->d_dialog == NULL) {	/* finished call */
			} else {
				char tmp_dialog[2048];
				char direction[20];
				char dlg_state[20];
				char *remote_uri = NULL;

				if (jd->d_dialog->type == CALLER)
					strcpy(direction, "initiator");
				else
					strcpy(direction, "recipient");
				if (jd->d_dialog->state == DIALOG_CONFIRMED)
					strcpy(dlg_state, "confirmed");
				else
					strcpy(dlg_state, "early");

				if (jd->d_dialog->remote_uri != NULL) {
				    osip_uri_to_str(&jd->d_dialog->remote_uri->url, &remote_uri);
				}
				if (remote_uri != NULL) {
					/* add dialog info */
					snprintf(tmp_dialog, sizeof(tmp_dialog),
							 "	<dialog id=\"%s\" call-id=\"%s\"" "\r\n"
							 "		local-tag=\"%s\" remote-tag=\"%s\""
							 "\r\n" "		direction=\"%s\">"
							 "\r\n"
							 "		<state>%s</state>"
							 "\r\n"
							 "		<remote>"
							 "\r\n"
							 "			<identity>%s</identity>"
							 "\r\n"
							 "		</remote>"
							 "\r\n"
							 "	</dialog>"
							 "\r\n",
							 jd->d_dialog->call_id,
							 jd->d_dialog->call_id,
							 jd->d_dialog->local_tag,
							 jd->d_dialog->remote_tag,
							 direction, dlg_state, remote_uri);
					if (strlen(xml) + strlen(tmp_dialog) < sizeof(xml))
					strcat(xml, tmp_dialog);
				}
			}
		}
	}
	if (strlen(xml) + 16 < sizeof(xml))
	strcat(xml, "</dialog-info>" "\r\n");
	osip_message_set_content_type(notify, "application/dialog-info+xml");
	osip_message_set_body(notify, xml, strlen(xml));

	return eXosip_insubscription_send_request(did, notify);
}

int eXosip_insubscription_automatic(eXosip_event_t * evt)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_notify_t *jn = NULL;

	osip_header_t *event_header;
	if (evt->did <= 0 || evt->nid <= 0)
		return OSIP_BADPARAMETER;
	if (evt->request == NULL)
		return OSIP_BADPARAMETER;

	eXosip_notify_dialog_find(evt->did, &jn, &jd);
	if (jd == NULL || jn == NULL) {
		OSIP_TRACE(osip_trace
				   (__FILE__, __LINE__, OSIP_ERROR, NULL,
					"eXosip: No incoming subscription here?\n"));
		return OSIP_NOTFOUND;
	}

	osip_message_header_get_byname(evt->request, "event", 0, &event_header);
	if (event_header == NULL || event_header->hvalue == NULL) {
		eXosip_insubscription_send_answer(evt->tid, 400, NULL);
		return OSIP_SUCCESS;
	}

	/* this event should be handled internally */
	if (osip_strcasecmp(event_header->hvalue, "dialog") == 0) {
		/* send 200 ok to SUBSCRIBEs */

		if (evt->type == EXOSIP_IN_SUBSCRIPTION_NEW) {
			osip_message_t *answer;
			int i;

			i = eXosip_insubscription_build_answer(evt->tid, 202, &answer);
			if (i == 0) {
				i = eXosip_insubscription_send_answer(evt->tid, 202, answer);
			}
			if (i != 0) {
				i = eXosip_insubscription_send_answer(evt->tid, 400, NULL);
				return OSIP_SUCCESS;
			}

			/* send initial notify */
			i = _eXosip_insubscription_auto_send_notify(evt->did,
														EXOSIP_SUBCRSTATE_ACTIVE,
														PROBATION);
			if (i != 0) {
				/* delete subscription... */
				return OSIP_SUCCESS;
			}
		}
	} else {
		if (evt->type == EXOSIP_IN_SUBSCRIPTION_NEW) {
			eXosip_insubscription_send_answer(evt->tid, 489, NULL);
		}
	}

	return OSIP_SUCCESS;
}

#endif

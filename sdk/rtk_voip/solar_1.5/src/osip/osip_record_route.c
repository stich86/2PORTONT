/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005  Aymeric MOIZARD jack@atosc.org
  
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

#include <stdlib.h>
#include <stdio.h>

#include "osip_port.h"
#include "osip_message.h"
#include "osip_parser.h"

int
osip_record_route_init (osip_record_route_t ** record_route)
{
  return osip_from_init ((osip_from_t **) record_route);
}

/* adds the record_route header to message.         */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_record_route (osip_message_t * sip, const char *hvalue)
{
  osip_record_route_t *record_route;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return 0;

  i = osip_record_route_init (&record_route);
  if (i != 0)
    return -1;
  i = osip_record_route_parse (record_route, hvalue);
  if (i != 0)
    {
      osip_record_route_free (record_route);
      return -1;
    }
  sip->message_property = 2;
  osip_list_add (sip->record_routes, record_route, -1);
  return 0;
}

/* returns the record_route header.    */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
int
osip_message_get_record_route (const osip_message_t * sip, int pos,
			       osip_record_route_t ** dest)
{
  osip_record_route_t *record_route;

  *dest = NULL;
  if (osip_list_size (sip->record_routes) <= pos)
    return -1;			/* does not exist */
  record_route =
    (osip_record_route_t *) osip_list_get (sip->record_routes, pos);
  *dest = record_route;
  return pos;
}

int
osip_record_route_parse (osip_record_route_t * record_route,
			 const char *hvalue)
{
  return osip_from_parse ((osip_from_t *) record_route, hvalue);
}

/* returns the record_route header as a string.          */
/* INPUT : osip_record_route_t *record_route | record_route header.  */
/* returns -1 on error. */
int
osip_record_route_to_str (const osip_record_route_t * record_route,
			  char **dest)
{
  char *url;
  char *buf;
  int i;
  size_t len;

  *dest = NULL;
  if ((record_route == NULL) || (record_route->url == NULL))
    return -1;

  i = osip_uri_to_str (record_route->url, &url);
  if (i != 0)
    return -1;

  if (record_route->displayname == NULL)
    len = strlen (url) + 5;
  else
    len = strlen (url) + strlen (record_route->displayname) + 5;

  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    {
      osip_free (url);
      return -1;
    }

  /* route and record-route always use brackets */
  if (record_route->displayname != NULL)
    sprintf (buf, "%s <%s>", record_route->displayname, url);
  else
    sprintf (buf, "<%s>", url);
  osip_free (url);

  {
    int pos = 0;
    osip_generic_param_t *u_param;
    size_t plen;
    char *tmp;

    while (!osip_list_eol (record_route->gen_params, pos))
      {
	u_param =
	  (osip_generic_param_t *) osip_list_get (record_route->gen_params,
						  pos);

	if (u_param->gvalue == NULL)
	  plen = strlen (u_param->gname) + 2;
	else
	  plen = strlen (u_param->gname) + strlen (u_param->gvalue) + 3;
	len = len + plen;
	buf = (char *) osip_realloc (buf, len);
	tmp = buf;
	tmp = tmp + strlen (tmp);
	if (u_param->gvalue == NULL)
	  sprintf (tmp, ";%s", u_param->gname);
	else
	  sprintf (tmp, ";%s=%s", u_param->gname, u_param->gvalue);
	pos++;
      }
  }
  *dest = buf;
  return 0;
}

/* deallocates a osip_record_route_t structure.  */
/* INPUT : osip_record_route_t *record_route | record_route header. */
void
osip_record_route_free (osip_record_route_t * record_route)
{
  osip_from_free ((osip_from_t *) record_route);
}

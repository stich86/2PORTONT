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


#ifndef _OSIP_AUTHENTICATION_INFO_H_
#define _OSIP_AUTHENTICATION_INFO_H_


/**
 * @file osip_authentication_info.h
 * @brief oSIP osip_authentication_info header definition.
 */

/**
 * @defgroup oSIP_AUTH_INFO oSIP authentication-info header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for Authentication-Info headers.
 * @var osip_authentication_info_t
 */
  typedef struct osip_authentication_info
  {
    char *nextnonce;    /* optional */
    char *qop_options;  /* optional */
    char *rspauth;      /* optional */
    char *cnonce;       /* optional */
    char *nonce_count;  /* optional */
  } osip_authentication_info_t ;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Authenication-Info element.
 * @param header The element to work on.
 */
  int osip_authentication_info_init (osip_authentication_info_t ** header);
/**
 * Parse a Authenication-Info element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_authentication_info_parse (osip_authentication_info_t * header, const char *hvalue);
/**
 * Get a string representation of a Authenication-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_authentication_info_to_str (const osip_authentication_info_t * header, char **dest);
/**
 * Free a Authenication-Info element.
 * @param header The element to work on.
 */
  void osip_authentication_info_free (osip_authentication_info_t * header);
/**
 * Clone a Authenication-Info element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_authentication_info_clone (const osip_authentication_info_t * header,
			      osip_authentication_info_t ** dest);


/**
 * Get value of the nextnonce parameter from a Authenication-Info element.
 * @param header The element to work on.
 */
  char *osip_authentication_info_get_nextnonce (osip_authentication_info_t * header);
/**
 * Add the nextnonce parameter from a Authenication-Info element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authentication_info_set_nextnonce (osip_authentication_info_t * header, char *value);
/**
 * Get value of the cnonce parameter from a Authenication-Info element.
 * @param header The element to work on.
 */
  char *osip_authentication_info_get_cnonce (osip_authentication_info_t * header);
/**
 * Add the cnonce parameter from a Authenication-Info element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authentication_info_set_cnonce (osip_authentication_info_t * header, char *value);
/**
 * Get value of the qop_options parameter from a Authenication-Info element.
 * @param header The element to work on.
 */
  char *osip_authentication_info_get_qop_options (osip_authentication_info_t * header);
/**
 * Add the qop_options parameter from a Authenication-Info element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authentication_info_set_qop_options (osip_authentication_info_t * header,
					char *value);
/**
 * Get value of the rspauth parameter from a Authenication-Info element.
 * @param header The element to work on.
 */
  char *osip_authentication_info_get_rspauth (osip_authentication_info_t * header);
/**
 * Add the rspauth parameter from a Authenication-Info element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authentication_info_set_rspauth (osip_authentication_info_t * header, char *value);
/**
 * Get value of the nc parameter from a Authenication-Info element.
 * @param header The element to work on.
 */
  char *osip_authentication_info_get_nonce_count (osip_authentication_info_t * header);
/**
 * Add the nc parameter from a Authenication-Info element.
 * @param header The element to work on.
 * @param value The value of the new parameter.
 */
  void osip_authentication_info_set_nonce_count (osip_authentication_info_t * header, char *value);


#ifdef __cplusplus
}
#endif

/** @} */

#endif

/*
 * ipxcp.h - IPX Control Protocol definitions.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: ipxcp.h,v 1.1.1.1 2003/08/18 05:40:56 kaohj Exp $
 */

/*
 * Options.
 */
#define IPX_NETWORK_NUMBER        1   /* IPX Network Number */
#define IPX_NODE_NUMBER           2
#define IPX_COMPRESSION_PROTOCOL  3
#define IPX_ROUTER_PROTOCOL       4
#define IPX_ROUTER_NAME           5
#define IPX_COMPLETE              6


typedef struct ipxcp_options {
    int neg_node       : 1;	/* Negotiate IPX node number? */
    int req_node       : 1;	/* Ask peer to send IPX node number? */

    int neg_nn         : 1;	/* Negotiate IPX network number? */
    int req_nn         : 1;     /* Ask peer to send IPX network number */

    int neg_name       : 1;	/* Negotiate IPX router name */
    int neg_complete   : 1;     /* Negotiate completion */
    int neg_router     : 1;	/* Negotiate IPX router number */

    int accept_local   : 1;	/* accept peer's value for ournode */
    int accept_remote  : 1;	/* accept peer's value for hisnode */
    int accept_network : 1;	/* accept network number */

    u_int32_t his_network;	/* base network number */
    u_int32_t our_network;	/* our value for network number */
    u_int32_t network;		/* the final network number */

    u_char his_node[6];		/* peer's node number */
    u_char our_node[6];		/* our node number */
    u_char name [48];		/* name of the router */
    int    router;		/* routing protocol */
} ipxcp_options;

extern fsm ipxcp_fsm[];
extern ipxcp_options ipxcp_wantoptions[];
extern ipxcp_options ipxcp_gotoptions[];
extern ipxcp_options ipxcp_allowoptions[];
extern ipxcp_options ipxcp_hisoptions[];

void ipxcp_init __P((int));
void ipxcp_open __P((int));
void ipxcp_close __P((int));
void ipxcp_lowerup __P((int));
void ipxcp_lowerdown __P((int));
void ipxcp_input __P((int, u_char *, int));
void ipxcp_protrej __P((int));
int  ipxcp_printpkt __P((u_char *, int, void (*)(), void *));

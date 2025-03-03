/*!
*	\file			
*	\brief			
*	\Author		stein 
*
*	@(#)	%filespec: appswup.h-2 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( _APPSWUP_H )
#define	_APPSWUP_H

#include "cmbs_platf.h"

#if defined( __cplusplus )
extern "C"
{
#endif

#if defined( WIN32 )
void           app_FwUpdStart( HANDLE fd );
#else
void           app_FwUpdStart( int fd );
#endif

void           app_FwUpdPrepare( void );

#if defined( __cplusplus )
}
#endif

#endif	// _APPSWUP_H
//*/

/****************************************************************************

  (c) SYSTEC electronic GmbH, D-07973 Greiz, August-Bebel-Str. 29
      www.systec-electronic.com

  Project:      openPOWERLINK

  Description:  include file for userspace PDO module

  License:

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    3. Neither the name of SYSTEC electronic GmbH nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without prior written permission. For written
       permission, please contact info@systec-electronic.com.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Severability Clause:

        If a provision of this License is or becomes illegal, invalid or
        unenforceable in any jurisdiction, that shall not affect:
        1. the validity or enforceability in that jurisdiction of any other
           provision of this License; or
        2. the validity or enforceability in other jurisdictions of that or
           any other provision of this License.

  -------------------------------------------------------------------------

                $RCSfile: EplPdou.h,v $

                $Author: jiunming $

                $Revision: 1.1.1.1 $  $Date: 2010/05/05 09:01:39 $

                $State: Exp $

                Build Environment:

  -------------------------------------------------------------------------

  Revision History:

  2006/05/22 d.k.:   start of the implementation, version 1.00

****************************************************************************/

#ifndef _EPL_PDOU_H_
#define _EPL_PDOU_H_

#include "../EplPdo.h"

tEplKernel EplPdouAddInstance(void);

tEplKernel EplPdouDelInstance(void);

#if (((EPL_MODULE_INTEGRATION) & (EPL_MODULE_PDOU)) != 0)
tEplKernel EplPdouCbObdAccess(tEplObdCbParam *pParam_p);
#else
#define EplPdouCbObdAccess		NULL
#endif

// returns error if bPdoId_p is already valid
/*
tEplKernel EplPdouSetMapping(
    u8 bPdoId_p, BOOL fTxRx_p, u8 bNodeId, u8 bMappingVersion,
    tEplPdoMapping * pMapping_p, u8 bMaxEntries_p);

tEplKernel EplPdouGetMapping(
    u8 bPdoId_p, BOOL fTxRx_p, u8 * pbNodeId, u8 * pbMappingVersion,
    tEplPdoMapping * pMapping_p, u8 * pbMaxEntries_p);
*/

#endif // #ifndef _EPL_PDOU_H_

/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains support for the SHOW command, used to return
**  information related to a session.
**
**  AUTHOR:         Tim E. Sneddon
**
**  Copyright (c) 2014, Endless Software Solutions.
**
**  All rights reserved.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**  CREATION DATE:  16-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      16-MAY-2014 V1.0    Sneddon     Initial coding.
**      20-MAY-2014 V1.1    Sneddon     Add SHOW CONNECTION.
**--
*/
#define MODULE PJLCP_SHOW
#define IDENT "V1.1"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

/*
** Forward declarations
*/

    int act_show(void *ctx);

int act_show(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_FETCH:
        switch (pcbp->prs.av2) {
        case 0:                         /* SHOW CONNECTION */
            if (pcbp->sock == -1) {
                error(ENOTCONN, 0);
                status = ACT_ERROR;
            } else {
                fprintf(stdout,"\
\n\
Current connection:\n\
\n\
  Host: %s Port: %d\n\
  CPU time:              Connect time:\n\\n\
  Bytes\n\
    sent: %d received: %d\n\
  Flags\n\
    Automatic UEL:       Summary: \n\
    Packet Dump: \n", pcbp->hostname, pcbp->port, pcbp->wcnt,
                              pcbp->rcnt);
            }
            break;

        case 1:                         /* SHOW VERSION */
            version();
            break;

        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_show */


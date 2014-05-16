/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains support for all Status Readback Commands as
**  described in Chapter 7 of the "Printer Job Language Technical Refernece
**  Manual".
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
**  CREATION DATE:  08-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      08-MAY-2014 V1.0    Sneddon     Initial coding.
**--
*/
#define MODULE PJLCP_STATUS
#define IDENT "V1.0"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/*
** Forward declarations
*/

    int act_echo(void *ctx);
    int act_info(void *ctx);

int act_echo(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        pcbp->pjlbuf = cat(pcbp->pjlbuf, "ECHO");
        break;

    case OP_STORE:
        pcbp->pjlbuf = cat(pcbp->pjlbuf, "%s", pcbp->prs.cur);
        break;

    case OP_FINISH:
        pcbp->flags2.send_pjl = 1;
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_echo */

int act_info(void *ctx) {
    PCBDEF *pcbp = ctx;

    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        break;

    case OP_STORE:
        pcbp->pjlbuf = cat(pcbp->pjlbuf, "INFO %s", keywords[pcbp->prs.av2]);
        break;

    case OP_FINISH:
        if (pcbp->pjlbuf == 0) {
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "INFO ID");
        }
        pcbp->flags2.send_pjl = 1;
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_info */

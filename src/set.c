/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains support for the SET command, this is used to
**  control the session environment.
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
**--
*/
#define MODULE PJLCP_SET
#define IDENT "V1.0"
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

    int act_set(void *ctx);

int act_set(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_STORE:
        switch (pcbp->prs.av2) {
        case 0 /*   AUTO_UEL */: pcbp->flags.auto_uel = 1; break;
        case 1 /* NOAUTO_UEL */: pcbp->flags.auto_uel = 0; break;
        case 2 /*   SUMMARY  */: pcbp->flags.summary = 1; break;
        case 3 /* NOSUMMARY  */: pcbp->flags.summary = 0; break;
        case 4 /*   TIMEOUT=n*/: pcbp->timeout = pcbp->prs.num; break;
        case 5 /* NOTIMEOUT  */: pcbp->timeout = 0; break;
        case 6 /*   DUMP     */: pcbp->flags.dump = 1; break;
        case 7 /* NODUMP     */: pcbp->flags.dump = 0; break;
        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_set */

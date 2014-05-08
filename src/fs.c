/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains support for all PJL File Systems Commands
**  described in Chapter 9 of the "Printer Job Language Technical
**  Reference Manual".
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
**  CREATION DATE:  01-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      01-MAY-2014 V1.0    Sneddon     Initial coding.
**--
*/
#define MODULE PJLCP_FS
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

    int act_fsdirlist(void *ctx);

/*
**++
**  ...
**
**--
*/
int act_fsdirlist(void *ctx) {
    static int entry, count, name;
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        name = count = entry = 0;
        pcbp->pjlbuf = strdup("FSDIRLIST");
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);

        break;

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME:
            name = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%s", pcbp->prs.cur);
            break;

        case KW_ENTRY:
            entry = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "ENTRY=%d", pcbp->prs.num);
            break;

        case KW_COUNT:
            count = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "COUNT=%d", pcbp->prs.num);
            break;
        }

        break;

    case OP_CHECK: {
        char *field = 0;

        switch (pcbp->prs.av2) {
        case KW_NAME:
            if (name) {
                field = "NAME";
                status = ACT_ERROR;
            }
            break;

        case KW_ENTRY:
            if (entry) {
                field = "ENTRY";
                status = ACT_ERROR;
            }
            break;

        case KW_COUNT:
            if (count) {
                field = "COUNT";
                status = ACT_ERROR;
            }
            break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once", field);
        }

        break;
    }

    case OP_FINISH:
        if (!name) {
            error(0, "no pathname specified");
            status = ACT_ERROR;
        } else {
            if (!entry) pcbp->pjlbuf = cat(pcbp->pjlbuf, "ENTRY=1");
            if (!count) pcbp->pjlbuf = cat(pcbp->pjlbuf, "COUNT=99");
        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fsdirlist */

/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains support for all PJL Device Attendance Commands
**  (really for modifying the LCD status display) described in Chapter 8 of
**  the "Printer Job Language Technical Reference Manual".
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
**  CREATION DATE:  21-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      21-MAY-2014 V1.0    Sneddon     Initial coding.
**--
*/
#define MODULE PJLCP_DISPLAY
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

    int act_display(void *ctx);

/*
**++
**  ...
**
**--
*/
int act_display(void *ctx) {
    static int keyword;
    PCBDEF *pcbp = ctx;
    int len, status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        keyword = pcbp->prs.av2;
        pcbp->pjlbuf = strdup(keywords[pcbp->prs.av2]);
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);
        break;

    case OP_STORE:
        len = pcbp->prs.end - pcbp->prs.cur;
        pcbp->pjlbuf = cat(pcbp->pjlbuf, "%s=%-*.*s", keywords[pcbp->prs.av2],
                           len, len, pcbp->prs.cur);
        break;

    case OP_FINISH:
        pcbp->flags2.send_pjl = 1;
        if (keyword == KW_STMSG) {
            pcbp->flags2.expect_ack = 1;
        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_display */

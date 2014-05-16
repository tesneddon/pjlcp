/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains all action routines that build the PJL
**  commands for transmission to the printer.
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
#define MODULE PJLCP_PJL
#define IDENT "V1.0"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/*
** Forward declarations
*/

    int act_pjl_info(void *ctx);
    int act_pjl_rol(void *ctx);
    int act_pjl_single(void *ctx);
    int act_pjl_uel(void *ctx);

int act_pjl_rol(void *ctx) {
    static const char *command[] = {
        "COMMENT", "ECHO"
    };
    PCBDEF *pcbp = ctx;
    int status;

    status = asprintf(&pcbp->pjlbuf, "%s %s", command[pcbp->prs.av1],
                      pcbp->prs.nxt);
    if (status == -1) raise(SIGSEGV);

    return ACT_SUCCESS;
} /* act_pjl_comment */

int act_pjl_info(void *ctx) {
    PCBDEF *pcbp = ctx;

    switch (pcbp->prs.av1) {
    case 0: pcbp->pjlbuf = strdup("INFO CONFIG");    break;
    case 1: pcbp->pjlbuf = strdup("INFO FILESYS");   break;
    case 2: pcbp->pjlbuf = strdup("INFO ID");        break;
    case 3: pcbp->pjlbuf = strdup("INFO INFO");      break;
    case 4: pcbp->pjlbuf = strdup("INFO MEMORY");    break;
    case 5: pcbp->pjlbuf = strdup("INFO PAGECOUNT"); break;
    case 6: pcbp->pjlbuf = strdup("INFO STATUS");    break;
    case 7: pcbp->pjlbuf = strdup("INFO USTATUS");   break;
    case 8: pcbp->pjlbuf = strdup("INFO VARIABLES"); break;
    }

    if (pcbp->pjlbuf == 0) raise(SIGSEGV);

    return ACT_SUCCESS;
} /* act_pjl_info */

int act_pjl_single(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status;

    switch (pcbp->prs.av1) {
    case 0: pcbp->pjlbuf = strdup("INITIALIZE"); break;
    case 1: pcbp->pjlbuf = strdup("RESET"); break;
    case 2: pcbp->pjlbuf = strdup("USTATUSOFF"); break;
    }
    if (pcbp->pjlbuf == 0) raise(SIGSEGV);

    return ACT_SUCCESS;
} /* act_pjl_single */

int act_pjl_uel(void *ctx) {
    PCBDEF *pcbp = ctx;
    int auto_uel = pcbp->flags.auto_uel;

    pcbp->flags.auto_uel = 1;
    pcbp->flags.auto_uel = auto_uel;

    return ACT_SUCCESS;
} /* act_pjl_uel */


/*
**++
**  MODULE DESCRIPTION:
**
**      Miscellaneous support routines for the PJLCP CLI.
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
#define MODULE PJLCP_MISC
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

    int act_eol(void *ctx);
    int act_exit(void *ctx);
    int act_please(void *ctx);

int act_eol(void *ctx) {
    PCBDEF *pcbp = ctx;

    warn(0, "trailing garbage ignored\n \\%s\\", pcbp->prs.cur);
    return ACT_ERROR;
} /* act_eol */

int act_exit(void *ctx) {
    PCBDEF *pcbp = ctx;

    pcbp->flags2.exit = 1;
    return ACT_SUCCESS;
} /* act_exit */

int act_please(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        switch (pcbp->prs.av2) {
        case 0:
            info(0, "thank you for asking, how may I help you?");
            break;

        case 1:
            error(0, "can't even ask properly?");
            status = ACT_ERROR;
            break;

        case 2:
            error(0, "plugh!");
            status = ACT_ERROR;
            break;
        }
        break;

    case OP_FINISH:
        switch (pcbp->prs.av2) {
        case 0:             /* PLEASE HIDE */
            break;

        default:
            break;
        }

    default:
        break;
    }

    return status;
} /* act_exit */

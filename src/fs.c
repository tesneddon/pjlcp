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
**      21-MAY-2014 V1.1    Sneddon     Fix range limits of NAME value.
**                                      Add FSDELETE, FSMKDIR, FSQUERY.
**      21-MAY-2014 V1.2    Sneddon     Add FSPWD.
**--
*/
#define MODULE PJLCP_FS
#define IDENT "V1.1"
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

    int act_fschdir(void *ctx);
    int act_fsdelete(void *ctx);
    int act_fsdirlist(void *ctx);
    int act_fsmkdir(void *ctx);
    int act_fspwd(void *ctx);
    int act_fsquery(void *ctx);
    int act_upload(void *ctx);

int act_fschdir(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    return ACT_ERROR;
} /* act_fschdir */

int act_fsdelete(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        pcbp->pjlbuf = strdup("FSDELETE");
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);

        break;

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME: {
            const int len = pcbp->prs.end - pcbp->prs.cur;

            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%-*.*s",
                               len, len, pcbp->prs.cur);
            break;
        }

        default:
            break;
        }

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
} /* act_fsdelete */

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
        case KW_NAME: {
            const int len = pcbp->prs.end - pcbp->prs.cur;

            name = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%-*.*s",
                               len, len, pcbp->prs.cur);
            break;
        }

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
            pcbp->flags2.send_pjl = pcbp->flags2.expect_ack = 1;
        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fsdirlist */

int act_fsmkdir(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        pcbp->pjlbuf = strdup("FSMKDIR");
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);

        break;

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME: {
            const int len = pcbp->prs.end - pcbp->prs.cur;

            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%-*.*s",
                               len, len, pcbp->prs.cur);
            break;
        }

        default:
            break;
        }

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
} /* act_fsmkdir */

int act_fspwd(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_FINISH:
        pcbp->flags2.send_pjl = pcbp->flags2.pseudo = 1;
        pcbp->pjlbuf = cat(0, "FSPWD NAME=\"%s\" TYPE=DIR", pcbp->fspath);
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fspwd */

int act_fsquery(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        pcbp->pjlbuf = strdup("FSQUERY");
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);

        break;

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME: {
            const int len = pcbp->prs.end - pcbp->prs.cur;

            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%-*.*s",
                               len, len, pcbp->prs.cur);
            break;
        }

        default:
            break;
        }

        break;

    case OP_FINISH:
        pcbp->flags2.send_pjl = pcbp->flags2.expect_ack = 1;
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fsquery */

int act_fsupload(void *ctx) {
    static int offset, size, name;
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        name = offset = size = 0;
        pcbp->pjlbuf = strdup("FSUPLOAD");
        if (pcbp->pjlbuf == 0) raise(SIGSEGV);

        break;

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME: {
            const int len = pcbp->prs.end - pcbp->prs.cur;

            name = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=%-*.*s",
                               len, len, pcbp->prs.cur);
            break;
        }

        case KW_OFFSET:
            offset = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "OFFSET=%d", pcbp->prs.num);
            break;

        case KW_SIZE:
            size = 1;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "SIZE=%d", pcbp->prs.num);
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

        case KW_OFFSET:
            if (offset) {
                field = "OFFSET";
                status = ACT_ERROR;
            }
            break;

        case KW_SIZE:
            if (size) {
                field = "SIZE";
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
            if (!offset) pcbp->pjlbuf = cat(pcbp->pjlbuf, "OFFSET=0");
            if (!size) pcbp->pjlbuf = cat(pcbp->pjlbuf, "SIZE=9999");
            pcbp->flags2.send_pjl = pcbp->flags2.expect_ack = 1;
        }
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fsupload */

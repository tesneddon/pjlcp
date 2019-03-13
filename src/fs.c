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
**      22-MAY-2014 V1.2    Sneddon     Add OUTPUT to FSUPLOAD command.  Add
**                                      FSDOWNLOAD.
**--
*/
#define MODULE PJLCP_FS
#define IDENT "V1.2"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "common.h"
#include "fsutil.h"
#define BUFMAX 512

/*
** Forward declarations
*/

    int act_fschdir(void *ctx);
    int act_fsdelete(void *ctx);
    int act_fsdirlist(void *ctx);
    int act_fsdown(void *ctx);
    int act_fsenv(void *ctx);
    int act_fsmkdir(void *ctx);
    int act_fspwd(void *ctx);
    int act_fsquery(void *ctx);
    int act_fsupload(void *ctx);

int act_fschdir(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    printf("fschdir %d, %d\n", pcbp->prs.av1, pcbp->prs.av2);

    switch (pcbp->prs.av1) {
    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_NAME: {
            char *path, *result;

            /*
            ** Save the value of NAME, without the quotes.
            */
            path = strndup(pcbp->prs.cur + 1,
                           (pcbp->prs.end - pcbp->prs.cur) - 2);
            if (path == 0) raise(SIGSEGV);

            result = fsget_path(path, pcbp->pwd, pcbp->fsmask);
            if (result == 0) {
                error(errno, "unable to resolve path");
                status = ACT_ERROR;
            } else {
                free(pcbp->pwd);
                pcbp->pwd = result;
            }
            free(path);

            break;
        }

        default:
            break;
        }

        break;

    case OP_FINISH:
        /*
        ** At this point, we should actually be sending an FSQUERY that we
        ** parse the response from and then use to set the default path.
        **
        ** However, correct handling of return traffic is still being written
        ** and so we short cut it.  Unfortunately that means we can
        ** FSCHDIR to a path that doesn't exist.
        */
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
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
        switch (pcbp->prs.av2) {
        default: break;
        case KW_COUNT: if (count) status = ACT_ERROR; break;
        case KW_ENTRY: if (entry) status = ACT_ERROR; break;
        case KW_NAME:  if (name)  status = ACT_ERROR; break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once",
                  keywords[pcbp->prs.av2]);
        }

        break;
    }

    case OP_FINISH:
        if (!name) pcbp->pjlbuf = cat(pcbp->pjlbuf, "NAME=\"%s\"", pcbp->pwd);
        if (!entry) pcbp->pjlbuf = cat(pcbp->pjlbuf, "ENTRY=1");
        if (!count) pcbp->pjlbuf = cat(pcbp->pjlbuf, "COUNT=99");
        pcbp->flags2.send_pjl = pcbp->flags2.expect_ack = 1;
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_fsdirlist */

int act_fsdown(void *ctx) {
    static int name, size;
    static char *input;
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;
#if 0
    switch (pcbp->prs.av1) {
    case OP_INIT:
        name = 0;
        input = 0;
        size = -1;
        pcbp->pjlbuf = strdup("FSDOWNLOAD");
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

        case KW_SIZE:
            size = pcbp->prs.num;
            pcbp->pjlbuf = cat(pcbp->pjlbuf, "SIZE=%d", size);
            break;

        case KW_INPUT:
            /*
            ** Copy the local filename path, without the surrounding ".
            */
            input = strndup(pcbp->prs.cur+1,
                            (pcbp->prs.end - pcbp->prs.cur) - 2);
            if (input == 0) raise(SIGSEGV);
            break;
        }

        break;

    case OP_CHECK: {
        switch (pcbp->prs.av2) {
        case KW_NAME:
            if (name) {
                status = ACT_ERROR;
            }
            break;

        case KW_SIZE:
            if (size != -1) {
                status = ACT_ERROR;
            }
            break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once",
                  keywords[pcbp->prs.av2]);
        }

        break;
    }

    case OP_FINISH:
        if (!name) {
            error(0, "no pathname specified");
            status = ACT_ERROR;
        } else {
            char *data = 0;

            if (input == 0) {
                /*
                ** The user did not supply a file for input, so we'll take
                ** the data off the terminal.
                */
                char buf[BUFMAX], *result = 0;

                do {
                    result = fgets(buf, sizeof(buf), stdin);
                    if (result != 0) {
                        data = cat(data, "%s", buf);
                        if (data == 0) raise(SIGSEGV);

                        if (size != -1) {
                            int data_len = strlen(data);

                            if (data_len >  size) {
                                data[size] = '\0';
                                warn(0, "input exceeded SIZE='%d' by %d byte(s), truncating",
                                     size, size - data_len);
                                break;
                            }
                        }
                    }
                } while (result != 0);

                if (size == -1) {
                    size = strlen(data);
                    pcbp->pjlbuf = cat(pcbp->pjlbuf, "SIZE=%d", size);
                    if (pcbp->pjlbuf == 0) raise(SIGSEGV);
                }
            } else {
                /*
                ** The user supplied a file for us to upload, so open it,
                ** map it into memory and append the relelvant number of bytes
                ** to pjlbuf.
                */
                int fd;

                fd = open(input, 0, O_RDONLY);
                if (fd == -1) {
                    // error
                } else {
                    status = fstat(fd, &stat);
                    if (status == -1) {
                        // error
                    } else {
                        if (size == -1) {
                            /*
                            ** The user did not supply a size, so we assume
                            ** it is the entire file.
                            */
                            size = stat.st_size;
                            pcbp->pjlbuf = cat(pcbp->pjlbuf, "SIZE=%d", size);
                            if (pcbp->pjlbuf == 0) raise(SIGSEGV);
                        }

                        data = mmap(0, size, PROT_READ,
                                   MAP_FILE, MAP_SHARED, fd, 0);
                        if (data == MAP_FAILED) {
                            // error
                        }
                    }
                }
            }

            if (status == ACT_SUCCESS) {

//                pcbp->pjlbuf = cat(pcbp->pjlbuf, "\r\n

            // if status is okay,
                // attach buf of size
                // attach terminating UEL
            }

            // if input != 0
                // unmap and close file
            // else
                // free buf...

//            pcbp->flags2.send_pjl = 1;
        }

        if (input != 0) free(input);
        break;

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }
#endif
    return status;
} /* act_fsdown */

int act_fsenv(void *ctx) {
    static unsigned off, on, option, seen;
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        off = on = option = seen = 0;
        break;

    case OP_CHECK: {
        switch (pcbp->prs.av2) {
        case KW_COMPACT:
            if (seen & O_FSCOMPACT) {
                status = ACT_ERROR;
            } else {
                seen |= option = O_FSCOMPACT;
            }
            break;

        case KW_ITEMS:
            if (seen & O_FSITEMS) {
                status = ACT_ERROR;
            } else {
                seen |= option = O_FSITEMS;
            }
            break;

        case KW_LENGTH:
            if (seen & O_FSLENGTH) {
                status = ACT_ERROR;
            } else {
                seen |= option = O_FSLENGTH;
            }
            break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once",
                  keywords[pcbp->prs.av2]);
        }
        break;
    }

    case OP_STORE:
        switch (pcbp->prs.av2) {
        case KW_ON:   on |= option; break;
        case KW_OFF: off |= option; break;
        }
        option = 0;
        break;

    case OP_FINISH:
        pcbp->fsmask |= on;
        pcbp->fsmask &= ~off;
        break;
    }

    return status;
} /* act_fsenv */

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

    fprintf(stdout, "@PJL FSPWD\nNAME=\"%s\"\n", pcbp->pwd);

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
    static int offset, size, name, output;
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:
        name = offset = size = output = 0;
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

        case KW_OUTPUT:
            output = 1;
            pcbp->output = strndup(pcbp->prs.cur,
                                   pcbp->prs.end - pcbp->prs.cur);
            if (pcbp->output == 0) raise(SIGSEGV);
            break;
        }

        break;

    case OP_CHECK: {
        char *field = 0;

        switch (pcbp->prs.av2) {
        default: break;
        case KW_NAME:   if (name)   status = ACT_ERROR; break;
        case KW_OFFSET: if (offset) status = ACT_ERROR; break;
        case KW_SIZE:   if (size)   status = ACT_ERROR; break;
        case KW_OUTPUT: if (output) status = ACT_ERROR; break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once",
                  keywords[pcbp->prs.av2]);
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

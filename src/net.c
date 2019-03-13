/*
**++
**  MODULE DESCRIPTION:
**
**      All routines related to network connectivity.
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
**      20-MAY-2014 V1.1    Sneddon     Removed SHOW CONNECTION.  Add init
**                                      of I/O counters.
**--
*/
#define MODULE PJLCP_NET
#define IDENT "V1.2"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "common.h"

/*
** Forward declarations
*/

    int act_connect(void *ctx);
    int act_disconnect(void *ctx);
    int send_pjl(void *ctx, char *buf, int len, int expect_response);

int act_connect(void *ctx) {
    static int name, port;

    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    switch (pcbp->prs.av1) {
    case OP_INIT:       /* Initialize storage */
        name = port = 0;
        if (pcbp->sock != -1) {
            error(EISCONN, "cannot open connection");
            status = ACT_ERROR;
        } else {
            /*
            ** Initialize the connection context.
            */
            pcbp->rcnt = pcbp->wcnt = 0;
            pcbp->hostname = 0;
            pcbp->port = DEFAULT_PORT;
            pcbp->pwd = cat(0, DEFAULT_PATH);
            pcbp->fsmask = -1;
        }
        break;

    case OP_CHECK:
        /*
        ** Check we haven't doubled up on arguments.
        */
        switch (pcbp->prs.av2) {
        default: break;
        case KW_NAME: if (name) status = ACT_ERROR; break;
        case KW_PORT: if (port) status = ACT_ERROR; break;
        }

        if (status == ACT_ERROR) {
            error(0, "'%s' argument can only appear once",
                  keywords[pcbp->prs.av2]);
        }
        break;

    case OP_STORE:      /* Store value from command line parse */
        switch (pcbp->prs.av2) {
        case KW_NAME:         /* Store hostname */
            name = 1;
            pcbp->hostname = strip(pcbp->prs.cur,
                                   pcbp->prs.end - pcbp->prs.cur);
            if (pcbp->hostname == 0) raise(SIGSEGV);
            break;

        case KW_PORT:         /* Store port number */
            switch (pcbp->prs.av3) {
            case 0:           /* Store numeric value */
                pcbp->port = pcbp->prs.num;
                break;

            case 1: {         /* Store named port number */
                struct servent *entry;
                char *str;

                str = strndup(pcbp->prs.cur, pcbp->prs.end - pcbp->prs.cur);
                if (str == 0) raise(SIGSEGV);

                entry = getservbyname(str, 0);
                if (entry == 0) {
                    error(0, "unable to translate service name '%s'", name);
                    status = ACT_ERROR;
                } else {
                    pcbp->port = ntohs(entry->s_port);
                }
                free(str);
                break;
            }

            }
            port = 1;
            break;
        }
        break;

    case OP_FINISH: {   /* Make the connection */
        struct hostent *entry;

        if (!name) {
            error(0, "no hostname specified");
            status = ACT_ERROR;
        } else {
            entry = gethostbyname(pcbp->hostname);
            if (entry == 0) {
                error(h_errno, "cannot resolve hostname '%s'",
                      pcbp->hostname);
                status = ACT_ERROR;
            } else {
                pcbp->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (pcbp->sock < 0) {
                    error(errno, "unable to open socket");
                    status = ACT_ERROR;
                } else {
                    int i;

                    pcbp->addr.sin_family = AF_INET;
                    pcbp->addr.sin_port = htons(pcbp->port);
                    for (i = 0; entry->h_addr_list[i] != 0; i++) {
                        pcbp->addr.sin_addr.s_addr =
                                          *(in_addr_t *) entry->h_addr_list[i];
                        status = connect(pcbp->sock,
                                         (struct sockaddr *)&pcbp->addr,
                                         sizeof(pcbp->addr));
                        if (status < 0) {
                            if ((errno != ECONNREFUSED)
                                && (errno != ENETUNREACH)
                                && (errno != ETIMEDOUT)) {
                                /*
                                ** Don't bother trying any other addresses
                                ** for any other error.
                                */
                                break;
                            }
                        } else {
                            status = ACT_SUCCESS;
                            break;
                        }
                    }

                    if (status < 0) {
                        error(errno, "unable to connect to printer");
                        status = ACT_ERROR;
                    }
                }
            }
        }

        if (status == ACT_ERROR) {
            if (pcbp->sock != -1) {
                close(pcbp->sock);
                pcbp->sock = -1;
            }
            free(pcbp->hostname);
            pcbp->hostname = 0;
        }

        break;
    }

    default:
        error(EPERM, "operation not supported by this command");
        status = ACT_ERROR;
        break;
    }

    return status;
} /* act_connect */

int act_disconnect(void *ctx) {

    PCBDEF *pcbp = ctx;

    if (pcbp->sock == -1) {
        error(ENOTCONN, "unable to disconnect printer");
        return ACT_SUCCESS;
    }

    close(pcbp->sock);
    pcbp->sock = -1;
    free(pcbp->hostname);

    return ACT_SUCCESS;
} /* act_disconnect */

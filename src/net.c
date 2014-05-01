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
**--
*/
#define MODULE PJLCP_NET
#define VERSION "V1.0"
#ifdef __VMS
# pragma module MODULE VERSION
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "common.h"

/*
** Forward declarations
*/

    int act_connect(void *ctx);
    int act_disconnect(void *ctx);
    int act_is_connected(void *ctx);
    int act_show_connection(void *ctx);

int act_connect(void *ctx) {

    PCBDEF *pcbp = ctx;
    char *ptr;
    size_t len;
    int status = ACT_SUCCESS;

    if (pcbp->sock != -1) {
        error(EISCONN, "cannot open connection");
        return ACT_ERROR;
    }

    switch (pcbp->prs.av1) {
    case 0: {        /* Make the connection */
        struct hostent *entry;

        entry = gethostbyname(pcbp->hostname);
        if (entry == 0) {
            error(h_errno, "cannot resolve hostname '%s'", pcbp->hostname);
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
                    pcbp->addr.sin_addr.s_addr = *(in_addr_t *) entry->h_addr_list[i];
                    status = connect(pcbp->sock,
                                     (struct sockaddr *)&pcbp->addr,
                                     sizeof(pcbp->addr));
                    if (status < 0) {
                        if ((errno != ECONNREFUSED)
                            && (errno != ENETUNREACH)
                            && (errno != ETIMEDOUT)) {
                            /*
                            ** Don't bother trying any other addresses for
                            ** any other error.
                            */
                            break;
                        }
                    } else {
                        break;
                    }
                }

                if (status < 0) {
                    error(errno, "unable to connect to printer");
                    status = ACT_ERROR;
                } else {
                    status = ACT_SUCCESS;
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

    case 1:         /* Store the hostname */
        ptr = pcbp->prs.cur;
        len = pcbp->prs.end - pcbp->prs.cur;

        pcbp->hostname = strndup(ptr, len);;
        if (pcbp->hostname == 0) raise(SIGSEGV);
        pcbp->port = DEFAULT_PORT;
        break;

    case 2:         /* Store an integer port number */
        pcbp->port = (unsigned short) pcbp->prs.num;
        break;

    case 3: {        /* Store service name */
        struct servent *entry;

        len = pcbp->prs.end - pcbp->prs.cur;
        ptr = strndup(pcbp->prs.cur, len);
        if (ptr == 0) raise(SIGSEGV);

        entry = getservbyname(ptr, 0);
        if (entry == 0) {
            error(0, "unable to translate service name '%s'", ptr);
            status = ACT_ERROR;
        } else {
            pcbp->port = ntohs(entry->s_port);
        }
        free(ptr);
        break;
    }

    }

    return status;
} /* act_connect */

int act_disconnect(void *ctx) {

    PCBDEF *pcbp = ctx;

    if (pcbp->sock == -1) {
        error(ENOTCONN, "unable to disconnect printer");
        return ACT_ERROR;
    }

    close(pcbp->sock);
    pcbp->sock = -1;
    free(pcbp->hostname);

    return ACT_SUCCESS;
} /* act_disconnect */

int act_is_connected(void *ctx) {
    PCBDEF *pcbp = ctx;
    int status = ACT_SUCCESS;

    if (pcbp->sock == -1) {
        error(ENOTCONN, "cannot execute PJL commands");
        status = ACT_ERROR;
    }

    return status;
}

int act_show_connection(void *ctx) {

    PCBDEF *pcbp = ctx;

    if (pcbp->sock == -1) {
        error(ENOTCONN, 0);
        return ACT_ERROR;
    }

    return ACT_SUCCESS;
} /* act_show_connection */

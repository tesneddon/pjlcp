/*
** COMMON.H
**
**  Header file containing common definitions for PJLCP.
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
**  08-MAY-2014  Sneddon    Added cat() macro for building PJL commands.
*/
#ifndef PJLCP_COMMON_H_
#define PJLCP_COMMON_H_
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <prsdef.h>
#include "keywords.h"
#include "operators.h"

#define DEFAULT_PORT 9100

/*
** Parser Control Block.  This is use by the pr_command routine to store
** sate is prs.  The remaining fields are used by the action routines.
*/

    typedef struct _pcb {
        struct prs prs;             /* PRS Control Block -- must be first   */
        char *pjlbuf;               /* PJL command buffer                   */
        char *hostname;             /* Hostname of current connection       */
        unsigned short port;        /* Post number (host order)             */
        struct sockaddr_in addr;    /* Socket address                       */
        int sock;                   /* Active connection socket (-1 if not) */
        unsigned int wcnt, rcnt;    /* Read/write byte count                */
        struct {
            unsigned auto_uel : 1;  /* Automatically prefix UEL             */
            unsigned summary : 1;   /* Report network traffic counts        */
        } flags;
        struct {
            unsigned exit : 1;      /* Leave gracefully                     */
        } flags2;
    } PCBDEF;

/*
** Action routines
*/

    int act_connect(void *ctx);
    int act_disconnect(void *ctx);
    int act_exit(void *ctx);
    int act_help(void *ctx);
    int act_is_connected(void *ctx);
    int act_set_flag(void *ctx);
    int act_show_connection(void *ctx);
    int act_show_version(void *ctx);

    int act_pjl_info(void *ctx);
    int act_pjl_rol(void *ctx);
    int act_pjl_single(void *ctx);
    int act_pjl_uel(void *ctx);
    int act_pjl_xmit(void *ctx);

/*
** main.c
*/

    void version(void);

/*
** net.c
*/

    int send_pjl(void *ctx, char *buf, int len, int expect_response);

/*
** Userful macros.
*/

#define cat(dest, src, ...)                                             \
do {                                                                    \
    char *newp;                                                         \
    if (asprintf(&newp, "%s " src, dest, ##__VA_ARGS__) == -1)          \
        raise(SIGSEGV);                                                 \
    free(dest);                                                         \
    dest = newp;                                                        \
} while (0)

#endif /* PJLCP_COMMON_H_ */

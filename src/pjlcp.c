/*
**++
**  MODULE DESCRIPTION:
**
**      Main entry point for the HP PJL Control Program (PJLCP).
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
#include "version.h"
#define MODULE PJLCP_ROOT
#define IDENT PJLCP_VERSION
#ifdef __VMS
# pragma module MODULE IDENT
# pragma extern_model save
# pragma extern_model common_block shr
    char $$Copyright[] = PJLCP_COPYRIGHT;
# pragma extern_model restore
#endif
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "action_routines_decl.h"
#define RECVBUF_MAX 1024

/*
** Forward routines
*/

    int main(int argc, char *argv[]);
    static void usage(int brief);
    void version(void) {
        fprintf(stderr, "%s - HP PJL Control Program %s\n%s\n",
                program_invocation_short_name, PJLCP_VERSION, PJLCP_COPYRIGHT);
    } /* version */

/*
** Own storage
*/

    static struct action actions[] = {
#include "action_routines_init.h"
    };

int main(int argc,
         char *argv[]) {

    PCBDEF pcb;
    char buf[512] = { 0 }, recvbuf[RECVBUF_MAX];
    ssize_t count;
    int status;

    /*
    ** Set up the parse control block.
    */
    memset(&pcb, 0, sizeof(pcb));
    pcb.prs.cmd = buf;
    pcb.prs.cms = sizeof(buf);
    pcb.prs.tbs = 0;
    pcb.prs.act = actions;

    pcb.sock = -1;
    pcb.flags.auto_uel = 1;

    /*
    ** Read in the parse table.
    */
    status = pr_read(&pcb.prs, "pjlcp.bin");
    if (status == RET_SIZE) {
        pcb.prs.tbl = malloc(pcb.prs.tbu);
        if (pcb.prs.tbl == 0) raise(SIGSEGV);
        pcb.prs.tbs = pcb.prs.tbu;

        status = pr_read(&pcb.prs, "pjlcp.bin");
    }

    if (status != RET_SUCCESS) {
        error(0, "unable to read parse table definition");
    } else {
        /*
        ** For interactive sessions we herald the user...
        */
        if (isatty(STDIN_FILENO)) version();
        fputc('\n', stderr);

        /*
        ** Start parsing commands...
        */
        do {
            status = pr_command(&pcb.prs);
            switch (status) {
            case RET_SUCCESS:
                if (pcb.flags2.send_pjl) {
                    /*
                    ** The dispatched command has requested transmission
                    ** of a PJL command to the printer.
                    */
                    size_t buflen, recvd = 0, sent = 0;

                    if (pcb.sock == -1) {
                        error(ENOTCONN, "cannot send command to printer");
                    } else {
                        /*
                        ** Build the PJL command, like so:
                        **
                        **   [ UEL ]@PJL [ PJLBUF ]<CR><LF>
                        */
                        ssize_t count;
                        char *outbuf = 0;

                        buflen = asprintf(&outbuf, "%s@PJL %s\r\n",
                                    pcb.flags.auto_uel ? "\033%%-12345X" : "",
                                    pcb.pjlbuf == 0 ? "" : pcb.pjlbuf);
                        if (buflen == -1) raise(SIGSEGV);

                        /*
                        ** Transmit the entire command to the printer, in
                        ** chunks, if necessary.
                        */
                        do {
                            count = send(pcb.sock, outbuf+sent,
                                         buflen-sent, 0);
                            if (count < 0) {
                                error(errno, "failed to transmit command");
                                break;
                            } else {
                                if (pcb.flags.dump)
                                    dump(0, outbuf+sent, buflen-sent);

                                sent += count;
                            }
                        } while (sent < buflen);
                        free(outbuf);

                        if (pcb.flags2.expect_ack) {

// need a SIGIO routine that can read an dump a response

// disable SIGIO

                            /*
                            ** This command expects a response from the
                            ** printer.  So, fetch it an dump it at
                            ** the terminal.
                            */
                            static char inbuf[BUFSIZ];

// sigsetjmp -- to catch ^C

                            do {

// loop, reading in buffer
                                buflen = recv(pcb.sock, inbuf,
                                              sizeof(inbuf), 0);
                                if (buflen < 0) {
                                    error(errno, "error receiving response");
                                } else if (buflen == 0) {
                                    error(0, "remote node closed connection");
                                    act_disconnect(&pcb);
                                } else {
                                    int i;

                                    if (pcb.flags.dump)
                                        dump(0, inbuf, buflen);
                                    recvd += buflen;

                                    for (i = 0; i < buflen; i++) {
                                        char c = inbuf[i];

                                        if (c == '\f') {
                                            buflen = 0;
                                            break;
                                        }
                                        fputc(c, stdout);
                                    }
                                }
                            } while (buflen > 0);

// pass buffer to parse routine

// restore SIGIO

                        }
                    }

                    if (pcb.flags.summary) {
                        info(0, "Sent %d byte(s); Received %d byte(s)",
                             sent, recvd);
                    }

                    pcb.rcnt += recvd;
                    pcb.wcnt += sent;
                }
                break;

            case RET_EOF:
                status = RET_QUIT;
#ifndef __VMS
                fputc('\n', stdout);
#endif
                break;

            case RET_FILE:
                /* ? */

            case RET_ERROR:
            default:
                break;
            }

            /*
            ** Clear the command buffer as well as free up and PJL
            ** buffer.
            */
            buf[0] = '\0';
            if (pcb.pjlbuf != 0) {
                free(pcb.pjlbuf);
                pcb.pjlbuf = 0;
            }
            if (pcb.output != 0) {
                free(pcb.output);
                pcb.output = 0;
            }
            pcb.flags2.send_pjl = pcb.flags2.expect_ack = 0;
            if (pcb.flags2.exit) status = RET_QUIT;
        } while (status != RET_QUIT);
    }

    return EXIT_SUCCESS;
}

static void usage(int brief) {
    if (brief) {
        fprintf(stderr, "Try `%s --help' for more information.\n",
                program_invocation_short_name);
    } else {
        fprintf(stderr, "\
Usage: %s [OPTION]... FILE [, ...]\n\
....\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -h, --help                display this help and exit\n\
  -v, --version             output version information and exit\n\
\n", program_invocation_short_name);
    }

} /* usage */


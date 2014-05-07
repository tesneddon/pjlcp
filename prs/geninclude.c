/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains the main entry point for geninclude (originally
**  parser_aid_2).  It identifies all the action routines in a parser file
**  and generates include files containing their definitions.
**
**      This module was part fo the DCE reference implementation offered by
**  the Open Group.  However, it was originally developeed at Network &
**  Communications Software Engineering, Digital Equipment Corporation.
**
**  AUTHOR:
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
**  @OSF_COPYRIGHT@
**  COPYRIGHT NOTICE
**  Copyright (c) 1990, 1991, 1992, 1993, 1994, 1996 Open Software Foundation, Inc.
**  ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE for
**  the full copyright text.
**
**  COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1990-1994. ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
**  SOFTWARE ON EQUIPMENT THAT IS NOT SUPPLIED BY DIGITAL.
**
**  CREATION DATE:
**
**  MODIFICATION HISTORY:
**
**      06-May-2014 V1.0    Sneddon   Big tidy up.
**      06-May-2014 V1.1    Sneddon   Changed to use FNV-1a hash.
**
**  Revision 1.1.11.2  1996/02/18  19:30:03  marty
**      Update OSF copyright years
**      [1996/02/18  18:11:38  marty]
**
**  Revision 1.1.11.1  1995/12/08  15:11:08  root
**      Submit OSF/DCE 1.2.1
**      [1995/12/08  14:40:57  root]
**
**  Revision 1.1.8.3  1994/06/09  18:37:36  devsrc
**      cr10871 - expand copyright
**      [1994/06/09  18:09:34  devsrc]
**
**  Revision 1.1.8.2  1994/03/16  23:40:40  jd
**      Included string.h to pick up retrun type of strtok.
**      [1994/03/16  21:27:25  jd]
**
**  Revision 1.1.8.1  1994/03/12  22:00:36  peckham
**      DEC serviceability and i18n drop
**      [1994/03/12  14:05:41  peckham]
**
**  Revision 1.1.6.2  1993/06/24  19:47:12  hinman
**      [hinman@sni] - Check in SNI merged version (these files have no
**                     tainted or private code
**      [1993/06/17  14:36:35  hinman]
**
**  Revision 1.1.4.3  1992/12/30  13:08:32  zeliff
**      Embedding copyright notices
**      [1992/12/29  22:40:18  zeliff]
**
**  Revision 1.1.4.2  1992/12/15  15:54:48  tom
**      Bug 6409 - add setlocale call for i18n.
**      [1992/12/15  15:17:08  tom]
**
**  Revision 1.1.2.2  1992/03/22  21:53:34  weisman
**              Big merge with post-1.0 development at Digital.
**      [1992/03/22  20:23:14  weisman]
**
**  Revision 1.1  1992/01/19  15:22:01  devrcs
**      Initial revision
**--
*/
#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fnv.h"
#define HT_MAX 0xff
/*
** Forward definitions
*/

    int main(int argc, char *argv[]);
    static void pass_1_file(char *name);
    static void username(char *word);
    static unsigned hash(char *word);

/*
** Own storage
*/

    static struct itm {
        struct itm *next;
        char *np;
    } *ht[HT_MAX];

int main(int argc,
         char *argv[]) {
    FILE *fpout1;                       /* ptr to decl output file */
    FILE *fpout2;                       /* ptr to init output file */
    int i;

    if (argc < 4) {
        fprintf(stderr, "usage: %s decl_outfile init_outfile infile...\n",
                program_invocation_short_name);
        exit(EXIT_FAILURE);
    } else {
        memset(ht, 0, sizeof(ht));

        for (i = 3; i < argc; i++) {
            pass_1_file(argv[i]);
        }
    }

    fpout1 = fopen(argv[1], "w");       /* open the decl output file */
    fpout2 = fopen(argv[2], "w");       /* open the init output file */

    if (!fpout1 || !fpout2) {
        fprintf(stderr, "%s: can not open output file(s)\n",
                program_invocation_short_name);
        exit(EXIT_FAILURE);
    } else {
        for (i = 0; i < HT_MAX; i++) {
            struct itm *itmp = ht[i];

            while (itmp != 0) {
                fprintf(fpout1, "int %s(void *ctx);\n", itmp->np);
                fprintf(fpout2, "{%s, \"%s\"},\n", itmp->np, itmp->np);
                itmp = itmp->next;
            }
        }
    }

    fclose(fpout1);
    fclose(fpout2);

    printf("end of %s\n", program_invocation_short_name);

    exit(EXIT_SUCCESS);
}

static void pass_1_file(char *name) {
    char buf[5555], *status;
    FILE *fpin = fopen(name, "r");      /* ptr to current input file */

    while ((status = fgets(buf, sizeof(buf), fpin)) != NULL) {
        char *s = buf;
        while ((s = strtok(s, " \t")) != NULL) {
            if (strcasecmp(s, "CALL") == 0) {
                if ((s = strtok(NULL, ": \t")) == NULL) break;
                username(s);
            }
            s = 0;
        }
    }

    fclose(fpin);
}

static void username(char *word) {
    int i = hash(word);
    struct itm **itmp = &ht[i];

    while (*itmp != 0) {
        if (strcmp((*itmp)->np, word) == 0) return;
        itmp = &(*itmp)->next;
    }

    *itmp = calloc(sizeof(struct itm), 1);
    if (*itmp == 0) raise(SIGSEGV);

    (*itmp)->np = strdup(word);
    if ((*itmp)->np == 0) raise(SIGSEGV);
}

unsigned hash(char *str) {
    Fnv32_t val;
#define TINY_MASK(x) (((u_int32_t)1<<(x))-1)

    val = fnv_32a_str(str, FNV_32_PRIME);
    return ((val >> 8) ^ val) & TINY_MASK(8);
}

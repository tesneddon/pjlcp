/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains the main entry point for genkeywords.  It
**  identifies all the keywords defined in a parser file and generates
**  include and source files containing their definitions.
**
**  AUTHOR:     Tim Sneddon <tsneddon@endlesssoftware.com.au>
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
**      08-May-2014 V1.0    Sneddon   Built from a modified genincludes.
**--
*/
#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fnv.h"
#define HT_MAX 256
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
    int i, j;

    if (argc < 5) {
        fprintf(stderr, "usage: %s prefix h_outfile c_outfile infile...\n",
                program_invocation_short_name);
        exit(EXIT_FAILURE);
    } else {
        memset(ht, 0, sizeof(ht));

        for (i = 4; i < argc; i++) {
            pass_1_file(argv[i]);
        }
    }

    fpout1 = fopen(argv[2], "w");       /* open the decl output file */
    fpout2 = fopen(argv[3], "w");       /* open the init output file */

    if (!fpout1 || !fpout2) {
        fprintf(stderr, "%s: can not open output file(s)\n",
                program_invocation_short_name);
        exit(EXIT_FAILURE);
    } else {
        fputs("#ifndef PRS_KEYWORDS_\n", fpout1);
        fputs("#define PRS_KEYWORDS_\n", fpout1);
        fputs("#ifdef PRS_KEYWORDS_EXTERN\n", fpout1);
        fputs("    extern const char *keywords[];\n", fpout1);
        fputs("#endif /* PRS_KEYWORDS_EXTERN */\n", fpout1);

        fputs("const char *keywords[] = {\n", fpout2);

        for (i = 0, j = 0; i < HT_MAX; i++) {
            struct itm *itmp = ht[i];

            while (itmp != 0) {
                fprintf(fpout1, "#define %s%s %d\n", argv[1], itmp->np, j);
                fprintf(fpout2, "    \"%s\",\n", itmp->np);
                itmp = itmp->next;

                j++;
            }
        }

        fputs("};\n", fpout2);

        fputs("#endif /* PRS_KEYWORDS_ */\n", fpout1);
    }

    fclose(fpout1);
    fclose(fpout2);

    exit(EXIT_SUCCESS);
}

static void pass_1_file(char *name) {
    char buf[5555], *status;
    FILE *fpin = fopen(name, "r");      /* ptr to current input file */

    while ((status = fgets(buf, sizeof(buf), fpin)) != NULL) {
        char *s = buf;
        while ((s = strtok(s, " \t")) != NULL) {
            if (strcasecmp(s, "WORD") == 0) {
                if ((s = strtok(NULL, " \t")) == NULL) break;
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

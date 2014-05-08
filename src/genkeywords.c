/*
**++
**  MODULE DESCRIPTION:
**
**      Thi module generates the keyword definition header files from the
**  command definition table.
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
**  CREATION DATE:  08-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      08-May-2014 V1.0    Sneddon   Initial version.
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
    static void pass_1_file(char *prefix, char *name);
    static void store(char *word, int len);
    static unsigned hash(char *word, int len);

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
            pass_1_file(argv[1], argv[i]);
        }
    }

    fpout1 = fopen(argv[2], "w");       /* open the h output file */
    fpout2 = fopen(argv[3], "w");       /* open the c output file */

    if (!fpout1 || !fpout2) {
        fprintf(stderr, "%s: can not open output file(s)\n",
                program_invocation_short_name);
        exit(EXIT_FAILURE);
    } else {
        fputs("#ifndef KEYWORDS_H_\n", fpout1);
        fputs("#define KEYWORDS_H_\n", fpout1);
        fputs("#ifndef PREPROCESS_CDT\n", fpout1);
        fputs("    extern const char *keywords[];\n", fpout1);
        fputs("#endif /* PREPROCESS_CDT */\n", fpout1);

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

        fputs("#endif /* KEYWORDS_H_ */\n", fpout1);
    }

    fclose(fpout1);
    fclose(fpout2);

    printf("end of %s\n", program_invocation_short_name);

    exit(EXIT_SUCCESS);
}

static void pass_1_file(char *prefix,
                        char *name) {
    char buf[5555], *status;
    FILE *fpin = fopen(name, "r");      /* ptr to current input file */
    size_t prefix_len = strlen(prefix);

    while ((status = fgets(buf, sizeof(buf), fpin)) != NULL) {
        char *start = buf;
        int len;

        if ((start = strstr(start, prefix)) != 0) {

            start += prefix_len;
            len = strcspn(start, ": \t");
            store(start, len);
            start +=  len + 1;
        }
    }

    fclose(fpin);
}

static void store(char *word,
                  int len) {
    int i = hash(word, len);
    struct itm **itmp = &ht[i];

    while (*itmp != 0) {
        if (strncmp((*itmp)->np, word, len) == 0) return;
        itmp = &(*itmp)->next;
    }

    *itmp = calloc(sizeof(struct itm), 1);
    if (*itmp == 0) raise(SIGSEGV);

    (*itmp)->np = strndup(word, len);
    if ((*itmp)->np == 0) raise(SIGSEGV);
}

unsigned hash(char *str,
              int len) {
    Fnv32_t val;
#define TINY_MASK(x) (((u_int32_t)1<<(x))-1)

    val = fnv_32a_buf(str, len, FNV_32_PRIME);
    return ((val >> 8) ^ val) & TINY_MASK(8);
}

/*
**++
**  MODULE DESCRIPTION:
**
**      This moduel contains useful utility routines.
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
**  CREATION DATE:  08-MAY-2014
**
**  MODIFICATION HISTORY:
**
**      08-MAY-2014 V1.0    Sneddon   Initial coding.
**      09-MAY-2014 V1.1    Sneddon   Fixed spacing in cat.
**      20-MAY-2014 V1.2    Sneddon   Add dump.
**--
*/
#define MODULE PJLCP_UTIL
#define IDENT "V1.2"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#define LINEMAX 16

/*
** Forward declarations
*/

    char *cat(char *old, char *format, ...);
    char *strip(const char *str, const int len);
    void dump(FILE *file, const char *buf, const size_t buflen);

char *cat(char *old,
          char *format,
          ...) {

    va_list ap;
    char *buf, *result;
    int status;

    /*
    ** Build the user supplied bit...
    */
    va_start(ap, format);
    status = vasprintf(&buf, format, ap);
    if (status == -1) raise(SIGSEGV);
    va_end(ap);

    /*
    ** Now, append it...
    */
    status = asprintf(&result, "%s%s%s", (old == 0) ? "" : old,
                      (old == 0) ? "" : " ", buf);
    if (status == -1) raise(SIGSEGV);

    /*
    ** Now, tidy up.
    */
    if (old != 0) free(old);
    free(buf);

    return result;
} /* cat */

char *strip(const char *oldstr,
            const int len) {

    char *newstr;
    int oldlen = len, i, j, quote = 0;

    if (oldlen == -1)
        oldlen = strlen(oldstr);

    newstr = calloc(1, oldlen+1);
    if (newstr != 0) {
        /*
        ** First, drop all trailing spaces.
        */
        for (; oldlen > 0; oldlen--) {
            if (!isspace(oldstr[oldlen-1])) break;
        }

        /*
        ** Skip all the leading space.
        */
        for (i = 0; i < oldlen; i++)
            if (!isspace(oldstr[i])) break;

        /*
        ** Finally, drop unnecessary quotes.
        */
        for (j = 0; i < oldlen; i++) {
            if (oldstr[i] == '"') {
                if (!quote) {
                    quote = 1;
                    continue;
                }
            }
            quote = 0;
            newstr[j++] = oldstr[i];
        }
    }

    return newstr;
} /* strip */

void dump(FILE *file,
          const char *buf,
          const size_t buflen) {
    FILE *out;
    int i, j;

    if (buflen == 0) return;
    out = file == 0 ? stdout : file;

    fprintf(out, ">Record    (%d bytes)\n", (int) buflen);
    for (i = 0; i < buflen; i += LINEMAX) {
        int jmax = i + LINEMAX;

        fputs("  [", out);

        for (j = i; j < jmax; j++) {
            char c = buf[j];

            if (j < buflen) {
                fputc(isprint(c) ? c : '.', out);
            } else {
                fputc(' ', out);
            }
        }

        fprintf(out, "]-%5d-[", i);

        j = i;
        do {
            if ((j % 16) != 0) fputc(' ', out);
            fprintf(out, "%02X", buf[j++]);
        } while ((j < jmax) && (j < buflen));

        fputs("]\n", out);
    }

} /* dump */



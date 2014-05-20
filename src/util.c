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
#include "common.h"

/*
** Forward declarations
*/

    char *cat(char *old, char *format, ...);
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


void dump(FILE *file,
          const char *buf,
          const size_t buflen) {
    FILE *out;
    int i, j;

    if (buflen == 0) return;
    out = file == 0 ? stdout : file;


#if 0
>Record 3   (59 bytes)
  [This is coming t]-    0-[54 68 69 73 20 69 73 20 63 6F 6D 69 6E 67 20 74]
  [o you from the e]-   16-[6F 20 79 6F 75 20 66 72 6F 6D 20 74 68 65 20 65]
  [mailing symbiant]-   32-[6D 61 69 6C 69 6E 67 20 73 79 6D 62 69 61 6E 74]
  [ on SKI....     ]-   48-[20 6F 6E 20 53 4B 49 2E 2E 2E 2E]

#endif

    fprintf(out, ">Record    (%d bytes)\n", buflen);
    for (i = 0; i < buflen; i += j) {
        int jmax = i + 16;

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

        for (j = i; (j < jmax) && (j < buflen); j++) {
            char c = buf[j];

            fprintf(out, "%02X", c);
            if ((j + 1) < jmax) fputc(' ', out);
        }

        fputs("]\n", out);
    }

} /* dump */


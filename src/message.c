/*
**++
**  MODULE DESCRIPTION:
**
**      Message output routines for PJLCP.
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
**      01-MAY-2014 V1.0    Sneddon   Initial coding.
**      08-MAY-2014 V1.1    Sneddon   Added debug().
**      16-MAY-2014 V1.2    Sneddon   Make first character from strerror
**                                    lowercase (looks better).
**--
*/
#define MODULE PJLCP_MESSAGE
#define IDENT "V1.2"
#ifdef __VMS
# pragma module MODULE IDENT
#else
# define _GNU_SOURCE
#endif
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** Forward declarations
*/

    void debug(const char *message, ...);
    void info(const int errnum, const char *message, ...);
    void error(const int errnum, const char *message, ...);
    void warn(const int errnum, const char *message, ...);
    static void output(const int errnum, const char *severity,
                       const char *s, va_list ap);

/*
** Own storage
*/

    int info_count = 0, error_count = 0, warn_count = 0;

void debug(const char *message,
           ...) {

    va_list ap;

    va_start(ap, message);
    output(0, "debug", message, ap);
    va_end(ap);
}

void info(const int errnum,
          const char *message,
          ...) {

    va_list ap;

    info_count++;
    va_start(ap, message);
    output(errnum, "info", message, ap);
    va_end(ap);
}

void error(const int errnum,
           const char *message,
           ...) {

    va_list ap;

    error_count++;
    va_start(ap, message);
    output(errnum, "error", message, ap);
    va_end(ap);
}

void warn(const int errnum,
          const char *message,
          ...) {

    va_list ap;

    warn_count++;
    va_start(ap, message);
    output(errnum, "warn", message, ap);
    va_end(ap);
}

static void output(int errnum,
                   const char *severity,
                   const char *message,
                   va_list ap) {

    const int have_message = ((message != 0) && (message[0] != '\0'));

    /*
    ** Always push out the severity.
    */
    fprintf(stderr, "%s: ", severity);
    if (have_message) {
        /*
        ** Only output the user message, if it was present.
        */
        vfprintf(stderr, message, ap);
    }
    if (errnum != 0) {
        char *errstr = strerror(errnum);

        /*
        ** Also output the error message from the system.
        */
        if (have_message) {
            /*
            ** Make sure that we drop in some punctuation if we had a
            ** user-speicifed message.
            */
            fputs(", ", stderr);
        }
        /*
        ** This might seem odd, but it makes sure that the first character
        ** of the string from strerror starts with a lowercase character, os
        ** that the error message look neater.
        */
        fprintf(stderr, "%c%s", tolower(errstr[0]), &errstr[1]);
    }
    fputc('\n', stderr);
}

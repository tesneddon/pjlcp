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
**      08-MAY-2014 V1.0    Sneddon     Initial coding.
**--
*/
#define MODULE PJLCP_UTIL
#define IDENT "V1.0"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include "common.h"

/*
** Forward declarations
*/

    char *cat(char *old, char *format, ...);

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
    status = asprintf(&result, "%s %s", (old == 0) ? "" : old, buf);
    if (status == -1) raise(SIGSEGV);

    /*
    ** Now, tidy up.
    */
    if (old != 0) free(old);
    free(buf);

    return result;
}

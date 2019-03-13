/**
 * @file    fsutil.c
 * @version 1.0
 *
 * @brief
 * PJL filesystem utility routines
 *
 * @copyright
 * Copyright (c) 2014, Endless Software Solutions.
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author  Tim E. Sneddon <tsneddon@endlesssoftware.com.au>
 * @date    09-MAR-2018
 *
 * @detail
 * This module contains a collection of routines for manipulating
 * filesystem related data (like pathnames) for use in handling
 * PJL filesystem commands.
 */
/*
 * Modifiction History:
 *  09-MAR-2019 V1.0   Sneddon    Initial coding.
 */
#define MODULE PJLCP_FSUTIL
#define IDENT "V1.0"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include "fsutil.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/**
 *
 */
char *fsget_path(const char *inpath,
                 const char *defpath,
                 const unsigned options) {

    char *endptr, *path, *result;
    int depth;

    errno = 0;

    /*
    ** Test to see if path is relative or absolute.  Prepend
    ** defpath if it is realtive.
    **
    ** For a path to be absolute, it must start with "<integer>:\".
    */
    strtol(inpath, &endptr, 10);
    if (errno == 0) {
        if (!((*endptr == ':') && (*(endptr+1) == '\\'))) {
            path = cat(0, "%s%s", defpath, inpath);
        } else {
            path = strdup(inpath);
        }
    } else {
        path = strdup(inpath);
    }
    if (path == 0) return 0;

    /*
    ** Attempt to compact and consolidate the path by removing superfluous
    ** ".\" and "..\".  Also remove extra "\", if so instructed.
    */
    depth = 0;
    result = calloc(strlen(path), 1);
    if (result == 0) {
        free(path);
        path = 0;
    } else {
        char *pp, *rp, *delim = 0;

        for (pp = path, rp = result; *pp != 0; pp++) {
            if (*pp == '\\') {
                depth++;

                if (delim != 0) {
                    if ((pp - delim) == 1) {
                        /*
                        ** We just stepped over '\\', so ignore the extra
                        ** '\', if asked to.
                        */
                        if (options & O_FSCOMPACT)
                            continue;
                    } else if ((pp - delim) == 2) {
                        if (*(pp-1) == '.') {
                            /*
                            ** We have just reached '\.\', so wind the result
                            ** back one character and skip the output.
                            */
                            rp--;
                            continue;
                        }
                    } else if ((pp -delim) == 3) {
                        if ((*(pp-1) == '.') && (*(pp-2) == '.')) {
                            /*
                            ** We have just reached '\..\, so window the result
                            ** back by two, if depth is greater than zero, and
                            ** skip the output.
                            */

                            // this should only happen if we ask! Can;t access certain things otherwise :-)

                            rp -= 2;
                            continue;
                        }
                    }
                }
                delim = pp;
            }
            *rp++ = *pp;
        }
    }

    // check depth in here also...

    /*
    ** Test for file name too long (only if enabled).
    */
    if (options & O_FSLENGTH) {
        if (strlen(path) > (FSPATH_MAX - 1)) {
            errno = ENAMETOOLONG;
            free(path);
            path = 0;
        }
    }

    return path;
} /* fsget_path */

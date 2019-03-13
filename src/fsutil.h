/*
** PATH.H
**
**  Header file containing definitions used by the filename parser.
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
**  29-SEP-2014  Sneddon    Initial coding.
*/
#ifndef PJLCP_FSUTIL_H_
#define PJLCP_FSUTIL_H_

/*
** Filesystem limits
*/
#define FSITEM_MAX     9    /* # path elements                              */
#define FSNAME_MAX   100    /* # chars in a file name                       */
#define FSPATH_MAX   256    /* # chars in a path name including nul         */

/*
** Filesystem processing options.
*/
#define O_FSITEMS   0x01     /* Test for more than 9 items                   */
#define O_FSLENGTH  0x02     /* Test for length greater than 255             */
#define O_FSCOMPACT 0x04     /* Compact multiple "\" characters.             */

/*
** path.c
*/

    char *fsget_path(const char *path, const char *defpath,
                     const unsigned options);

#endif /* PJLCP_FSUTIL_H_ */

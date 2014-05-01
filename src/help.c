/*
**++
**  MODULE DESCRIPTION:
**
**      Routines for outputing help messages for PJL.
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
#define MODULE PJLCP_HELP
#define IDENT "V1.0"
#ifdef __VMS
# pragma module MODULE IDENT
#endif
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int act_help(void *ctx) {

    PCBDEF *pcbp = ctx;

    printf("HELP...\n");

    return ACT_SUCCESS;
} /* act_help */

/*
 * @OSF_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993, 1996 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE in the
 * src directory for the full copyright text.
 */
/*
 * HISTORY
 * $Log: prsdef.h,v $
 * Revision 1.1.7.2  1996/02/18  23:11:48  marty
 * 	Update OSF copyright years
 * 	[1996/02/18  22:36:39  marty]
 *
 * Revision 1.1.7.1  1995/12/08  18:07:33  root
 * 	Submit OSF/DCE 1.2.1
 * 	[1995/12/08  17:23:55  root]
 * 
 * Revision 1.1.5.2  1994/05/12  15:17:18  rhw
 * 	continued code cleanup & merging
 * 	[1994/05/09  18:46:28  rhw]
 * 
 * 	first pass at code cleanup (compiler warnings, function prototypes)
 * 	[1994/04/25  14:10:34  rhw]
 * 
 * Revision 1.1.5.1  1994/04/21  22:05:13  cbrooks
 * 	CR 3231, 8353
 * 	[1994/04/21  20:05:19  cbrooks]
 * 
 * Revision 1.1.2.2  1992/12/30  20:39:40  htf
 * 	Embedding copyright notice
 * 	[1992/12/30  19:08:19  htf]
 * 
 * Revision 1.1  1992/01/19  15:31:54  devrcs
 * 	Initial revision
 * 
 * $EndLog$
 */
/*
*/
/*
 * Library PRS, Module PRSDEF - Prefix file.
 *
 * Copyright (C) 1985 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Library PRS,  Module PRSDEF
 *
 * Prefix definitions for calling PRS functions.
 *
 *
 * Networks & Communications Software Engineering
 *
 *
 * EDIT HISTORY
 *
 * 0.01	24-May-85
 *	Networks and Communications, Tools
 *
 * 0.02	20-Aug-85
 *	Add RET_EOF error code.
 *
 * END EDIT HISTORY
 *
 * << MOD=[PRSDEF] VER=[0.02] COM=[PRS function call prefix file] >>
 */


#ifndef _PRSDEF_H
#define _PRSDEF_H
/*
 * Constants.
 */
#define ACT_SUCCESS  1			/* Action routine succeeded */
#define ACT_ERROR    0			/* Action routine discovered error */
#define ACT_QUIT    -1			/* Action routine had fatal error */
#define ACT_FILE    -2			/* No such action routine */

#define RET_SUCCESS    1		/* Parse completed successfully */
#define RET_ERROR      0		/* Parse found syntax error */
#define RET_EOF       -1		/* PROMPT input returned EOF */
#define RET_QUIT      -2		/* Action routine had fatal error */
#define RET_OPENERROR -3		/* Parse table file open error */
#define RET_IOERROR   -4		/* Parse file I/O error */
#define RET_VERSION   -5		/* Parse file version error */
#define RET_FILE      -6		/* Parse file had illegal contents */
#define RET_SIZE      -7		/* Parse table buffer too small */

/*
 * Structure for defining an action routine array entry.
 */

struct prs ;			/* forward declaration */

struct	action
	{
	int (*adr)(void *); /* Pointer to routine to call */
	char *nam;			/* Pointer to routine name */
	};

/*
 * Structure for calling "pr_command".
 */
struct prs
    {
	char   *tbl;			/* Pointer to parse table buffer */
	unsigned int tbs;		/* Size of parse table buffer */
	unsigned int tbu;		/* Amount of buffer actually used */
	char   *cmd;			/* Pointer to command buffer */
	int	cms;			/* Size of command buffer */
	char   *cur;			/* Pointer to current element in cmd */
	char   *end;			/* Pointer to next char after element */
	char   *nxt;			/* Pointer to next element to check */
	long    num;			/* Value of OCT/DEC/HEX numeric */
	struct  action *act;		/* Pointer to action routine array */
	int	av1;			/* Action routine value 1 */
	int	av2;			/* Action routine value 2 */
	int	av3;			/* Action routine value 3 */
	char *maxcur;			/* the highest address seen */
    };

/*
 * Structure for calling "pf_command".
 */
struct prf
    {
	int     fpt;			/* Index for parse table file */
	char   *cmd;			/* Pointer to command buffer */
	int	cms;			/* Size of command buffer */
	char   *cur;			/* Pointer to current element in cmd */
	char   *end;			/* Pointer to next char after element */
	char   *nxt;			/* Pointer to next element to check */
	long    num;			/* Value of OCT/DEC/HEX numeric */
	struct  action *act;		/* Pointer to action routine array */
	int	av1;			/* Action routine value 1 */
	int	av2;			/* Action routine value 2 */
	int	av3;			/* Action routine value 3 */
    };

#endif /* ifndef _PRSDEF_H */

/*
 * @OSF_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993, 1996 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE in the
 * src directory for the full copyright text.
 */
/*
 * HISTORY
 * $Log: prsdep.h,v $
 * Revision 1.1.4.2  1996/02/18  23:12:03  marty
 * 	Update OSF copyright years
 * 	[1996/02/18  22:36:58  marty]
 *
 * Revision 1.1.4.1  1995/12/08  18:09:04  root
 * 	Submit OSF/DCE 1.2.1
 * 	[1995/12/08  17:24:14  root]
 * 
 * 	Embedding copyright notice
 * 	[1992/12/30  19:09:04  htf]
 * 
 * 	Fix OT 5110 - make sure unix is defined properly, make sure to include pthread.h
 * 	[1992/11/04  15:26:50  ko]
 * 
 * Revision 1.1.2.1  1994/04/21  22:05:24  cbrooks
 * 	CR 8353, 3231
 * 	[1994/04/21  19:38:02  cbrooks]
 * 
 * Revision 1.1  1992/01/19  15:35:20  devrcs
 * 	Initial revision
 * 
 * $EndLog$
 */
/*
*/
/*
 * Program PRS, Module PRSDEP - Prefix file, machine dependent macros
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
 * Program PRS,  Module PRSPRE
 *
 * Prefix definitions, including binary file record and field definitions.
 *
 *
 * Networks & Communications Software Engineering
 *
 * EDIT HISTORY
 *
 * 0.01	29-Apr-85
 *	Networks and Communications, Tools
 *
 * 0.02	20-Aug-85
 *	Remove version numbers from extensions.
 *	Put in system specific binary file I/O controls.
 *
 * 0.03	29-Aug-85
 *	Put in system specific file deletion calls.
 *	Put in a default terminator string.
 *
 * 0.05 18-Feb-94
 *	Factor out platform specific macros as part of code cleanup 
 *      efforts, and move POSIX specific values into separate file
 *
 * END EDIT HISTORY
 *
 * << MOD=[PRSPRE] VER=[0.03] COM=[Header file for parse table routines] >>
 */

#ifndef _PRSDEP_H 
#define _PRSDEP_H 1

/*
 * File access definitions
 */

#define SYSTYPE YES		/* System type is known (UNIX) */

#define FDECL FILE *		/*   standard file pointer */
#define FOPENB fopen		/*   standard fopen call */
#define FINP "r"		/*     read */
#define FOUT "w"		/*     write */
#define FGETCB fgetc		/*   standard fgetc call */
#define FPUTCB fputc		/*   standard fputc call */
#define FCLOSB fclose		/*   standard fclose call */
#define FDELET unlink		/*   standard unlink (delete) call */

/*
 * Parse table conversion definitions
 */

#define INP_EXT	      ".txt"	/* Default extension for source files */
#define TMP_EXT	      ".tmp"	/* Default extension for temporary files */
#define OUT_EXT	      ".bin"	/* Default extension for output files */

#endif /* _PRSDEP_H */

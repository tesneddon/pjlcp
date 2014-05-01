/*
 * @OSF_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993, 1996 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE in the
 * src directory for the full copyright text.
 */
/*
 * HISTORY
 * $Log: prsrea.c,v $
 * Revision 1.1.9.2  1996/02/17  23:34:50  marty
 * 	Update OSF copyright year
 * 	[1996/02/17  22:51:02  marty]
 *
 * Revision 1.1.9.1  1995/12/08  18:09:48  root
 * 	Submit OSF/DCE 1.2.1
 * 
 * 	HP revision /main/HPDCE02/2  1994/09/26  13:53 UTC  pare
 * 	Merge bl18 changes
 * 	[1995/12/08  17:24:26  root]
 * 
 * 	    Loading drop DCE1_0_3b03
 * 	Revision 1.1.7.2  1993/07/08  19:48:59  root
 * 	Initial King Kong branch
 * 	[1993/07/08  19:48:30  root]
 * 
 * Revision 1.1.7.2  1994/08/23  20:32:14  cbrooks
 * 	Code Cleanup
 * 	[1994/08/19  17:52:50  cbrooks]
 * 
 * 	Code Cleanup
 * 	[1994/08/19  17:29:19  cbrooks]
 * 
 * 	HP revision /main/HPDCE02/1  1994/06/30  12:31 UTC  pare
 * 	Merge hpdce02 (DCE 1.1 beta) and HPDCE01 into HPDCE02
 * 
 * Revision 1.1.7.3  1993/09/23  18:06:12  pare
 * 	DCE1_0_3b03 Merge
 * 	[1993/09/23  18:05:36  pare]
 * 
 * Revision 1.1.5.2  1993/03/18  14:20:56  truitt
 * 	Merge Hp_dce1_0_1 with DCE1_0_2
 * 	[1993/03/18  14:19:56  truitt]
 * 
 * Revision 1.1.1.8  1993/03/11  22:32:40  osfrcs
 * 	    Loading drop DCE1_0_2B18
 * 
 * Revision 1.1.3.2  1992/12/21  23:55:31  smythe
 * 	added OSF OT 5110 fix to include pthread calls for thread safety
 * 	[1992/12/19  18:24:31  smythe]
 * 
 * Revision 1.1.7.1  1994/04/21  22:05:47  cbrooks
 * 	CR 3231, 8353
 * 	[1994/04/21  19:52:28  cbrooks]
 * 
 * 	Checkpoint
 * 	[1994/03/28  20:23:47  cbrooks]
 * 
 * Revision 1.1.3.3  1992/12/30  16:35:30  htf
 * 	Embedding copyright notice
 * 	[1992/12/28  22:13:24  htf]
 * 
 * Revision 1.1.3.2  1992/11/04  18:10:10  ko
 * 	fix OT 5110 - make sure unix is defined, make sure to include pthread.h
 * 	[1992/11/04  15:27:38  ko]
 * 
 * Revision 1.1.1.7  1992/09/21  18:54:57  osfrcs
 * 	    Loading drop Dce1_0_1fnl
 * 
 * Revision 1.1  1992/01/19  15:35:21  devrcs
 * 	Initial revision
 * 
 * $EndLog$
 */
/*
 * Library PRS, Module PRSREA - Read a parse table into memory.
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
 * Library PRS,  Module PRSREA
 *
 * Get an entire parse table file into memory.
 *
 *
 * Networks & Communications Software Engineering
 *
 * EDIT HISTORY:
 *
 * 1.00  25-May-85
 *       Networks and Communications
 *
 * 1.01	20-Aug-85
 *	Put in system specific binary file I/O controls.
 *
 * END EDIT HISTORY
 */


/*
 * General declarations.
 */

#include <pthread.h>                 /* include jackets for pthreads */

#include <stdio.h>
#include <prspre.h>
#include <prsdef.h>

/*
 * File control if non-VMS.
 */

static FILE *file_p1;			/* File pointer for non-VMS */


/*
 * Local routine declarations
 */

static int pr_rd1(struct prs *context) ;
int pr_read( struct prs *, char *) ;


/*
 * pr_read ***
 *
 * Get a an entire parse table file into caller's buffer.
 *
 * Inputs:
 *	context - Address of argument structure.
 *
 *		The following entries must be set prior to the call.
 *
 *		tbl -	"char" pointer to parse table buffer.
 *		tbs -	"unsigned int" size of parse table buffer.
 *
 *	file_name - Char pointer to file name string.
 *
 * Outputs:
 *	int	RET_SUCCESS	Parse table file has been read in.
 *		RET_OPENERROR	Parse table file open error.
 *		RET_IOERROR	Parse file I/O error.
 *		RET_VERSION	Parse file version error.
 *		RET_FILE	Parse file had illegal contents.
 *		RET_SIZE	Parse table buffer too small.
 *
 *	Argument structure entries:
 *
 *		The following entries are valid as output.
 *
 *		tbl -	parse table buffer now filled in with contents of file.
 *		tbu -	"unsigned int" amount of buffer actually used.
 *
 *		The following entries are unmodified.
 *
 *		tbl -	"char" pointer to parse table buffer.
 *		tbs -	"unsigned int" size of parse table buffer.
 *		cmd -	"char" pointer to command buffer.
 *		cms -	"int" size of command buffer.
 *		act -	"struct" pointer to action routine array.
 *
 *		All other entries are invalid.
 */

int pr_read(struct prs *context, char *file_name)
{
    int result;

    /*
     * Open the file, for READ, in BINARY mode.
     */

    if ((file_p1 = FOPENB(file_name, FINP)) == (int)NULL)
    {
    	return (RET_OPENERROR);
    }

    /*
     * Read in the file.
     */
    result = pr_rd1(context);

    /*
     * Close the file.
     */
    FCLOSB(file_p1);

    /*
     * Finished.
     */
    return (result);
}

/*
 * pr_rd1 ***
 *
 * Perform the actual file read operation.  This is done in a subroutine
 * in order to save having to close the file multiple places.
 *
 * Inputs:
 *	context - Address of argument structure, from "pr_read".
 *
 * Outputs:
 *	int, same as "pr_read".
 */

static int pr_rd1(struct prs *context)
{
    /*
     * Local data.
     */
    int file_char;			/* Integer character from file */
    register char *file_buffer;		/* Local pointer to buffer */
    unsigned int file_size;		/* File size for calculations */
    int tmp_file_size;			/* File size as returned from the
					   FGETCB function - must be int
					   so we can check for EOF */
    char local_buffer[4];		/* In case buffer much too small */

    /*
     * Must be at least 4 bytes in order to read in the version and size.
     */
    file_buffer = context->tbl;

    if (context->tbs < 4)
    {
    	file_buffer = &local_buffer[0];
    }

    /*
     * Check the version/edit.
     */
    if ((file_char = FGETCB(file_p1)) == EOF)
    {
    	return (RET_IOERROR);
    }

    *file_buffer = file_char;
    if (*file_buffer++ != PRS_VERSION)
    {
	return (RET_VERSION);
    }

    if ((file_char = FGETCB(file_p1)) == EOF)
    {
    	return (RET_IOERROR);
    }

    *file_buffer = file_char;
    if (*file_buffer++ > PRS_EDIT)
    {
	return (RET_VERSION);
    }

    /*
     * Check the file size - save it as an unsigned int for use in the
     * calculations below.
     */
    if ((tmp_file_size = FGETCB(file_p1)) == EOF)
    {
    	return (RET_IOERROR);
    }
    file_size = (unsigned int)tmp_file_size;

    if ((file_char = FGETCB(file_p1)) == EOF)
    {
    	return (RET_IOERROR);
    }

    *file_buffer++ = file_size;
    *file_buffer++ = file_char;

    file_size |= file_char << 8;
    context->tbu = file_size;

    if (context->tbs < file_size)
    {
	return (RET_SIZE);
    }

    /*
     * Read the file into memory.
     */
    file_size -= 4;
    while (file_size-- > 0)
    {
    	if ((file_char = FGETCB(file_p1)) == EOF)
    	{
    	    return (RET_IOERROR);
    	}

    	*file_buffer++ = file_char;
    }

    /*
     * File has been read in.
     */
    return (RET_SUCCESS);
}


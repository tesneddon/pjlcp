/*
 * @OSF_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993, 1996 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE in the
 * src directory for the full copyright text.
 */
/*
 * HISTORY
 * $Log: prscmd.c,v $
 * Revision 1.1.9.2  1996/02/17  23:34:49  marty
 * 	Update OSF copyright year
 * 	[1996/02/17  22:51:00  marty]
 *
 * Revision 1.1.9.1  1995/12/08  18:09:40  root
 * 	Submit OSF/DCE 1.2.1
 * 	[1995/12/08  17:24:24  root]
 * 
 * Revision 1.1.7.2  1994/08/23  20:32:13  cbrooks
 * 	Code Cleanup
 * 	[1994/08/19  17:52:48  cbrooks]
 * 
 * 	Code Cleanup
 * 	[1994/08/19  17:29:17  cbrooks]
 * 
 * Revision 1.1.7.1  1994/04/21  22:05:43  cbrooks
 * 	CR 3231, 8353
 * 	[1994/04/21  19:52:25  cbrooks]
 * 
 * 	Checkpoint
 * 	[1994/03/28  20:23:20  cbrooks]
 * 
 * Revision 1.1.4.3  1993/01/13  21:44:25  jd
 * 	The cma package messes with tty settings and doesn't always
 * 	get a chance to put them back. The program should have its
 * 	own file descriptor in this case so as not to do strange
 * 	things to the calling shell.
 * 	[1993/01/13  20:59:26  jd]
 * 
 * Revision 1.1.4.2  1992/12/30  16:35:26  htf
 * 	Embedding copyright notice
 * 	[1992/12/28  22:12:34  htf]
 * 
 * Revision 1.1.2.4  1992/05/28  19:43:04  root
 * 	Ko Baryiames - Fix OT 3713,3698.  Uninitialized variable causing command
 * 	line interface to exit control program fixed.
 * 	[1992/05/28  19:32:13  root]
 * 
 * Revision 1.1.2.3  1992/05/13  19:40:47  ko
 * 	fix ot 2892: strip leading space away from command lines, allowing user
 * 	to insert spaces ahead of commands.
 * 	[1992/05/13  19:39:55  ko]
 * 
 * Revision 1.1.1.2  1992/05/07  13:10:22  ko
 * 	Check for EOF, use isxxx for EBCDIC portability, ot 2894 & ot 2851
 * 
 * Revision 1.1  1992/01/19  15:35:18  devrcs
 * 	Initial revision
 * 
 * $EndLog$
 */
/*
*/
/*
 * Library PRS, Module PRSCMD - Parse a command using in-memory parse tables.
 *
 * COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1990. ALL RIGHTS RESERVED.
 *
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERRED. 
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
 * AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
 * CORPORATION.
 *
 * DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
 * SOFTWARE ON EQUIPMENT THAT IS NOT SUPPLIED BY DIGITAL.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Library PRS,  Module PRSCMD
 *
 * Parse a user command using the in-memory parse table.
 *
 *
 * Networks & Communications Software Engineering
 *
 * EDIT HISTORY:
 *
 * 1.00	25-May-85
 *	Networks and Communications
 *
 * 1.01	20-Aug-85
 *	Add the RET_EOF error code.
 *	Make sure terminator takes precedence over test string.
 *
 * 1.02	29-Aug-85
 *	No local default terminator set.  This was moved into the parse
 *	table compiler.
 *
 * 1.03	03-Sep-85
 *	Double check the end-of-action-routine-list.  Some C compilers
 *	allow "0" addresses.
 *
 * 1.04	05-Sep-85
 *	Fix the above double check.
 *
 * 1.05	11-Sep-85
 *	Fix bad check for CHAR character counter.
 *
 * 1.06	22-Apr-86
 *	Don't skip CHAR counter test if end of string.
 *
 * 1.07 24-Aug-87 (Arun Sastry)
 *	Added command line recall feature using SMG. Valid only for VMS.
 *
 * 1.08 01-Jun-1988
 *      Removed non-portable routine "dns$bcopy"
 *
 * END EDIT HISTORY
 */


/*
 * General declarations.
 */

#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <prspre.h>
#include <prsdef.h>

#ifndef DEBUG_EVENT
#define DEBUG_EVENT(a,b)    
#endif
#include <string.h>

extern int debug;

/*
 * General data.
 */
static char *record;		/* Pointer to parse table record */

/*
 * Local routine declarations
 */


static int pr_wrk(struct prs * /* argument structure */);

static int pr_ele(struct prs * );

static int pr_cha(struct prs * );

static int pr_wor(struct prs * );

static int pr_str(struct prs * );

static int pr_oct(struct prs * );

static int pr_dec(struct prs * );

static int pr_hex(struct prs * );

static int pr_gos(struct prs * );

static int pr_for(struct prs * );

static int pr_vmatch(struct prs * , int  );

static int pr_ltmatch(char ** , int , int  );

static int pr_cmatch(char ** , char ** , int  , int );

static int pr_wmatch(char ** , char *(*list_address) , int case_match );

static int pr_call(struct prs * , char * , int );

static int pr_exvalue(struct prs * , char * );

static long pr_exlong(char * );

static int pr_exword(char * );

static int pr_exbyte(register char * );

static int pr_upper(register char  );

static char *pr_gets(char * , const char *);

int pr_command(struct prs *	/* Argument structure */);

static int pr_wrk(struct prs *	/* Argument structure */);

static int pr_ele(struct prs * );

static int pr_cha(struct prs * );

static int pr_wor(struct prs * );

static int pr_str(struct prs * );

static int pr_oct(struct prs * );

static int pr_dec(struct prs * );

static int pr_hex(struct prs * );

static int pr_gos(struct prs * );

static int pr_for(struct prs * );

static int pr_vmatch(struct prs * , int  );

static int pr_ltmatch(char ** , int  , int  );

static int pr_cmatch(char ** , char **, int , int );

static int pr_wmatch(char ** , char ** , int);

static int pr_call(struct prs * , char *, int  );

static int pr_exvalue(struct prs * , char * );

static long pr_exlong(char * );

static int pr_exword(char * );

static int pr_exbyte(register char * );

static int pr_upper(register char  );

static char *pr_nonblank(register char * );



/*
 * pr_command ***
 *
 * Parse a command string using parse tables already in memory.
 *
 * Inputs:
 *	context - Address of argument structure.
 *
 *		The following entries must be set prior to the call.
 *
 *		tbl -	"char" pointer to parse table buffer.
 *		cmd -	"char" pointer to command buffer.
 *		cms -	"int" size of command buffer.
 *		act -	"struct" pointer to action routine array.
 *
 * Outputs:
 *	int	RET_SUCCESS	Table indicated "success".
 *		RET_ERROR	Table indicated "error".
 *		RET_QUIT	Action routine indicated fatal error.
 *		RET_FILE	Parse table file had illegal contents.
 *
 *	Argument structure entries:
 *
 *		The following entries are valid as output.
 *
 *		cur -	NULL or "char" pointer to command element in error.
 *
 *		The following entries are unmodified.
 *
 *		tbl -	"char" pointer to parse table buffer.
 *		tbs -	"unsigned int" size of parse table buffer.
 *		tbu -	"unsigned int" Amount of buffer actually used.
 *		cmd -	"char" pointer to command buffer.
 *		cms -	"int" size of command buffer.
 *		act -	"struct" pointer to action routine array.
 *
 *		All other entries are invalid.
 */

int pr_command(struct prs *context /* Argument structure */)
{
    /*
     * Local data.
     */

    int result;

    /*
     * Set up initial data.
     */

    record = (context->tbl) + 4; /* Pointer to parse table record */
    context->cur = pr_nonblank(context->cmd);	/* Pointer to first element to parse */
    context->maxcur = context->cur;	/* set up to find where error is */

    /*
     * Do the parsing work.
     */

    result = pr_wrk(context);
    context->end = context->cur;
    context->nxt = context->cur;
    return (result);
}

/*
 * pr_wrk ***
 *
 * Do the actual parse work.  This may be called directly from the user's
 * initial call, or from a GOSUB.
 *
 * Inputs:
 *	context - Address of argument structure.
 *
 *		The following entries must be set prior to the call.
 *
 *		cms -	"int" size of command buffer.
 *		cur -	"char" pointer to current element in user command.
 *		act -	"struct" pointer to action routine array.
 *
 *	The "record" variable contains the pointer to the current parse table
 *	record.
 *
 * Outputs:
 *	int	RET_SUCCESS	Table indicated "success".
 *		RET_ERROR	Table indicated "error".
 *		RET_QUIT	Action routine indicated fatal error.
 *		RET_FILE	Parse table file had illegal contents.
 *
 *	Argument structure entries:
 *
 *		The following entries are valid as output.
 *
 *		cur -	NULL or "char" pointer to command element in error.
 *
 *		The following entries are unmodified.
 *
 *		tbl -	"char" pointer to parse table buffer.
 *		tbs -	"unsigned int" size of parse table buffer.
 *		tbu -	"unsigned int" Amount of buffer actually used.
 *		cmd -	"char" pointer to command buffer.
 *		cms -	"int" size of command buffer.
 *		act -	"struct" pointer to action routine array.
 *
 *		All other entries are invalid.
 */

static int pr_wrk(struct prs *context) /* Argument structure */
{
    /*
     * Local data.
     */
    int   test_result;		/* Result from ELEMENT or TEST */
    char *next_record;		/* Pointer to next parse table record */

    /*
     * Cycle through the states until the next label indicates "done".
     */

    for(;;)
    {
    	/*
    	 * Set up any common information.
    	 */
    	context->num = 0;

    	/*
    	 * Dispatch depending on the parse table record.
    	 */
    	switch (*record)
    	{

	    case (REC_ELE):	test_result = pr_ele(context);
    				break;

	    case (REC_CHA):	test_result = pr_cha(context);
    				break;

	    case (REC_WOR):	test_result = pr_wor(context);
    				break;

	    case (REC_STR):	test_result = pr_str(context);
    				break;

	    case (REC_OCT):	test_result = pr_oct(context);
    				break;

	    case (REC_DEC):	test_result = pr_dec(context);
    				break;

	    case (REC_HEX):	test_result = pr_hex(context);
    				break;

	    case (REC_GOS):	test_result = pr_gos(context);
    				break;

	    case (REC_FOR):	test_result = pr_for(context);
    				break;

	    case (REC_EOF):	return (RET_ERROR);

    	    default:		return (RET_FILE);
    	}

	if (context->cur > context->maxcur)
	    context->maxcur = context->cur;

    	/*
    	 * If some kind of error, return that to the caller.
    	 */

    	if ( (test_result != RET_SUCCESS) && (test_result != RET_ERROR) )
    	{
    	    return (test_result);
    	}

    	/*
    	 * Determine what to do next.
    	 */

    	next_record = NULL;

    	/*
    	 *if got ELEMENT record or TEST failed, perform ADVANCE TO NEXT RECORD.
    	 */

    	if ( (*record == REC_ELE) || (test_result == RET_ERROR) )
    	{
    	    /*
    	     * Advance the pointer.
    	     */
    	    next_record = record + pr_exbyte(record + XXX_LEN);

    	    /*
    	     * If we advanced to (another) ELEMENT, then all tests failed.
    	     */
    	    if ((*next_record == REC_ELE) || (*next_record == REC_EOF))
    	    {
    		return (RET_ERROR);
    	    }
    	}

    	/*
    	 * If got successful TEST, perform GOTO RECORD.
    	 */
    	else
    	{
    	    /*
    	     * Isolate the GOTO flags
    	     */

    	    test_result = *(record+XXX_FLG) & FLG_GOT;

    	    /*
    	     * Check for GOTO OFFSET.
    	     */

    	    if (test_result == FLG_GOF)
    	    {
    		next_record = context->tbl + pr_exword(record+XXX_GOL);
    	    }

    	    /*
    	     * Check for GOTO NEXT ELEMENT.
    	     */

    	    else if (test_result == FLG_GNE)
    	    {
    		next_record = record;
    		while ((*next_record != REC_ELE) && (*next_record != REC_EOF))
    		{
    		    next_record += pr_exbyte(next_record + XXX_LEN);
    		}
    	    }

    	    /*
    	     * Check for RETURN SUCCESS.
    	     */

    	    else if (test_result == FLG_GSU)
    	    {
    		return (RET_SUCCESS);
    	    }

    	    /*
    	     * Must be RETURN ERROR
    	     */

    	    else
    	    {
    		return (RET_ERROR);
    	    }
    	}

    	/*
    	 * If no table record pointer, then there is a real error.
    	 */
    	if (next_record == NULL)
    	{
    	    return (RET_FILE);
    	}

    	/*
    	 * Reset the real pointer into the table.
    	 */
        record = next_record;
    }
}

/*
 * pr_ele ***
 *
 * Perform ELEMENT record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (go on to next TEST record),
 *	      RET_QUIT (call routine error),
 *	      RET_FILE (no such call routine),
 *	      RET_EOF (prompt returned an EOF).
 *
 *	context->cur = pointer to new element in command string.
 */     

static int pr_ele(struct prs *context )

{
    /*
     * Local data
     */

    int action_result;
    char *get_status;
    char * prompt_string ;

#if 0
    register char *prpnt;
    register int prlen;
#endif

    /*
     * If prompt string defined, and we are at the end of the command buffer,
     * get more input.
     */

    if ( (*(record+ELE_PRO) != 0) && (*context->cur == '\0') )
    {
/*    	printf("%s", record + pr_exbyte(record+ELE_PRO)); */
	prompt_string = record + pr_exbyte(record + ELE_PRO ) ;
	*context->cur++ = ' ';
	*context->cur = '\0';
	get_status = pr_gets(context->cur, prompt_string);
        if (get_status == NULL)
            return(RET_EOF);
#if 0
    	prlen = strlen(context->cmd);
	prpnt = context->cmd + prlen;
	prlen = context->cms - prlen - 1;
#endif

    }

    /*
     * Call any specified action routine.
     */

    action_result = pr_call(context, record, ELE_CAL);

    if ((action_result == ACT_ERROR) || (action_result == ACT_QUIT))
	return (RET_QUIT);

    if (action_result == ACT_FILE)
	return (RET_FILE);

    /*
     * ELEMENT records do nothing else.
     */

    return (RET_SUCCESS);
}

/*
 * pr_cha ***
 *
 * Perform CHARACTER TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 */     

static int pr_cha(struct prs *context )

{
    /*
     * Local data.
     */

    char *work_element;		/* Working pointer to element */
    char *temp_element;		/* Temporary pointer to element */
    char *start_element;	/* True start of matched string */
    char *end_element;		/* True end of matched string */
    char *lead_element;		/* Lead-in string pointer */
    char *match_list;		/* Pointer to match list */
    int  alpha_flag;		/* Logical/Alphabetic match switch */
    int  case_flag;		/* General/Exact case match switch */
    int  count_chars;		/* Count number of characters matched */
    int  action_result;		/* Action routine result */

    /*
     * Check the user's command string against the CHARACTER match list.
     */

    match_list = record + pr_exbyte(record+CHA_TST);
    work_element = context->cur;
    count_chars = 0;

    /*
     * Set pointer to element being matched.
     */

    lead_element = start_element = work_element;

    /*
     * Make sure we have a legal lead-in character.
     */    

    action_result = pr_ltmatch(&work_element, CHA_SLS, 1);

    if (action_result == NO)
    {
	return (RET_ERROR);
    }

    if (action_result == YES)
    {
	start_element = work_element;
    }

    /*
     * Step through each match list.
     */

    alpha_flag = pr_exbyte(record+XXX_SWT);
    case_flag = alpha_flag & SWT_CEL;
    alpha_flag &= SWT_MEL;

    for(;;)
    {
	/*
	 * Make sure not a terminator.
	 */
	temp_element = work_element;
	if (pr_ltmatch(&temp_element, CHA_STS, 2) == YES)
    	{
	    action_result = NO;
    	    break;
    	}

    	/*
    	 * Check the current command character against the current match list.
	 * If failure, indicate not at end of list.
    	 */
    	if (pr_cmatch(&work_element, &match_list, alpha_flag, case_flag) == NO)
    	{
	    action_result = NO;
    	    break;
    	}

    	count_chars++;

    	/*
    	 * If another match list, loop and check next character.
    	 * If not, exit the loop, and indicate end of list.
    	 */
    	if (*match_list++ != CHR_SSP)
    	{
	    action_result = YES;
    	    break;
    	}

    	/*
    	 * If another match list, but no other character to match,
    	 * exit the loop, and indicate NOT at end of list.
    	 */
    	if (*work_element == '\0')
    	{
	    action_result = NO;
    	    break;
    	}
    }

    /*
     * Set pointer to end of matched element.
     */
    end_element = work_element;

    /*
     * Make sure we have a legal terminator.
     */
    if (pr_ltmatch(&work_element, CHA_STS, 2) == NO)
    {
	return (RET_ERROR);
    }

    /*
     * We now have a full match.  If not at end of list, make sure it is 
     * not less than minimum length.
     */
    if (action_result == NO)
    {
	if (count_chars < pr_exbyte(record+CHA_TSL))
	{
    	    return (RET_ERROR);
	}
    }

    /*
     * Set end and next pointers for action routine.
     */
    if ((*(record+XXX_FLG) & FLG_TIN) != 0)	/* Include terminator */
    {
    	context->end = work_element;
    	context->nxt = pr_nonblank(work_element);
    }
    else
    {
    	if ((*(record+XXX_FLG) & FLG_TNS) != 0)	/* Leave term for next test */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(end_element);
    	}

    	else					/* Skip the terminator */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(work_element);
    	}
    }

    /*
     * Set start pointer for action routine (also save old one, in case action
     * routine indicates error).
     */
    work_element = context->cur;

    if ((*(record+XXX_FLG) & FLG_LIN) != 0 )	/* Include lead-in character */
    	context->cur = lead_element;

    else					/* Skip lead-in character */
    	context->cur = start_element;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, CHA_CAL);

    /*
     * If any error, restore old element pointer for next test, and return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	context->cur = work_element;

    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);

    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    context->cur = context->nxt;
    return (RET_SUCCESS);
}

/*
 * pr_wor ***
 *
 * Perform WORD TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 */     

static int pr_wor(struct prs *context )

{
    /*
     * Local data.
     */

    char *work_element;		/* Pointer to user command */
    char *work_save;		/* Pointer to start of last matched element */
    char *start_element;	/* True start of matched string */
    char *end_element;		/* True end of matched string */
    char *lead_element;		/* Lead-in string pointer */
    char *match_word;		/* Pointer to match word */
    int  case_flag;		/* General/Exact case match switch */
    int  action_result;		/* Action routine result */

    /*
     * Check the user's command string against the match word list.
     */
    match_word = record + pr_exbyte(record+WOR_TST);
    work_element = context->cur;

    DEBUG_EVENT (parse, ("compare <%s> to <%s>\n", match_word, work_element));

    /*
     * Set pointer to element being matched.
     */
    lead_element = start_element = work_element;

    /*
     * Make sure we have a legal lead-in character.
     */
    action_result = pr_ltmatch(&work_element, WOR_SLS, 1);

    if (action_result == NO)
    {
      DEBUG_EVENT (parse, ("BAD LEAD-IN\n"));
      return (RET_ERROR);
    }

    if (action_result == YES)
    {
	start_element = work_element;
    }

    /*
     * Step through each word match.
     */
    case_flag = pr_exbyte(record+XXX_SWT) & SWT_CEL;

    for(;;)
    {
    	/*
    	 * Check the current command word against the current match word
    	 */
    	work_save = work_element;
    	action_result = pr_wmatch(&work_element, &match_word, case_flag);

    	/*
    	 * If did not reach end of match word, check the length
    	 */
	if (action_result == 0)
	{
    	    if ((work_element - work_save) < pr_exbyte(record+WOR_TSL))
    	    {
	      DEBUG_EVENT (parse, ("(too short)\n"));
	      return (RET_ERROR);
	    }
    	}

    	/*
    	 * If that was the last match word, then exit the loop and check 
    	 * for the final terminator.
    	 */
    	if (*match_word++ != CHR_SSP)
    	{
    	    break;
    	}

    	/*
    	 * If another match word, check for the default terminator.
    	 */
    	if ((*work_element != ' ') && (*work_element != '\0'))
    	{
	  DEBUG_EVENT (parse, ("BAD TERMINATOR\n"));
	  return (RET_ERROR);
    	}

    	/*
    	 * Reset to match the next command word.
    	 */
    	work_element = pr_nonblank(work_element);
    }

    /*
     * Reached end of match words.
     */
    end_element = work_element;

    /*
     * Make sure we have a legal terminator.
     */
    if (pr_ltmatch(&work_element, WOR_STS, 2) == NO)
    {
      DEBUG_EVENT (parse, ("BAD TERMINATOR\n"));
      return (RET_ERROR);
    }

    /*
     * Set pointers for action routine.
     */
    if ((*(record+XXX_FLG) & FLG_TIN) != 0)	/* Include terminator */
    {
    	context->end = work_element;
    	context->nxt = pr_nonblank(work_element);
    }
    else
    {
    	if ((*(record+XXX_FLG) & FLG_TNS) != 0)	/* Leave term for next test */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(end_element);
    	}

    	else					/* Skip the terminator */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(work_element);
    	}
    }

    /*
     * Set start pointer for action routine (also save old one, in case action
     * routine indicates error).
     */
    work_element = context->cur;

    if ((*(record+XXX_FLG) & FLG_LIN) != 0 )	/* Include lead-in character */
    	context->cur = lead_element;

    else					/* Skip lead-in character */
    	context->cur = start_element;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, WOR_CAL);

    /*
     * If any error, restore old element pointer for next test, and return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	context->cur = work_element;

    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);

    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    context->cur = context->nxt;
    DEBUG_EVENT (parse, ("***Match***\n"));
    return (RET_SUCCESS);
}

/*
 * pr_str ***
 *
 * Perform STRING TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 */     

static int pr_str(struct prs *context )

{
    /*
     * Local data.
     */

    char *work_element;		/* Working pointer to element */
    char *temp_element;		/* Temporary pointer to element */
    char *start_element;	/* True start of matched string */
    char *end_element;		/* True end of matched string */
    char *lead_element;		/* Lead-in string pointer */
    char *match_list;		/* Pointer to match list */
    char *work_list;		/* Working pointer to a match list */
    int  alpha_flag;		/* Logical/Alphabetic match switch */
    int  case_flag;		/* General/Exact case match switch */
    int  count_chars;		/* Count of matched characters */
    int  action_result;		/* Action routine result */

    /*
     * Check the user's command string against the STRING match list.
     */
    match_list = record + pr_exbyte(record+STR_TST);
    work_element = context->cur;
    count_chars = 0;

    /*
     * Set pointer to element being matched.
     */
    lead_element = start_element = work_element;

    /*
     * Make sure we have a legal lead-in character.
     */
    action_result = pr_ltmatch(&work_element, STR_SLS, 1);

    if (action_result == NO)
    {
	return (RET_ERROR);
    }

    if (action_result == YES)
    {
	start_element = work_element;
    }

    /*
     * Step through match lists.
     */
    alpha_flag = pr_exbyte(record+XXX_SWT);
    case_flag = alpha_flag & SWT_CEL;
    alpha_flag &= SWT_MEL;

    for(;;)
    {
    	/*
    	 * Step through multiple command characters, until a mismatch.
    	 */
	for(;;)
    	{
	    /*
	     * Make sure not a terminator.
	     */
	    temp_element = work_element;
	    if (pr_ltmatch(&temp_element, STR_STS, 2) == YES)
		break;
	    temp_element = NULL;

    	    /*
    	     * Check the current command character against the current match
    	     * list.  If mismatch, exit the loop and go on to the next list.
    	     */
    	    work_list = match_list;

    	    if (pr_cmatch(&work_element, &work_list, alpha_flag, case_flag) == NO)
    	    {
    		match_list = work_list;
    		break;
    	    }

    	    count_chars++;

    	    if (*work_element == '\0')
    	    {
    		match_list = work_list;
    		break;
    	    }

    	    /*
    	     * This character matched.  Loop and check next.
    	     */
    	}

    	/*
    	 * If no more match lists, exit the loop.
	 * If found a terminator, exit the loop.
    	 * Else loop and check next character.
    	 */
    	if ((*match_list++ != CHR_SSP) || (*work_element == '\0') || (temp_element != NULL))
    	{
    	    break;
    	}
    }

    /*
     * Set pointer to end of matched element.
     */
    end_element = work_element;

    /*
     * Make sure we have a legal terminator.
     */
    if (pr_ltmatch(&work_element, STR_STS, 2) == NO)
	return (RET_ERROR);

    /*
     * We now have a full match.  Make sure it falls in the length range.
     */
    if (count_chars < pr_exbyte(record+STR_TSL))
    {
    	return (RET_ERROR);
    }

    if (count_chars > pr_exbyte(record+STR_TSM))
    {
    	return (RET_ERROR);
    }

    /*
     * Set end and next pointers for action routine.
     */
    if ((*(record+XXX_FLG) & FLG_TIN) != 0)	/* Include terminator */
    {
    	context->end = work_element;
    	context->nxt = pr_nonblank(work_element);
    }
    else
    {
    	if ((*(record+XXX_FLG) & FLG_TNS) != 0)	/* Leave term for next test */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(end_element);
    	}

    	else					/* Skip the terminator */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(work_element);
    	}
    }

    /*
     * Set start pointer for action routine (also save old one, in case action
     * routine indicates error).
     */
    work_element = context->cur;

    if ((*(record+XXX_FLG) & FLG_LIN) != 0 )	/* Include lead-in character */
    	context->cur = lead_element;

    else					/* Skip lead-in character */
    	context->cur = start_element;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, STR_CAL);

    /*
     * If any error, restore old element pointer for next test, and return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	context->cur = work_element;

    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);

    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    context->cur = context->nxt;
    return (RET_SUCCESS);
}

/*
 * pr_oct ***
 *
 * Perform OCTAL TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 *	context->num = 0.
 */     

static int pr_oct(struct prs *context )

{
    return (pr_vmatch(context, 8));
}

/*
 * pr_dec ***
 *
 * Perform DECIMAL TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 *	context->num = 0.
 */     

static int pr_dec(struct prs *context )

{
    return (pr_vmatch(context, 10));
}

/*
 * pr_hex ***
 *
 * Perform HEXADECIMAL TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 *	context->num = 0.
 */     

static int pr_hex(struct prs *context )

{
    return (pr_vmatch(context, 16));
}

/*
 * pr_gos ***
 *
 * Perform GOSUB TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine or GOSUB nested too far).
 *
 *	context->cur = pointer to next element in command string.
 */     

static int pr_gos(struct prs *context )

{
    /*
     * Local data
     */

    char *work_record;		/* Saved parse table record pointer */
    char *work_element;		/* Pointer to user command */
    char *start_element;	/* True start of matched string */
    char *end_element;		/* True end of matched string */
    char *lead_element;		/* Lead-in string pointer */
    int  action_result;		/* Sub-parse result */

    /*
     * Save the current parse table record.
     */
    work_record = record;

    /*
     * Set pointer to element being matched.
     */
    work_element = lead_element = start_element = context->cur;

    /*
     * Make sure we have a legal lead-in character.
     */
    action_result = pr_ltmatch(&work_element, GOS_SLS, 1);

    if (action_result == NO)
    {
	return (RET_ERROR);
    }

    if (action_result == YES)
    {
	start_element = work_element;
    }

    /*
     * Do the sub-parse.
     */
    record = context->tbl + pr_exword(record+GOS_GSL);
    action_result = pr_wrk(context);

    /*
     * Reached end of sub-parse.
     */
    end_element = context->cur;
    work_element = context->cur;
    context->cur = lead_element;
    record = work_record;

    /*
     * Check for sub-parse error result.
     */
    if (action_result != RET_SUCCESS)
    {
    	return (action_result);
    }

    /*
     * Make sure we have a legal terminator.
     */
    if (pr_ltmatch(&work_element, GOS_STS, 2) == NO)
    {
	return (RET_ERROR);
    }

    /*
     * Set pointers for action routine.
     */
    if ((*(record+XXX_FLG) & FLG_TIN) != 0)	/* Include terminator */
    {
    	context->end = work_element;
    	context->nxt = pr_nonblank(work_element);
    }
    else
    {
    	if ((*(record+XXX_FLG) & FLG_TNS) != 0)	/* Leave term for next test */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(end_element);
    	}

    	else					/* Skip the terminator */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(work_element);
    	}
    }

    /*
     * Set start pointer for action routine (also save old one, in case action
     * routine indicates error).
     */
    work_element = context->cur;

    if ((*(record+XXX_FLG) & FLG_LIN) != 0 )	/* Include lead-in character */
    	context->cur = lead_element;

    else					/* Skip lead-in character */
    	context->cur = start_element;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, GOS_CAL);

    /*
     * If any error, restore old element pointer for next test, and return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	context->cur = work_element;

    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);

    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    context->cur = context->nxt;
    return (RET_SUCCESS);
}

/*
 * pr_for ***
 *
 * Perform FORCE TEST record operations.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 */     

static int pr_for(struct prs *context )

{
    /*
     * Local data
     */
    int action_result;		/* Action routine result */

    /*
     * Set pointers for action routine.
     */
    context->end = context->cur;
    context->nxt = context->cur;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, FOR_CAL);

    /*
     * If any error return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);
    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    return (RET_SUCCESS);
}

/*
 * pr_vmatch ***
 *
 * Perform a value type test (OCT, DEC, HEX).
 *
 * This code is common, since these tests are all extremely similar.  This
 * requires that the offsets for the OCT, DEC, and HEX records be identical.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	base = Value base (8, 10, 16).
 *
 *	context->cur = pointer to current element in command string.
 *
 * Outputs:
 *      "int" RET_SUCCESS (TEST succeeded, go to next ELEMENT),
 *	      RET_ERROR (TEST failed, go on to next TEST record),
 *	      RET_QUIT (call routine indicated quit parse),
 *	      RET_FILE (no such call routine).
 *
 *	context->cur = pointer to next element in command string.
 *	context->num = 0.
 */     

static int pr_vmatch(struct prs *context ,
                      int base )

{
    /*
     * Local data.
     */

    char *work_element;		/* Pointer to user command */
    char *start_element;	/* True start of matched string */
    char *end_element;		/* True end of matched string */
    char *lead_element;		/* Lead-in string pointer */
    int  action_result;		/* Action routine result */

    /*
     * Point at the command element.
     */
    work_element = context->cur;

    /*
     * Set pointer to element being matched.
     */
    lead_element = start_element = work_element;

    /*
     * Make sure we have a legal lead-in character.
     */
    action_result = pr_ltmatch(&work_element, NUM_SLS, 1);

    if (action_result == NO)
    {
	return (RET_ERROR);
    }

    if (action_result == YES)
    {
	start_element = work_element;
    }

    /*
     * Pick up the octal/decimal/hex value
     */
    if (base == 8)
    {
    	while (isdigit(*work_element) && 
	       (*work_element != '8' && *work_element != '9'))
    	{
    	    context->num <<= 3;
    	    context->num  |= *work_element++ & 0x07;
    	}
    }

    if (base == 10)
    {
    	while (isdigit(*work_element))
    	{
    	    context->num *= 10;
    	    context->num += *work_element++ & 0x0F;
    	}
    }

    if (base == 16)
    {
    	while (isxdigit(*work_element))
    	{
    	    context->num <<= 4;
    	    if (isdigit(*work_element))
    	    {
    		context->num |= *work_element++ & 0x0F;
    	    }
    	    else
    	    {
    		context->num |= (pr_upper(*work_element++) & 0x0F) + 9;
    	    }
    	}
    }

    /*
     * Reached end of match string.
     */
    end_element = work_element;

    /*
     * Make sure value exists and is in legal range.
     */
    if (end_element == start_element)              return (RET_ERROR);
    if (context->num < pr_exlong(record+NUM_VL1))  return (RET_ERROR);
    if (context->num > pr_exlong(record+NUM_VM1))  return (RET_ERROR);

    /*
     * Make sure we have a legal terminator.
     */
    if (pr_ltmatch(&work_element, NUM_STS, 2) == NO)
    {
	return (RET_ERROR);
    }

    /*
     * Set pointers for action routine.
     */
    if ((*(record+XXX_FLG) & FLG_TIN) != 0)	/* Include terminator */
    {
    	context->end = work_element;
    	context->nxt = pr_nonblank(work_element);
    }
    else
    {
    	if ((*(record+XXX_FLG) & FLG_TNS) != 0)	/* Leave term for next test */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(end_element);
    	}

    	else					/* Skip the terminator */
    	{
    	    context->end = end_element;
    	    context->nxt = pr_nonblank(work_element);
    	}
    }

    /*
     * Set start pointer for action routine (also save old one, in case action
     * routine indicates error).
     */
    work_element = context->cur;

    if ((*(record+XXX_FLG) & FLG_LIN) != 0 )	/* Include lead-in character */
    	context->cur = lead_element;

    else					/* Skip lead-in character */
    	context->cur = start_element;

    /*
     * Call any specified action routine.
     */
    action_result = pr_call(context, record, NUM_CAL);

    /*
     * If any error, restore old element pointer for next test, and return.
     */
    if (action_result != ACT_SUCCESS)
    {
    	context->cur = work_element;

    	if (action_result == ACT_QUIT)    return (RET_QUIT);
    	if (action_result == ACT_FILE)    return (RET_FILE);

    	return (RET_ERROR);
    }

    /*
     * Successful test
     */
    context->cur = context->nxt;
    return (RET_SUCCESS);
}

/*
 * pr_ltmatch ***
 *
 * Check for a lead-in or terminator character.
 *
 * Inputs:
 *	char_address = Address of "char" pointer to character to check.
 *	rec_offset = "int" offset into parse table record for lead-in or terminator string.
 *	type = "int" type of string, 1=lead-in, 2=terminator.
 *
 *	The "record" variable points to the current parse table record.
 *
 * Outputs:
 *	YES   if lead-in or terminator character present.
 *	NO    if lead-in or terminator character not present.
 *	ERROR if no lead-in or terminator character list to check against.
 *
 *	Pointer to character to check is reset to point to the next char.
 */

static int pr_ltmatch(char *(*char_address) ,
                       int rec_offset ,
                       int type )

{
    /*
     * Local data.
     */
    char *work_list;
    int alpha_flag;
    int case_flag;

    /*
     * See if there is a lead-in or terminator list.
     */
    if (*(record + rec_offset) == 0)
    {
	return (ERROR);
    }

    /*
     * Make sure we have a legal lead-in or terminator.
     */
    work_list = record + pr_exbyte(record+rec_offset);
    alpha_flag = pr_exbyte(record+XXX_SWT);

    if (type == 1)
    {
	case_flag = alpha_flag & SWT_CLE;
	alpha_flag &= SWT_MLE;
    }

    else
    {
	case_flag = alpha_flag & SWT_CTE;
	alpha_flag &= SWT_MTE;
    }

    return (pr_cmatch(char_address, &work_list, alpha_flag, case_flag));
}

/*
 * pr_cmatch ***
 *
 * Check a single character against a single match list.
 *
 * Inputs:
 *	char_address = Address of "char" pointer to character to check.
 *	list_address = Address of "char" pointer to match list.
 *	alpha_match  = Zero if logical match, non-zero if alphabetic.
 *	case_match   = Zero if general match, non-zero if case sensitive.
 *
 *	The "record" variable points to the current parse table record.
 *
 * Outputs:
 *	YES if character matched something in the match list.
 *	NO  if not.
 *
 *	Pointer to character to check is reset to point to the next char.
 *	Pointer to match list is reset to point at the "end-of-list" char.
 */

static int pr_cmatch(char *(*char_address) ,
                      char *(*list_address) ,
                      int alpha_match ,
                      int case_match )

{
    /*
     * Local data.
     */
    register char *user_char;	/* Local pointer to character to check */
    register char *list_char;	/* Local pointer to match list */
    char  got_match;		/* Set to YES if command character is matched */
    int   upper_char;		/* "int" of uppercase command string character */

    /*
     * Set the local pointers
     */
    user_char = *char_address;
    list_char = *list_address;

    /*
     * Indicate current command character not yet matched.  Get command 
     * string character in uppercase.
     */
    got_match = NO;
    upper_char = pr_upper(*user_char);

    /*
     * Step through match list.
     */
    while ( (*list_char != CHR_SSP) && (*list_char != CHR_SEN) )
    {
	/*
    	 * If not alphabetic match, and is an alphabetic, do a "logical" match.
    	 */
    	if ((alpha_match == 0) && isupper(*list_char))
    	{
	    switch (*list_char)
	    {
		/*
		 * Any alphabetic.
		 */
		case 'A':
			if (isupper(upper_char)) got_match = YES;
			break;
		/*
		 * Decimal digit.
		 */
		case 'D':
			if (isdigit(*user_char) ) got_match = YES;
    			break;
		/*
		 * End-of-line (with spaces).
		 */
		case 'E':
    			if (*pr_nonblank(user_char) == '\0') got_match = YES;
    			break;
		/*
		 * Hexadecimal digit.
		 */
		case 'H':
			if (isdigit(*user_char)) got_match = YES;
    			if (isupper(upper_char) && isxdigit(upper_char)) got_match = YES;
    			break;
		/*
		 * Lowercase alphabetic.
		 */
		case 'L':
			if (islower(*user_char)) got_match = YES;
    			break;
		/*
		 * Not white space.
		 */
		case 'N':
			if (isgraph(upper_char)) got_match = YES;
    			break;
		/*
		 * Octal digit.
		 */
		case 'O':
			if (isdigit(*user_char) && 
			    (*user_char != '8'&& *user_char != '9')) got_match = YES;
    			break;
		/*
		 * Punctuation.
		 */
		case 'P': if (ispunct(*user_char)) got_match = YES;
    			break;
		/*
		 * White space.
		 */
		case 'S':
			if ((isspace(*user_char) ||
			     iscntrl(*user_char) )  &&
			     *user_char != '\0')
			    got_match = YES;
    			break;
		/*
		 * Uppercase alphabetic.
		 */
		case 'U':
			if ((isupper(*user_char)))
			    got_match = YES;
    			break;
		/*
		 * Any character but EOL.
		 */
		case 'X':
    			if (*pr_nonblank(user_char) != '\0') got_match = YES;
			break;
		/*
		 * End of logical match.
		 */
    		default:
    			break;
	    }
    	}

	/*
	 * If not a letter, or match letters alphabetically, do an exact match.
	 */
	else
	{
	    /*
    	     * If matching a letter alphabetically, we must worry about the 
	     * case.  The letter in the test string is already in the correct
    	     * case.
    	     *
    	     * If matching a non-letter, this is moot.
	     */
    	    if (case_match != 0)
    	    {
    		if (*user_char == *list_char)
    		    got_match = YES;
    	    }
    	    else
    	    {
    		if (upper_char == pr_exbyte(list_char))
    		    got_match = YES;
    	    }
	}

    	/*
    	 * End of test for this entry in match list.  If got a match, exit
    	 * the loop.
    	 */
    	if (got_match != NO)   break;

    	/*
    	 * No match on that entry in the match list.  Try the next.
    	 */
    	list_char++;
    }

    /*
     * We have an end condition.  Reset the caller's list pointer.
     */
    while ( (*list_char != CHR_SSP) && (*list_char != CHR_SEN) )  list_char++;
    *list_address = list_char;

    /*
     * If not a match, don't update the caller's command string pointer.
     */
    if (got_match == NO)	return (NO);

    /*
     * Match succeeded.  Make sure we return the correct pointer.
     *
     * If standard character, just point at next character (use "upper_char" 
     * to get int version, so we don't have to worry about sign extended
     * characters).
     *
     * If white space, skip until non-white space.
     */
    if (isgraph(upper_char))	user_char++;
    else			user_char = pr_nonblank(user_char);

    *char_address = user_char;
    return (YES);
}

/*
 * pr_wmatch ***
 *
 * Check a single word against a single match word.
 *
 * Inputs:
 *	char_address = Address of "char" pointer to word to check.
 *	list_address = Address of "char" pointer to match word.
 *	case_match   = Zero if general case, non-zero if exact case.
 *
 *	The "record" variable points to the current parse table record.
 *
 * Outputs:
 *	"int" flag, 0 if mismatch, 1 if end of match word.
 *
 *	Pointer to character to check is reset to point to the next char.
 *	Pointer to match word is reset to point at the "end-of-word" char.
 */

static int pr_wmatch(char *(*char_address) ,
                      char *(*list_address) ,
                      int case_match )

{
    /*
     * Local data.
     */

    register char *user_char;	/* Local pointer to character to check */
    register char *list_char;	/* Local pointer to match word */
    int endword;		/* Set to 1 if end of word found */

    /*
     * Set the local pointers
     */
    user_char = *char_address;
    list_char = *list_address;

    /*
     * Step through the word.  Exit the loop only when a mismatch, or we 
     * come to the end of the match word.
     */
    endword = 1;

    while ( (*list_char != CHR_SSP) && (*list_char != CHR_SEN) )
    {
	/*
	 * Check for general case matching.  If so, parse table already
    	 * has uppercase letter.
	 */
	 if (case_match == 0)
	{
	    if (pr_upper(*user_char) != pr_exbyte(list_char))
	    {
		endword = 0;
		break;
	    }
	}

	/*
	 * Check for exact case matching.  If so, parse table already
    	 * has correct case letter.
	 */
	else
	 {
	     if (*user_char != *list_char)
	    {
		endword = 0;
		break;
	    }
	 }

    	user_char++;
	list_char++;
    }

    /*
     * Reset the caller's list pointer.
     */
    while ( (*list_char != CHR_SSP) && (*list_char != CHR_SEN) )  list_char++;
    *list_address = list_char;

    /*
     * Reset caller's command pointer.
     */
    *char_address = user_char;

    /*
     * Now return
     */
    return (endword);
}

/*
 * pr_call ***
 *
 * Call the designated action routine.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	record = "char" pointer to parse table record.
 *	offset = "int" offset into parse table record for CALL data.
 *
 *	The CALL data consists of an offset to a call routine name,
 *	followed by 3 bytes of call routine values.
 *
 * Outputs:
 *      The three bytes are placed in the context block and the routine
 *	is called.
 *
 *	"int" ACT_SUCCESS (action routine succeeded),
 *	      ACT_ERROR (action routine indicated error),
 *	      ACT_QUIT (action routine indicated fatal error),
 *	      ACT_FILE (no such action routine).
 */     

static int pr_call(struct prs *context ,
                    char *record ,
                    int offset )

{
    /*
     * Local data.
     */
    register char *call_name;
    register struct action *action;
    register char *actnam;
    int (*address)(void *);
    char found;
    int status;				/* value returned by action routine */

    /*
     * If no routine defined, then assume it would have succeeded.
     */
    if (*(record+offset) == 0)
    {
    	return (ACT_SUCCESS);
    }

    /*
     * Get pointer to routine name, and get the action routine values.
     */
    call_name = record + pr_exbyte(record+offset);
    pr_exvalue(context, (record+offset+1));

    /*
     * Step through the routine table, and check entries.
     */
    action = context->act;

    for(;;)
    {
	address = action->adr;
	actnam = action->nam;

	if (actnam != NULL)
	{
	    if (*actnam == '\0')  actnam = NULL;
	}
	

	if ( (address == NULL) && (actnam == NULL) )
	{
	    found = NO;
	    break;
	}

	if (strcmp(call_name, actnam) == 0)
	{
	    found = YES;
    	    break;
	}

	action++;
    }

    /*
     * If no such routine name...
     */
    if (found == NO)
    {
    	printf("\n\nParse table error, routine not defined: %s\n\n", call_name);
	return (ACT_FILE);
    }

    /*
     * Call the action routine and check the result.
     */
    status = (*address)(context);	/* call the action routine */
    if (status == ACT_SUCCESS)
	{
	context->cur = context->nxt;	/* adjust pointer for user */
	};
    return (status);			/* back to the caller */
}

/*
 * pr_exvalue ***
 *
 * Extract action routine values from a parse table record.
 *
 * Inputs:
 *	context = Pointer to context block structure.
 *	values  = "char" pointer into parse table record.
 *
 *	The values consist of 3 bytes.
 *
 * Outputs:
 *      The three bytes are placed in the context block.
 */     

static int pr_exvalue(struct prs *context ,
                       char *values )

{
    context->av1 = pr_exbyte(values++);
    context->av2 = pr_exbyte(values++);
    context->av3 = pr_exbyte(values);
    return 0;
}

/*
 * pr_exlong ***
 *
 * Extract a 32-bit value from a parse table record.
 *
 * Inputs:
 *	value = "char" pointer into parse table value.
 *
 *	The value is low-byte to high-byte.
 *
 * Outputs:
 *      The "long int" value.
 */     

static long pr_exlong(char *value )

{
    long result;

    result   = pr_exbyte(value + 3);
    result <<= 8;
    result  |= pr_exbyte(value + 2);
    result <<= 8;
    result  |= pr_exbyte(value + 1);
    result <<= 8;
    result  |= pr_exbyte(value + 0);
   
    return (result);
}

/*
 * pr_exword ***
 *
 * Extract a 16-bit value from a parse table record.
 *
 * Inputs:
 *	value = "char" pointer into parse table record.
 *
 *	The value is low-byte, then high-byte.
 *
 * Outputs:
 *      The "int" value.
 */     

static int pr_exword(char *value )

{
    register int result;

    result  = pr_exbyte(value++);
    result |= pr_exbyte(value) << 8;
   
    return (result);
}

/*
 * pr_exbyte ***
 *
 * Extract an 8-bit value from a parse table record.
 *
 * Inputs:
 *	value = "char" pointer into parse table record.
 *
 * Outputs:
 *      The "int" value.
 */     

static int pr_exbyte(register char *value )

{
    register int result;

    result = *value;
    result &= 0xFF;

    return (result);
}

/*
 * pr_upper ***
 *
 * Convert a character to uppercase
 *
 * Inputs:
 *	character = "char" to convert.
 *
 * Outputs:
 *      "int" character value, in uppercase if was lowercase alphabetic.
 */     

static int pr_upper(register char character )

{
    return ((int) ((islower(character))
		   ? toupper(character)
		   : (int) character));

}

/*
 * pr_nonblank ***
 *
 * Find the next non-blank character.
 *
 * Inputs:
 *	string = "char" pointer to string.
 *
 * Outputs:
 *      "char" pointer to non-blank character in string (control chars
 *	are considered blanks).
 */     

static char *pr_nonblank(register char *string )

{
    while ((isspace(*string) || iscntrl(*string)) && *string != '\0') string++;
    return (string);
}


/*
 * pr_gets ***
 *
 * Get the next input string.  This is a thread-synchronous replacement for
 * gets.
 *
 * Inputs:
 *	string = "char" pointer to string.
 *
 * Outputs:
 *      "char" pointer to non-blank character in string (control chars
 *	are considered blanks).
 */     

char * readline( const char * );
void add_history( const char * );

static char *pr_gets( char * string, const char * prompt ) 
{
    char * cs = 0 , *ns = 0 ;
    register int n = 0 ;

     for(;;) 
     {
	 if((cs = readline(prompt)) == NULL)
	     return NULL;

	 ns = pr_nonblank( cs ) ;
	 
	 /* cs points to the first non-WS char, or the trailing 0 */

	 if (*ns == '\0')		/* just WS - get a new line  */
	 {
	     continue ;
	 }		 
	 
	 if (ns [ (n = strlen(ns) - 1) ] == '\n')
	     ns [ n ] = '\0' ;

	 add_history( ns ) ;
	 strcpy(string, ns) ;
	 break;
     }

    return(string);

}

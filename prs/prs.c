/*
**++
**  MODULE DESCRIPTION:
**
**      This module contains the code that converts a parse table
**  definition file to a PRS binary file.  It was part fo the DCE reference
**  implementation offer by the Open Group .  However, it was originally
**  developer at Network & Communications Software Engineering, Digital
**  Equipment Corporation.
**
**  AUTHOR:
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
**  @OSF_COPYRIGHT@
**  COPYRIGHT NOTICE
**  Copyright (c) 1990, 1991, 1992, 1993, 1994, 1996 Open Software Foundation, Inc.
**  ALL RIGHTS RESERVED (DCE).  See the file named COPYRIGHT.DCE for
**  the full copyright text.
**
**  COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1990-1994. ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
**  SOFTWARE ON EQUIPMENT THAT IS NOT SUPPLIED BY DIGITAL.
**
**  CREATION DATE:  29-APR-1985
**
**  MODIFICATION HISTORY:
**
**      02-MAY-2014  V1.0   Sneddon  Clean up to remove compiler warnings.
**
**  Revision 1.1.6.2  1996/02/18  19:30:15  marty
**   Update OSF copyright years
**   [1996/02/18  18:11:44  marty]
**
**  Revision 1.1.6.1  1995/12/08  15:11:35  root
**   Submit OSF/DCE 1.2.1
**   [1995/12/08  14:41:02  root]
**
**  Revision 1.1.4.3  1994/06/09  18:37:41  devsrc
**   cr10871 - expand copyright
**   [1994/06/09  18:09:43  devsrc]
**
**  Revision 1.1.4.2  1994/03/16  19:41:04  tom
**   Bug 10134 - change isalnum to !isalnum.
**   [1994/03/16  19:33:05  tom]
**
**  Revision 1.1.4.1  1994/03/12  22:00:48  peckham
**   DEC serviceability and i18n drop
**   [1994/03/12  14:06:26  peckham]
**
**  Revision 1.1.2.2  1992/12/30  13:09:03  zeliff
**   Embedding copyright notices
**   [1992/12/29  22:40:58  zeliff]
**
**  Revision 1.1  1992/01/19  15:22:06  devrcs
**   Initial revision
**
**  0.01 29-Apr-85
**   Networks and Communications, Tools
**
**  0.02 15-Aug-85
**   Fix "int" to "long" casting problems.
**
**  0.03 20-Aug-85
**   Put in system specific binary file I/O controls.
**
**  0.04 29-Aug-85
**   Put in system specific file deletion calls.
**   Put in a default terminator set.
**--
*/

/*
 * General declarations.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "prspre.h"

/*
 * Under DECUS C, do not auto-prompt for a command.
 */
#ifdef decus
int $$narg = 1;
#endif

/*
 * Buffers to contain file names.
 */
char ifile_name[PATH_MAX];          /* Input file name buffer */
char tfile_name[PATH_MAX];          /* Temporary file name buffer */
char ofile_name[PATH_MAX];          /* Output file name buffer */

/*
 * File control.  Note the output buffer is oversized (INP_MAX) so that we 
 * don't have to do a length check for every byte we insert.  We only have 
 * to do a final length check before writing out to the file, to make sure
 * we don't write more than OUT_MAX bytes.
 */
FILE *inpfp;                        /* Input file pointer */
FILE *tmpfp;                        /* Temporary file pointer */
FILE *outfp;                        /* Output file pointer */

char  inp_buf[INP_MAX];             /* File input buffer */
char  sav_buf[INP_MAX];             /* Saved input buffer */

long  lin_cnt;                      /* Line number */
long  err_cnt;                      /* Error counter */

char  out_buf[INP_MAX];             /* Output buffer */
int   out_len;                      /* Length of contents */

long  out_total;                    /* Current size of output file */

/*
 * Label definition data.  This allows the GOTO buffer index to be stored,
 * rather than a GOTO string label.
 */
struct
    {				/* Label offset structure */
    	char name[LBL_SIZ+1];	/* Storage for label name */
    	long offset;		/* Offset to element record */
    }
    label[LBL_NUM];		/* Declare enough for max labels  */

/*
 * Internal switches.
 */
char swt_flags;			/* Match and Case switches for subsequent tests */

/*
 * Record contents control.
 */
char def_term[] = DEF_TRM;	/* Default if no terminator set */
int goto_defined = NO;		/* Use to make sure GOTO specified */

/*
 * General error message strings (prevent duplication).
 */

/*
 * Keyword, sub-keyword, and switch keyword definitions.
 */
char key_ele[] = KEY_ELE;
char key_cha[] = KEY_CHA;
char key_wor[] = KEY_WOR;
char key_str[] = KEY_STR;
char key_oct[] = KEY_OCT;
char key_dec[] = KEY_DEC;
char key_hex[] = KEY_HEX;
char key_for[] = KEY_FOR;
char key_gos[] = KEY_GOS;

char sub_pro[] = SUB_PRO;
char sub_min[] = SUB_MIN;
char sub_max[] = SUB_MAX;
char sub_lea[] = SUB_LEA;
char sub_ter[] = SUB_TER;
char sub_cal[] = SUB_CAL;
char sub_got[] = SUB_GOT;
char sub_nex[] = SUB_NEX;
char sub_suc[] = SUB_SUC;
char sub_err[] = SUB_ERR;

char swi_inc[] = SWI_INC;
char swi_ski[] = SWI_SKI;
char swi_kee[] = SWI_KEE;

char int_swi[] = INT_SWI;
char int_ele[] = INT_ELE;
char int_lea[] = INT_LEA;
char int_ter[] = INT_TER;
char int_mat[] = INT_MAT;
char int_cas[] = INT_CAS;
char int_alp[] = INT_ALP;
char int_log[] = INT_LOG;
char int_exa[] = INT_EXA;
char int_gen[] = INT_GEN;

/*
 * local prototypes
 */
static int add_label(long,int);
static int add_routine(int);
static int add_string(int, int);
static int checkfiles(void);
static int chk_goto(void);
static int chk_match(char *, char, int);
static int df_ter(int);
static int dis_err(const char *);
static int do_cha(void);
static int do_dec(void);
static int do_ele(void);
static int do_for(void);
static int do_gos(void);
static int do_hex(void);
static int do_oct(void);
static int do_str(void);
static int do_swi(void);
static int do_wor(void);
static char *fnd_blank(char *);
static char *fnd_nonblank(char *);
static int rea_record(FILE *);
static int sb_cal(int);
static int sb_pro(int);
static int sb_lea(int);
static int sb_ter(int);
static int sb_min(int,int);
static int sb_max(int,int);
static int sb_got(void);
static int sb_nex(void);
static int sb_suc(void);
static int sb_err(void);
static int trn_file(void);
static long trn_number(long,long,char);
static char trn_upper(char);
static int wrt_record(FILE *,int);

/*
 * main ***
 *
 * Main routine.  Get the name and type of the file to convert.
 *
 * Inputs:
 *      argc    - Integer - Number of command line arguments.
 *
 *      argv    - Pointer - Address of array of string pointers.  These strings
 *			    contain the command line arguments.
 *
 *
 * Outputs:
 *      ...
 */     

int
main (
int   argc,
char *argv[])
{
    /*
     * Local data.
     */
    int   exit_flag = NO;   /* Exit after command if YES */
    char *pnt0;             /* Temporary pointer */
    char *pnt1;             /* Temporary pointer */
    char *pnt2;             /* Temporary pointer */

    /*
     * Check to see if there was any command line input.  If there was,
     * move it into the standard user input buffer (do not change "argc",
     * because this is used later as a flag).
     */
    if (argc > 1)
    {
        exit_flag = YES;
        ifile_name[0] = '\0';
        strcat(&ifile_name[0], *(++argv));
    }

    /*
     * Get and execute commands until the user says "hold, enough".
     */
    while (YES)
    {
        /*
         * Get user input.  If there was input on the invocation line,
         * the input buffer is already filled.
         */
	    if (argc <= 1)
        {
            ifile_name[0] = '\0';
            while (ifile_name[0] == '\0')
            {
                printf("File: ");
                if (fgets(&ifile_name[0], PATH_MAX, stdin) == 0)
                {
                    putchar('\n');
                    return EXIT_FAILURE;
                }
            }
        }

        /*
         *  Set up temp and output file names.  Make sure names have extensions.
         */
        pnt0 = &ifile_name[0];
        pnt1 = &tfile_name[0];
        pnt2 = &ofile_name[0];

        while ((*pnt0 != '\0') && (*pnt0 != '.') && (*pnt0 != ';'))
        {
            *pnt1++ = *pnt0;
            *pnt2++ = *pnt0;

            if ((*pnt0 == ']') || (*pnt0 == '>') || (*pnt0 == ':'))
            {
                pnt1 = &tfile_name[0];
            }

            pnt0++;
        }

        if (*pnt0 == ';')
        {
            while (*pnt0 != '\0')
            {
                *(pnt0 + strlen(INP_EXT)) = *pnt0;
                pnt0++;
            }
            pnt0 -= strlen(INP_EXT);
            strcat(pnt0, INP_EXT);
            *(pnt0 + strlen(INP_EXT)) = ';';
        }

        if (*pnt0 == '\0')
        {
            strcat(pnt0, INP_EXT);
        }

        *pnt1 = '\0';
        strcat(pnt1, TMP_EXT);

        *pnt2 = '\0';
        strcat(pnt2, OUT_EXT);

        /*
         * Open the files and start the conversion.
         */
        putchar('\n');
        checkfiles();

        /*
         * Finished with user command.
         */
        if (exit_flag == YES)
        {
            return EXIT_SUCCESS;
        }
    }

    /*
     * End of main routine.
     */
}

/*
 * checkfiles ***
 *
 * Open the files and make sure we can proceed with the conversion.
 *
 * Inputs:
 *      ...
 *
 * Outputs:
 *      ...
 */     

static int
checkfiles (void)
{
    static long longtmp;		/* Used for tempoary storage */

    /*
     * Local data.
     */
    register int   int1;
    register char  chr1;
    long save_total;

    /*
     * Open the input file.  Check for errors.
     */
    if ((inpfp = fopen(&ifile_name[0], "r")) == 0)
    {
	printf("Could not open input file: %s\n\n", &ifile_name[0]);
	return;
    }

    /*
     * Open the temporary file.
     */
    if ((tmpfp = fopen(&tfile_name[0], "w")) == 0)
    {
	fclose(inpfp);
	printf("Could not open temporary file: %s\n\n", &tfile_name[0]);
	return;
    }

    /*
     * Files now open.  Translate the file contents.
     */
    for (int1 = 0 ; int1 < LBL_NUM ; int1++)
    {
    	label[int1].name[0] = '\0';	/* No labels defined yet */
    	label[int1].offset = 0;
    }

    out_total = 4;			/* Size of file, and offset for element */

    lin_cnt = 0;			/* Line number */
    err_cnt = 0;			/* No errors yet */
    swt_flags = 0;			/* All MATCH/CASE switches to default */

    trn_file();				/* Translate the file */

    /*
     * Finished with translation.
     */
    fputc(REC_EOF, tmpfp);
    out_total++;

    fclose(inpfp);
    fclose(tmpfp);

    save_total = out_total;

    /*
     * Tell user finished with pass 1.
     */
    printf("Finished pass 1 with %ld errors.  Starting pass 2.\n\n", err_cnt);

    /*
     * Re-open the temporary file and open the real output file.
     */
    if ((tmpfp = fopen(&tfile_name[0], "r")) == 0)
    {
        unlink(&tfile_name[0]);
        printf("Could not reopen temporary file: %s\n\n", &tfile_name[0]);
        return;
    }

    if ((outfp = fopen(&ofile_name[0], "w")) == 0)
    {
        fclose(tmpfp);
        unlink(&tfile_name[0]);
        printf("Could not open output file: %s\n\n", &ofile_name[0]);
        return;
    }

    /*
     * Insert the version and file size.
     */
    fputc(PRS_VERSION, outfp);
    fputc(PRS_EDIT, outfp);

    chr1 = out_total & 0xFF;
    fputc(chr1, outfp);

    chr1 = (out_total >> 8) & 0xFF;
    fputc(chr1, outfp);

    /*
     * Copy the temporary file into the real output file.  Put in the proper
     * record offsets.  Note: This will cause "out_total" to become invalid.
     */
    while (YES)
    {
	/*
	 * Get record
	 */
    	if (rea_record(tmpfp) == -1)
    	{
    	    fclose(tmpfp);
    	    unlink(&tfile_name[0]);
    	    fclose(outfp);
    	    unlink(&ofile_name[0]);
    	    return (-1);
    	}

    	/*
    	 * If last record, output it and finish.
    	 */
    	if (out_buf[XXX_FNC] == REC_EOF)
    	{
    	    wrt_record(outfp, 1);
    	    break;
    	}

    	/*
    	 * If not element record, convert the goto index.
    	 */
    	if (out_buf[XXX_FNC] != REC_ELE)
    	{
	    if ((out_buf[XXX_FLG] & FLG_GOF) != 0)
	    {
		int1 = (out_buf[XXX_GOL] & 0xFF) | ((out_buf[XXX_GOH] & 0xFF) << 8);

		longtmp = label[int1].offset;

		if (longtmp == 0)
		{
		    printf("%s label %s was not defined.\n",
			&key_ele[0], &label[int1].name[0]);
    		    out_buf[XXX_FLG] &= ~FLG_GOT;
    		    out_buf[XXX_FLG] |=  FLG_GER;
	    	    out_buf[XXX_GOL] = 0;
	    	    out_buf[XXX_GOH] = 0;
		}

		else if ((longtmp >> 16) != 0)
		{
		    printf("%s target %s is out of range.  Parse table is too large.\n",
			&sub_got[0], &label[int1].name[0]);
    		    out_buf[XXX_FLG] &= ~FLG_GOT;
    		    out_buf[XXX_FLG] |=  FLG_GER;
	    	    out_buf[XXX_GOL] = 0;
	    	    out_buf[XXX_GOH] = 0;
		}

		else
		{
	    	    out_buf[XXX_GOL] = longtmp & 0x00FF;
	    	    out_buf[XXX_GOH] = (longtmp >> 8) & 0x00FF;
		}
	    }
    	}

    	/*
    	 * If gosub record, convert the gosub index.
    	 */
    	if (out_buf[XXX_FNC] == REC_GOS)
    	{
    	    int1 = (out_buf[GOS_GSL] & 0xFF) | ((out_buf[GOS_GSH] & 0xFF) << 8);
    	    longtmp = label[int1].offset;

    	    if (longtmp == 0)
    	    {
		printf("%s label %s was not defined.\n",
			&key_ele[0], &label[int1].name[0]);
    	    }

    	    else if ((longtmp >> 16) != 0)
	    {
		printf("%s target %s is out of range.  Parse table is too large.\n",
			&key_gos[0], &label[int1].name[0]);
    	    }

    	    out_buf[GOS_GSL] = longtmp & 0x00FF;
    	    out_buf[GOS_GSH] = (longtmp >> 8) & 0x00FF;
    	}

    	wrt_record(outfp, (out_buf[XXX_LEN] & 0xFF));
    }

    /*
     * All done with file.
     */
    fclose(tmpfp);
    unlink(&tfile_name[0]);

    fclose(outfp);

    printf("Finished pass 2.\n\n");
    printf("The size of the converted data file is %ld bytes.\n\n", save_total);

    /*
     * End of conversion.
     */
    return;
}

/*
 * trn_file ***
 *
 * Translate the contents of the file.
 *
 * Inputs:
 *	None.
 *
 * Outputs:
 *	None.
 */     

static int
trn_file (void)
{
    /*
     * Local data.
     */
    register char *pnt1;
    int flg1;

    /*
     * Loop until no more to read from input file.
     */
    while (YES)
    {
	/*
	 * Get a record from the file.
	 */
	pnt1 = &inp_buf[0];

	while (YES)
	{
	    /*
	     * Read a line
	     */
	    lin_cnt++;
	    if (fgets(pnt1, (INP_MAX-(pnt1-&inp_buf[0])), inpfp) == NULL)
	    {
		if (pnt1 != &inp_buf[0])
		{
	    	    printf("Continuation line missing or too long.  Maximum total length is %d.\n", INP_MAX);
	    	    dis_err(pnt1);
		}

		return;
	    }

	    /*
	     * Make sure it properly ends in NULL.
	     */
	    if (inp_buf[strlen(&inp_buf[0])-1] == '\n')
	    {
		inp_buf[strlen(&inp_buf[0])-1] = '\0';
	    }

	    /*
	     * Check for a continuation line.
	     */
	    pnt1 = &inp_buf[strlen(&inp_buf[0])-1];

	    while ( isspace(*pnt1))
	    {
		pnt1--;
	    }

	    if (*pnt1 != '-')
	    {
		break;
	    }
	}

	/*
	 * Check for legal characters
	 */
	pnt1 = &inp_buf[0] - 1;
	while (*(++pnt1) != '\0')
	{
	    if (isspace(*pnt1))
	    {
		*pnt1 = ' ';
	    }

	    if (iscntrl(*pnt1))
	    {
		printf("Illegal control character in text (octal code = %o).\n", *pnt1);
		dis_err(pnt1);
		*pnt1 = ' ';
	    }
	}

	strcpy(&sav_buf[0], &inp_buf[0]);

	/*
	 * If not a blank line or comment, check for a legal keyword.
    	 *
    	 * Note that, as each portion of the line is parsed, it is
    	 * blanked out.  If any non-blank characters are left, it
    	 * indicates an error occured.
	 */
    	flg1 = -1;

    	/*
    	 * Check for comment or blank line.
    	 */
	pnt1 = fnd_nonblank(&inp_buf[0]);

	if ( (*pnt1 == '!') || (*pnt1 == ';') || (*pnt1 == '*') || 
	     (*pnt1 == '/') || (*pnt1 == '#') )
	{
	    *pnt1 = '\0';
	}

    	if (*pnt1 == '\0')
    	{
    	    flg1 = 0;
    	}

    	/*
    	 * Check for legal keyword.  If got one, blank it out and call
    	 * the processing routine.
    	 */
    	if (flg1 == -1)				/* Check for internal SWITCH */
    	{
	    if ((flg1 = chk_match(&int_swi[0], '\0', 3)) >= 0)
    	    {
	        do_swi();
    	    }
    	}

    	if (flg1 == -1)				/* Check for ELEMENT */
    	{
	    if ((flg1 = chk_match(&key_ele[0], '\0', 3)) >= 0)
    	    {
	        if (do_ele() == -1)
        	    return;
    	    }
    	}

    	if (flg1 == -1)				/* Check for CHARACTER */
    	{
	    if ((flg1 = chk_match(&key_cha[0], '\0', 3)) >= 0)
	        do_cha();
    	}

    	if (flg1 == -1)				/* Check for WORD */
    	{
	    if ((flg1 = chk_match(&key_wor[0], '\0', 3)) >= 0)
	        do_wor();
    	}

    	if (flg1 == -1)				/* Check for STRING */
    	{
	    if ((flg1 = chk_match(&key_str[0], '\0', 3)) >= 0)
	        do_str();
    	}

    	if (flg1 == -1)				/* Check for OCTAL */
    	{
	    if ((flg1 = chk_match(&key_oct[0], '\0', 3)) >= 0)
	        do_oct();
    	}

    	if (flg1 == -1)				/* Check for DECIMAL */
    	{
	    if ((flg1 = chk_match(&key_dec[0], '\0', 3)) >= 0)
	        do_dec();
    	}

    	if (flg1 == -1)				/* Check for HEXADECIMAL */
    	{
	    if ((flg1 = chk_match(&key_hex[0], '\0', 3)) >= 0)
	        do_hex();
    	}

    	if (flg1 == -1)				/* Check for GOSUB */
    	{
	    if ((flg1 = chk_match(&key_gos[0], '\0', 3)) >= 0)
	        do_gos();
    	}

    	if (flg1 == -1)				/* Check for FORCED */
    	{
	    if ((flg1 = chk_match(&key_for[0], '\0', 3)) >= 0)
	        do_for();
    	}

    	/*
    	 * If no match, print an error.
    	 */
    	if (flg1 == -1)
    	{
    	    printf("Unrecognized %s or test type keyword.\n",
		&key_ele[0]);
    	    dis_err(pnt1);
    	}
    }
}

/*
 * do_swi ***
 *
 * Do an internal switch definition line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_swi (void)
{
    /*
     * Local data.
     */
    register char *chpnt;
    int bits;

    bits = 0;

    /*
     * Check for SWITCH TEST.
     */
    if (chk_match(&int_ele[0], '\0', 3) >= 0)
    {
    	/*
    	 * Set up possible bits for TEST switches.
    	 */
    	bits = SWT_MEL | SWT_CEL;
    }

    /*
     * Check for SWITCH LEADING.
     */
    else if (chk_match(&int_lea[0], '\0', 3) >= 0)
    {
    	/*
    	 * Set up possible bits for LEADING switches.
    	 */
    	bits = SWT_MLE | SWT_CLE;
    }

    /*
     * Check for SWITCH TERMINATING.
     */
    else if (chk_match(&int_ter[0], '\0', 3) >= 0)
    {
    	/*
    	 * Set up possible bits for TERMINATING switches.
    	 */
    	bits = SWT_MTE | SWT_CTE;
    }

    /*
     * Did we find one of the above?
     */
    else
    {
    	chpnt = fnd_nonblank(&inp_buf[0]);
    	printf("Unrecognized or duplicated keyword.\n");
    	dis_err(chpnt);
    	return;
    }

    /*
     * Check for MATCH.
     */
    if (chk_match(&int_mat[0], '\0', 3) >= 0)
    {
    	/*
         * Clear out all but MATCH switches
         */
    	bits &= SWT_MEL | SWT_MLE | SWT_MTE;

    	/*
    	 * Check for MATCH LOGICAL.
    	 */
    	if (chk_match(&int_log[0], '\0', 3) >= 0)
    	{
    	    swt_flags &= ~bits;
    	}

    	/*
    	 * Check for MATCH ALPHABETIC.
    	 */
    	else if (chk_match(&int_alp[0], '\0', 3) >= 0)
    	{
    	    swt_flags |= bits;
    	}

    	/*
    	 * Check for error
    	 */
    	else
        {
    	    chpnt = fnd_nonblank(&inp_buf[0]);
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return;
    	}
    }

    /*
     * Check for CASE.
     */
    else if (chk_match(&int_cas[0], '\0', 3) >= 0)
    {
    	/*
         * Clear out all but CASE switches
         */
    	bits &= SWT_CEL | SWT_CLE | SWT_CTE;

    	/*
    	 * Check for CASE GENERAL.
    	 */
    	if (chk_match(&int_gen[0], '\0', 3) >= 0)
    	{
    	    swt_flags &= ~bits;
    	}

    	/*
    	 * Check for CASE EXACT.
    	 */
    	else if (chk_match(&int_exa[0], '\0', 3) >= 0)
    	{
    	    swt_flags |= bits;
    	}

    	/*
    	 * Check for error
    	 */
    	else
        {
    	    chpnt = fnd_nonblank(&inp_buf[0]);
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return;
    	}
    }

    /*
     * Check for error
     */
    else
    {
    	chpnt = fnd_nonblank(&inp_buf[0]);
    	printf("Unrecognized or duplicated keyword.\n");
    	dis_err(chpnt);
    	return;
    }

    return;
}

/*
 * do_ele ***
 *
 * Do an element definition line.
 *
 * Inputs:
 *
 * Outputs:
 *	"int" 1 if record ok or non-fatal error, -1 if fatal error
 */     

static int
do_ele (void)
{
    /*
     * Local data.
     */
    register char *chpnt;
    int f1_cal, f2_cal;
    int f1_pro, f2_pro;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Make sure a label is defined.
     */
    if (*chpnt == '\0')
    {
    	printf("%s label is missing.  Please use * to indicate no label is necessary.\n",
		&key_ele[0]);
	dis_err(chpnt);
    	*(--chpnt) = '*';
    }

    /*
     * If a label is present, add it to the list.
     */
    if ( (*chpnt == '*') && ( (*(chpnt+1) == ' ') || (*(chpnt+1) == '\0') ) )
    {
	*chpnt++ = ' ';
    }

    else
    {
        if (add_label(out_total, 1) == -1)
        {
    	    return (-1);
        }
    }

    /*
     * Start setting up the record.
     */
    out_buf[ELE_FNC] = REC_ELE;
    out_buf[ELE_PRO] = 0;
    out_buf[ELE_CAL] = 0;
    out_buf[ELE_CA1] = 0;
    out_buf[ELE_CA2] = 0;
    out_buf[ELE_CA3] = 0;
    out_len = ELE_XXX;
    
    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_cal = 0;
    f1_pro = 0;

    while (YES)
    {
	f2_cal = 0;
	f2_pro = 0;

        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(ELE_CAL);
        if (f1_pro == 0)  f1_pro = f2_pro = sb_pro(ELE_PRO);

    	chpnt = fnd_nonblank(&inp_buf[0]);

    	if ((f2_cal + f2_pro) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
	    return (1);
    	}

    	if ((f1_cal + f1_pro) == 2)
	{
	    if (*chpnt != '\0')
	    {
    	        printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_cha ***
 *
 * Do a character test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_cha (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_CHA;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags;
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[CHA_TST] = 0;
    out_buf[CHA_TSL] = 255;
    out_buf[CHA_SLS] = 0;
    out_buf[CHA_STS] = 0;
    out_buf[CHA_CAL] = 0;
    out_buf[CHA_CA1] = 0;
    out_buf[CHA_CA2] = 0;
    out_buf[CHA_CA3] = 0;
    out_len = CHA_XXX;

    goto_defined = NO;

    /*
     * Pick up the character list and blank it out.
     */
    add_string(CHA_TST, 3);

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(CHA_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(CHA_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(CHA_TSL, 1);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(CHA_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_cal + f2_got) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_cal + f1_got) == 5)
	{
	    if (*chpnt != '\0')
	    {
    	        printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * NOTE: No default terminator set is used here.  Characters can match
     * regardless of the following character.  The user can still explicitly 
     * specify one.
     */

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_wor ***
 *
 * Do a word test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_wor (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_WOR;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags & ~SWT_MEL;
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[WOR_TST] = 0;
    out_buf[WOR_TSL] = 255;
    out_buf[WOR_SLS] = 0;
    out_buf[WOR_STS] = 0;
    out_buf[WOR_CAL] = 0;
    out_buf[WOR_CA1] = 0;
    out_buf[WOR_CA2] = 0;
    out_buf[WOR_CA3] = 0;
    out_len = WOR_XXX;

    goto_defined = NO;

    /*
     * Pick up the "word" and blank it out.
     */
    add_string(WOR_TST, 4);

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(WOR_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(WOR_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(WOR_TSL, 1);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(WOR_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_cal + f2_got) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_cal + f1_got) == 5)
	{
	    if (*chpnt != '\0')
	    {
    	        printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * If no terminator string, put in the default.
     */
    df_ter(WOR_STS);

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_str ***
 *
 * Do a string test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_str (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_max, f2_max;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_STR;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags;
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[STR_TST] = 0;
    out_buf[STR_TSL] = 1;
    out_buf[STR_TSM] = 255;
    out_buf[STR_SLS] = 0;
    out_buf[STR_STS] = 0;
    out_buf[STR_CAL] = 0;
    out_buf[STR_CA1] = 0;
    out_buf[STR_CA2] = 0;
    out_buf[STR_CA3] = 0;
    out_len = STR_XXX;

    goto_defined = NO;

    /*
     * Pick up the "string" and blank it out.
     */
    add_string(STR_TST, 3);

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_max = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_max = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(STR_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(STR_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(STR_TSL, 1);
        if (f1_max == 0)  f1_max = f2_max = sb_max(STR_TSM, 1);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(STR_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_max + f2_cal + f2_got) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_max + f1_cal + f1_got) == 6)
	{
	    if (*chpnt != '\0')
	    {
    	        printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * NOTE: No default terminator set is used here.  Strings can match
     * regardless of the following character.  The user can still explicitly 
     * specify one.
     */

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_oct ***
 *
 * Do an octal number test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_oct (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_max, f2_max;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_OCT;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags & ~(SWT_MEL | SWT_CEL);
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[NUM_VL1] = 0x00;
    out_buf[NUM_VL2] = 0x00;
    out_buf[NUM_VL3] = 0x00;
    out_buf[NUM_VL4] = 0x00;
    out_buf[NUM_VM1] = 0xFF;
    out_buf[NUM_VM2] = 0xFF;
    out_buf[NUM_VM3] = 0xFF;
    out_buf[NUM_VM4] = 0x7F;
    out_buf[NUM_SLS] = 0;
    out_buf[NUM_STS] = 0;
    out_buf[NUM_CAL] = 0;
    out_buf[NUM_CA1] = 0;
    out_buf[NUM_CA2] = 0;
    out_buf[NUM_CA3] = 0;
    out_len = NUM_XXX;

    goto_defined = NO;

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_max = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_max = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(NUM_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(NUM_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(NUM_VL1, 2);
        if (f1_max == 0)  f1_max = f2_max = sb_max(NUM_VM1, 2);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(NUM_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_max + f2_cal + f2_got) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_max + f1_cal + f1_got) == 6)
	{
	    if (*chpnt != '\0')
	    {
    	        printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * If no terminator string, put in the default.
     */
    df_ter(NUM_STS);

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_dec ***
 *
 * Do a decimal number test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_dec (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_max, f2_max;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_DEC;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags & ~(SWT_MEL | SWT_CEL);
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[NUM_VL1] = 0x00;
    out_buf[NUM_VL2] = 0x00;
    out_buf[NUM_VL3] = 0x00;
    out_buf[NUM_VL4] = 0x00;
    out_buf[NUM_VM1] = 0xFF;
    out_buf[NUM_VM2] = 0xFF;
    out_buf[NUM_VM3] = 0xFF;
    out_buf[NUM_VM4] = 0x7F;
    out_buf[NUM_SLS] = 0;
    out_buf[NUM_STS] = 0;
    out_buf[NUM_CAL] = 0;
    out_buf[NUM_CA1] = 0;
    out_buf[NUM_CA2] = 0;
    out_buf[NUM_CA3] = 0;
    out_len = NUM_XXX;

    goto_defined = NO;

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_max = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_max = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(NUM_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(NUM_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(NUM_VL1, 2);
        if (f1_max == 0)  f1_max = f2_max = sb_max(NUM_VM1, 2);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(NUM_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_max + f2_cal + f2_got) == 0)
    	{
    	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_max + f1_cal + f1_got) == 6)
	{
	    if (*chpnt != '\0')
	    {
		printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * If no terminator string, put in the default.
     */
    df_ter(NUM_STS);

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_hex ***
 *
 * Do a hexadecimal number test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_hex (void)
{						  
    /*
     * Local data.
     */
    char *chpnt;
    int f1_cal, f2_cal;
    int f1_min, f2_min;
    int f1_max, f2_max;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_HEX;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags & ~(SWT_MEL | SWT_CEL);
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[NUM_VL1] = 0x00;
    out_buf[NUM_VL2] = 0x00;
    out_buf[NUM_VL3] = 0x00;
    out_buf[NUM_VL4] = 0x00;
    out_buf[NUM_VM1] = 0xFF;
    out_buf[NUM_VM2] = 0xFF;
    out_buf[NUM_VM3] = 0xFF;
    out_buf[NUM_VM4] = 0x7F;
    out_buf[NUM_SLS] = 0;
    out_buf[NUM_STS] = 0;
    out_buf[NUM_CAL] = 0;
    out_buf[NUM_CA1] = 0;
    out_buf[NUM_CA2] = 0;
    out_buf[NUM_CA3] = 0;
    out_len = NUM_XXX;

    goto_defined = NO;

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_min = 0;
    f1_max = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_min = 0;
	f2_max = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(NUM_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(NUM_STS);
        if (f1_min == 0)  f1_min = f2_min = sb_min(NUM_VL1, 2);
        if (f1_max == 0)  f1_max = f2_max = sb_max(NUM_VM1, 2);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(NUM_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_min + f2_max + f2_cal + f2_got) == 0)
    	{
	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_min + f1_max + f1_cal + f1_got) == 6)
	{
	    if (*chpnt != '\0')
	    {
		printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * If no terminator string, put in the default.
     */
    df_ter(NUM_STS);

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_gos ***
 *
 * Do a gosub test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_gos (void)
{						  
    /*
     * Local data.
     */
    register char *chpnt;
    int f1_cal, f2_cal;
    int f1_lea, f2_lea;
    int f1_ter, f2_ter;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_GOS;
    out_buf[XXX_FLG] = FLG_GER | FLG_LSK | FLG_TSK;
    out_buf[XXX_SWT] = swt_flags & ~(SWT_MEL | SWT_CEL);
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[GOS_GSL] = 0;
    out_buf[GOS_GSH] = 0;
    out_buf[GOS_SLS] = 0;
    out_buf[GOS_STS] = 0;
    out_buf[GOS_CAL] = 0;
    out_buf[GOS_CA1] = 0;
    out_buf[GOS_CA2] = 0;
    out_buf[GOS_CA3] = 0;
    out_len = GOS_XXX;

    goto_defined = NO;

    /*
     * Define the label.
     */
    if (*chpnt == '\0')
    {
    	printf("%s label is missing.\n", &key_gos[0]);
	dis_err(chpnt);
    }

    /*
     * If a label is present, add it to the list.
     */
    else
    {
        f1_got = add_label((long)0, 2);
    	if (f1_got != -1)
    	{
    	    out_buf[GOS_GSL] = f1_got & 0xFF;
    	    out_buf[GOS_GSH] = (f1_got >> 8) & 0xFF;
    	}
    }

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_lea = 0;
    f1_ter = 0;
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_lea = 0;
	f2_ter = 0;
	f2_cal = 0;
	f2_got = 0;

        if (f1_lea == 0)  f1_lea = f2_lea = sb_lea(GOS_SLS);
        if (f1_ter == 0)  f1_ter = f2_ter = sb_ter(GOS_STS);
        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(GOS_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_lea + f2_ter + f2_cal + f2_got) == 0)
    	{
	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_lea + f1_ter + f1_cal + f1_got) == 4)
	{
	    if (*chpnt != '\0')
	    {
		printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * NOTE: No default terminator set is used here.  Otherwise, it might
     * conflict with the last terminator set from the sub-parse.  The user
     * can still explicitly require one.
     */

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * do_for ***
 *
 * Do a forced test line.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
do_for (void)
{						  
    /*
     * Local data.
     */
    register char *chpnt;
    int f1_cal, f2_cal;
    int f1_got, f2_got;

    chpnt = fnd_nonblank(&inp_buf[0]);

    /*
     * Start setting up the record.
     */
    out_buf[XXX_FNC] = REC_FOR;
    out_buf[XXX_FLG] = FLG_GER;
    out_buf[XXX_SWT] = 0;
    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;
    out_buf[FOR_CAL] = 0;
    out_buf[FOR_CA1] = 0;
    out_buf[FOR_CA2] = 0;
    out_buf[FOR_CA3] = 0;
    out_len = FOR_XXX;

    goto_defined = NO;

    /*
     * Check for the possible sub-keys.  Keep looping until we get a fatal
     * error, or all functions indicate "done".
     */
    f1_cal = 0;
    f1_got = 0;

    while (YES)
    {
	f2_cal = 0;
	f2_got = 0;

        if (f1_cal == 0)  f1_cal = f2_cal = sb_cal(FOR_CAL);
        if (f1_got == 0)  f1_got = f2_got = sb_got();
        if (f1_got == 0)  f1_got = f2_got = sb_nex();
        if (f1_got == 0)  f1_got = f2_got = sb_suc();
        if (f1_got == 0)  f1_got = f2_got = sb_err();

    	chpnt = fnd_nonblank(&inp_buf[0]);

	if ((f2_cal + f2_got) == 0)
    	{
	    printf("Unrecognized or duplicated keyword.\n");
    	    dis_err(chpnt);
    	    return (1);
    	}

    	if ((f1_cal + f1_got) == 2)
	{
	    if (*chpnt != '\0')
	    {
		printf("Unrecognized or duplicated keyword.\n");
    	        dis_err(chpnt);
	        return (1);
	    }

	    break;
	}
    }

    /*
     * NOTE: No default terminator set is used here.  FORCE does not take 
     * a terminator set at all.
     */

    /*
     * Make sure there is a GOTO.
     */
    chk_goto();

    /*
     * Set the record length and write the record.
     */
    wrt_record(tmpfp, out_len);
    return (1);
}

/*
 * sb_cal ***
 *
 * Check for a CALL sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for results.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found.
 */     

static int
sb_cal (
int off0)
{
    /*
     * Find possible CALL keyword.
     */
    if (*fnd_nonblank(&inp_buf[0]) == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_cal[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Process the routine name.
     */
    add_routine(off0);

    return (1);
}

/*
 * sb_pro ***
 *
 * Check for a PROMPT sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for results.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_pro (
int off0)
{
    /*
     * Find possible PROMPT keyword.
     */
    if (*fnd_nonblank(&inp_buf[0]) == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_pro[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Put the prompt string in the record.
     */
    add_string(off0, 6);
    return (1);
}

/*
 * sb_lea ***
 *
 * Check for a LEADING sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for string results.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_lea (
int off0)
{
    /*
     * Local data.
     */
    int keyend;
    int flg1;

    /*
     * Find possible LEADING keyword.
     */
    if (*fnd_nonblank(&inp_buf[0]) == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if ((keyend = chk_match(&sub_lea[0], '/', 3)) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Get switch and process that also.
     */
    out_buf[XXX_FLG] &= ~FLG_LEA;
    flg1 = 0;

    if (keyend != '/')
    {
    	out_buf[XXX_FLG] |= FLG_LSK;
    	flg1 = 1;
    }

    if (flg1 == 0)
    {
    	if (chk_match(&swi_ski[0], '\0', 2) >= 0)
    	{
    	    out_buf[XXX_FLG] |= FLG_LSK;
    	    flg1 = 1;
    	}
    }

    if (flg1 == 0)
    {
        if (chk_match(&swi_inc[0], '\0', 2) >= 0)
        {
    	    out_buf[XXX_FLG] |= FLG_LIN;
    	    flg1 = 1;
    	}
    }

    if (flg1 != 1)
    {
    	printf("Illegal switch.  Use %s or %s.\n",
		&swi_ski[0], &swi_inc[0]);
    	dis_err(fnd_nonblank(&inp_buf[0]));
    	return (1);
    }

    /*
     * Process the string.
     */
    add_string(off0, 1);
    return (1);
}

/*
 * sb_ter ***
 *
 * Check for a TERMINATION sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for string results.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_ter (
int off0)
{
    /*
     * Local data.
     */
    int keyend;
    int flg1;

    /*
     * Find possible TERMINATOR keyword.
     */
    if (*fnd_nonblank(&inp_buf[0]) == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if ((keyend = chk_match(&sub_ter[0], '/', 3)) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Get switch and process that also.
     */
    out_buf[XXX_FLG] &= ~FLG_TER;
    flg1 = 0;

    if (keyend != '/')
    {
    	out_buf[XXX_FLG] |= FLG_TSK;
    	flg1 = 1;
    }

    if (flg1 == 0)
    {
    	if (chk_match(&swi_ski[0], '\0', 2) >= 0)
    	{
    	    out_buf[XXX_FLG] |= FLG_TSK;
    	    flg1 = 1;
    	}
    }

    if (flg1 == 0)
    {
        if (chk_match(&swi_kee[0], '\0', 2) >= 0)
        {
    	    out_buf[XXX_FLG] |= FLG_TNS;
    	    flg1 = 1;
    	}
    }

    if (flg1 == 0)
    {
        if (chk_match(&swi_inc[0], '\0', 2) >= 0)
        {
    	    out_buf[XXX_FLG] |= FLG_TIN;
    	    flg1 = 1;
    	}
    }

    if (flg1 != 1)
    {
    	printf("Illegal switch.  Use %s, or %s, or %s.\n",
		&swi_ski[0], &swi_kee[0], &swi_inc[0]);
    	dis_err(fnd_nonblank(&inp_buf[0]));
    	return (1);
    }

    /*
     * Process the string.
     */
    add_string(off0, 2);
    return (1);
}

/*
 * sb_min ***
 *
 * Check for a MINIMUM sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for results.
 *	typ0 = "int" type, 1 = match length, 2 = longword value.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_min (
int off0,
int typ0)
{
    /*
     * Local data.
     */
    long result;
    register char *pnt0;

    /*
     * Find possible MINIMUM keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key, and blank it out.
     */
    if (chk_match(&sub_min[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Process the number, and blank that out too.
     */
    if (typ0 == 1)
    {
        out_buf[off0] = trn_number((long)1, (long)255, '\0') & 0xFF;
    }

    else
    {
        result = trn_number((long)-1, (long)-1, '\0');

    	out_buf[off0++] = result         & 0xFF;
    	out_buf[off0++] = (result >> 8)  & 0xFF;
    	out_buf[off0++] = (result >> 16) & 0xFF;
    	out_buf[off0]   = (result >> 24) & 0xFF;
    }

    return (1);
}

/*
 * sb_max ***
 *
 * Check for a MAXIMUM sub-key.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for results.
 *	typ0 = "int" type, 1 = match length, 2 = longword value.
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_max (
int off0,
int typ0)
{
    /*
     * Local data.
     */
    long  result;
    register char *pnt0;

    /*
     * Find possible MAXIMUM keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_max[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.  Blank it out, process the number, and blank that out too.
     */
    if (typ0 == 1)
    {
        out_buf[off0] = trn_number((long)1, (long)255, '\0') & 0xFF;
    }

    else
    {
        result = trn_number((long)-1, (long)-1, '\0');

    	out_buf[off0++] = result         & 0xFF;
    	out_buf[off0++] = (result >> 8)  & 0xFF;
    	out_buf[off0++] = (result >> 16) & 0xFF;
    	out_buf[off0]   = (result >> 24) & 0xFF;
    }

    return (1);
}

/*
 * sb_got ***
 *
 * Check for a GOTO sub-key.
 *
 * Inputs:
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_got (void)
{
    /*
     * Local data.
     */
    register char *pnt0;
    int lbl1;

    /*
     * Find possible GOTO keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_got[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.
     */
    out_buf[XXX_FLG] &= ~FLG_GOT;
    out_buf[XXX_FLG] |=  FLG_GOF;

    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;

    /*
     * Define the label.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	printf("%s label is missing.\n", &sub_got[0]);
	dis_err(pnt0);
    }

    /*
     * If a label is present, add it to the list.
     */
    else
    {
        lbl1 = add_label((long)0, 2);
    	if (lbl1 != -1)
    	{
    	    out_buf[XXX_GOL] = lbl1 & 0xFF;
    	    out_buf[XXX_GOH] = (lbl1 >> 8) & 0xFF;
    	}
    }

    goto_defined = YES;

    return (1);
}

/*
 * sb_nex ***
 *
 * Check for a NEXT sub-key.
 *
 * Inputs:
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_nex (void)
{
    /*
     * Local data.
     */
    register char *pnt0;

    /*
     * Find possible NEXT keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_nex[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.
     */
    out_buf[XXX_FLG] &= ~FLG_GOT;
    out_buf[XXX_FLG] |=  FLG_GNE;

    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;

    goto_defined = YES;

    return (1);
}

/*
 * sb_suc ***
 *
 * Check for a SUCCESS sub-key.
 *
 * Inputs:
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_suc (void)
{
    /*
     * Local data.
     */
    register char *pnt0;

    /*
     * Find possible SUCCESS keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_suc[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.
     */
    out_buf[XXX_FLG] &= ~FLG_GOT;
    out_buf[XXX_FLG] |=  FLG_GSU;

    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;

    goto_defined = YES;

    return (1);
}

/*
 * sb_err ***
 *
 * Check for an ERROR sub-key.
 *
 * Inputs:
 *
 * Outputs:
 *	"int" 1 if processed or not present.
 *	      0 if not yet found
 */     

static int
sb_err (void)
{
    /*
     * Local data.
     */
    register char *pnt0;

    /*
     * Find possible ERROR keyword.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    if (*pnt0 == '\0')
    {
    	return (1);
    }

    /*
     * See if it is the proper sub-key.
     */
    if (chk_match(&sub_err[0], '\0', 3) == -1)
    {
    	return (0);
    }

    /*
     * Got it.
     */
    out_buf[XXX_FLG] &= ~FLG_GOT;
    out_buf[XXX_FLG] |=  FLG_GER;

    out_buf[XXX_GOL] = 0;
    out_buf[XXX_GOH] = 0;

    goto_defined = YES;

    return (1);
}

/*
 * df_ter ***
 *
 * Default the TERMINATION sub-key, if necessary.
 *
 * Inputs:
 *	off0 = "int" index into out_buf, for string results.
 *
 * Outputs:
 */     

static int
df_ter (
int off0)
{
    /*
     * Local data.
     */
    register char *p1;
    register char *p2;

    /*
     * If terminator already defined, just return.
     */
    if (out_buf[off0] != 0)
    {
	return;
    }

    /*
     * Use TERM/SKIP switch as the default.
     */
    out_buf[XXX_FLG] &= ~FLG_TER;
    out_buf[XXX_FLG] |= FLG_TSK;

    /*
     * Put the default string in the input buffer for the add routine.
     * Do not put a null char at the end.
     */
    p2 = &inp_buf[0];
    p1 = &def_term[0];
    while (*p1 != '\0') *p2++ = *p1++;

    /*
     * Process the string.
     */
    add_string(off0, 2);
    return (1);
}

/*
 * trn_upper ***
 *
 *  Convert lower case to upper case alphabetic characters.
 *
 * Inputs:
 *	cc = Character to check and convert, if necessary.
 *
 * Outputs:
 *	If cc is alphabetic and lower case, converted upper case character,
 *	Else, cc
 */

static char
trn_upper (
register char	cc)
{
    if (islower(cc)) return (toupper(cc));
    return (cc);
}

/*
 * trn_number ***
 *
 *  Convert a number to binary.
 *
 * If number has leading "0x", it is hex.
 * If number has leading "0", it is octal.
 * If neither, it is decimal.
 *
 * Inputs:
 *	min0 = "long" minimum value (-1 if none).
 *	max0 = "long" maximum value (-1 if none).
 *	trm0 = "char" indicating specific legal terminator (NULL if none).
 *
 *	Numeric string starts with first non-blank character in buffer.
 *	Space and null are always legal terminators.
 *
 * Outputs:
 *	"long" binary value.
 *
 *	Numeric characters will be blanked out as they are processed.
 *	Convertion stops with first non-numeric character.
 */

static long
trn_number (
long min0,
long max0,
char trm0)
{
    register char *cp;
    char *ep;
    long result;
    int base;

    cp = fnd_nonblank(&inp_buf[0]);
    ep = cp;

    /*
     * Determine proper base for number.
     */
    if (cp[0] == '0')
	if ((cp[1] == 'x') || (cp[1] == 'X'))
	    base = 16;
	else
	    base = 8;
    else
	base = 10;

    /*
     * Translate each character.
     */
    result = strtol(cp, &ep, 0);

    while (cp < ep)
    {
	*cp++ = ' ';
    }

    /*
     * Check the terminator.
     */
    if ((*cp != ' ') && (*cp != '\0') && (*cp != trm0))
    {
    	printf("Illegal character after the number.\n");
    	dis_err(cp);
    }

    /*
     * Check the range
     */
    if ((min0 != -1) && (result < min0))
    {
    	result = min0;

    	if (base == 8)
    	{
    	    printf("Value too low.  Minimum value is 0%lo (octal).\n", min0);
    	}

    	if (base == 10)
    	{
    	    printf("Value too low.  Minimum value is %ld (decimal).\n", min0);
    	}

    	if (base == 16)
    	{
    	    printf("Value too low.  Minimum value is 0x%lx (hex).\n", min0);
    	}

    	dis_err(ep);
    }

    if ((max0 != -1) && (result > max0))
    {
    	result = max0;

    	if (base == 8)
    	{
    	    printf("Value too high.  Maximum value is 0%lo (octal).\n", max0);
    	}

    	if (base == 10)
    	{
    	    printf("Value too high.  Maximum value is %ld (decimal).\n", max0);
    	}

    	if (base == 16)
    	{
    	    printf("Value too high.  Maximum value is 0x%lx (hex).\n", max0);
    	}

    	dis_err(ep);
    }

    /*
     * Finished
     */
    return (result);
}

/*
 * fnd_blank ***
 *
 *  Find next blank.
 *
 * Inputs:
 *	cp = Character pointer.
 *
 * Outputs:
 *	Character pointer to next blank character.
 */

static char *
fnd_blank (
register char *cp)
{
    while ((*cp != ' ') && (*cp != '\0'))   cp++;
    return (cp);
}

/*
 * fnd_nonblank ***
 *
 *  Find next non-blank.
 *
 * Inputs:
 *	cp = Character pointer.
 *
 * Outputs:
 *	Character pointer to next non-blank character.
 */

static char *
fnd_nonblank (
register char *cp)
{
    while ((*cp == ' ') && (*cp != '\0'))   cp++;
    return (cp);
}

/*
 * add_label ***
 *
 *  Add a label.
 *
 * Inputs:
 *	lo = "long" offset into file to use for label's location.
 *	lt = "int" label type, 1 = ELEMENT, 2 = GOTO or GOSUB.
 *
 *	For ELEMENT labels, the offset should be the current location in
 *	the output file.
 *
 * 	For GOTO and GOSUB labels, the offset is ignored.
 *
 * Outputs:
 *	"int" label structure index if added or already existed.
 *	      -1 if fatal error.
 *
 *	Label in calling string will be blanked out.
 */

static int
add_label (
long lo,
int  lt)
{
    /*
     * Local data
     */
    register char *lp;
    char *le;
    char *ep;
    register int lbl1;
    register int lbl2;

    /*
     * Find start and end of label.
     */
    lp = fnd_nonblank(&inp_buf[0]);
    le = fnd_blank(lp);
    ep = lp;

    /*
     * Make sure a label is defined.
     */
    if (*lp == '\0')
    {
    	printf("Label is missing.\n");
	dis_err(ep);
    	return (0);
    }

    /*
     * Check the length.
     */
    if ((le - lp) > LBL_SIZ)
    {
    	printf("Label is too long.  Maximum length is %d characters.\n", LBL_SIZ);
	dis_err(ep);

    	while ((le - lp) > LBL_SIZ)
    	{
    	    *(--le) = ' ';
    	}
    }

    /*
     * See if it already in the table, and find an empty slot.
     */
    lbl2 = -1;

    for (lbl1 = 0 ; lbl1 < LBL_NUM ; lbl1++)
    {
    	if (label[lbl1].name[0] == '\0')
    	{
    	    lbl2 = lbl1;
    	}
 
    	else
    	{
    	    if ((le - lp) == strlen(&label[lbl1].name[0]))
    	    {
    		if (chk_match(&label[lbl1].name[0], '\0', 0) >= 0)
    		{
    		    lbl2 = lbl1;
    		    break;
    		}
    	    }
        }
    }

    /*
     * If label not found, and no empty slots, its a fatal error.
     */
    if (lbl2 == -1)
    {
    	printf("Too many labels defined.  Maximum number is %d labels.\n", LBL_NUM);
    	dis_err(ep);
    	while (lp != le)  *lp++ = ' ';
    	return (-1);
    }


    /*
     * "lbl2" points at the structure to use.  Check the type.
     * If we are adding a new ELEMENT label, it must be undefined.
     */
    if (lt == 1)
    {
    	if (label[lbl2].offset != 0)
        {
    	    printf("Duplicate label defined.\n");
    	    dis_err(ep);
    	}

        label[lbl2].offset = lo;
    }

    /*
     * Copy the label name, and make it a normal string.
     */
    if (label[lbl2].name[0] == '\0')
    {
    	ep = &label[lbl2].name[0];
    	while (lp != le)
    	{
    	    *ep++ = trn_upper(*lp);
    	    *lp++ = ' ';
    	}

    	*ep = '\0';
    }

    return (lbl2);
}

/*
 * add_routine ***
 *
 * Add a routine name and call values to the record in out_buf.
 *
 * Inputs:
 *	off0 = "int" index in out_buf for CALL routine information.
 *
 *	The routine name is the first non-blank in the input buffer.
 *	"out_len" indicates the current length of the record in out_buf.
 *
 * Outputs:
 *	out_len indicates new length of record in out_buf.
 */     

static int
add_routine (
int off0)
{
    register char *pnt1;

    pnt1 = fnd_nonblank(&inp_buf[0]);

    add_string(off0, 5);

    out_buf[off0+1] = 0;
    out_buf[off0+2] = 0;
    out_buf[off0+3] = 0;

    if (*(pnt1 = fnd_nonblank(&inp_buf[0])) == ':')
    {
    	*pnt1 = ' ';
    	out_buf[off0+1] = trn_number((long)0, (long)255, ':') & 0xFF;
    }

    if (*(pnt1 = fnd_nonblank(&inp_buf[0])) == ':')
    {
    	*pnt1 = ' ';
    	out_buf[off0+2] = trn_number((long)0, (long)255, ':') & 0xFF;
    }

    if (*(pnt1 = fnd_nonblank(&inp_buf[0])) == ':')
    {
    	*pnt1 = ' ';
    	out_buf[off0+3] = trn_number((long)0, (long)255, '\0') & 0xFF;
    }

    return;
}

/*
 * add_string ***
 *
 * Add a string to the record in out_buf.
 *
 * Inputs:
 *	off0 = "int" index in out_buf for string's offset in the record.
 *	typ0 = "int" type:
 *			1 = single LEAD string
 *			2 = single TERM string
 *			3 = compound CHAR/STRING test string
 *			4 = compound WORD test string
 *			5 = routine name
 *			6 = prompt string
 *
 *	The string is the first non-blank in the input buffer.
 *	"out_len" indicates the current length of the record in out_buf.
 *
 * Outputs:
 *	The string in the calling buffer is blanked out.
 *	out_len indicates new length of record in out_buf.
 */     

static int
add_string (
int off0,
int typ0)
{
    register char *pnt0;
    register char *pnt1;
    register char *pnt2;
    int upcase;
    int cnt1;

    /*
     * Point at string.
     */
    pnt0 = fnd_nonblank(&inp_buf[0]);

    /*
     * Point at place in buffer to get string.
     */
    out_buf[off0] = out_len;
    pnt1 = &out_buf[out_len];

    /*
     * Determine whether to make letters uppercase or not.
     *
     * For LEAD/TERM/CHAR/STRI type strings, make letters uppercase if
     * a letter match is "logical" (i.e. not "alphabetic" ), or if alphabetic
     * matching is not case sensitive.
     *
     * For WORD type strings, make letters uppercase if matching is not case
     * sensitive.
     */
    upcase = NO;

    if ( (typ0 == 1) && ( ((out_buf[XXX_SWT] & SWT_MLE) == 0) || ((out_buf[XXX_SWT] & SWT_CLE) == 0) ) )
        upcase = YES;

    if ( (typ0 == 2) && ( ((out_buf[XXX_SWT] & SWT_MTE) == 0) || ((out_buf[XXX_SWT] & SWT_CTE) == 0) ) )
        upcase = YES;

    if ( (typ0 == 3) && ( ((out_buf[XXX_SWT] & SWT_MEL) == 0) || ((out_buf[XXX_SWT] & SWT_CEL) == 0) ) )
        upcase = YES;

    if ( (typ0 == 4) && ((out_buf[XXX_SWT] & SWT_CEL) == 0) )
        upcase = YES;

    /*
     * Determine the type of string.
     */
    switch (typ0)
    {
    	/*
    	 * Do a lead-in/termination type string.  This is a single string,
    	 * and letters are matched alphabetically or logically.  If
    	 * alphabetically,they may be matched upper/lower case.
    	 */
    	case 1: 
    	case 2: cnt1 = out_len;

    		while ((*pnt0 != ' ') && (*pnt0 != '\0'))
    		{
    		    if (upcase != NO)
    			*pnt1++ = trn_upper(*pnt0);
    		    else
    			*pnt1++ = *pnt0;

    		    out_len++;
		    *pnt0++ = ' ';
    		}

    		if (out_len == cnt1)
    		{
    		    printf("Missing character string.\n");
    		    dis_err(pnt0);
    		}

    		*pnt1++ = CHR_SEN;
    		out_len++;
    		return;

    	/*
    	 * Do a WORD or CHARACTER/STRING test type string.  This is a compound 
    	 * string, and letters are matched alphabetically or logically.  If
    	 * alphabetically,they may be matched upper/lower case.
    	 *
    	 * Note that for WORD type tests, letters are ALWAYS matched 
    	 * alphabetically.
    	 */
    	case 3:
    	case 4:	while (YES)
    		{
    		    cnt1 = out_len;

    		    while ((*pnt0 != ' ') && (*pnt0 != '\0'))
    		    {
    			if (upcase != NO)
    			    *pnt1++ = trn_upper(*pnt0);
    			else
    			    *pnt1++ = *pnt0;

    			out_len++;
			*pnt0++ = ' ';
    		    }

    		    if (out_len == cnt1)
    		    {
    			printf("Missing word or character string to test against.\n");
    			dis_err(pnt0);
    		    }

    		    pnt2 = pnt0;
    		    while (*pnt2 == ' ')  pnt2++;
    		    if (*pnt2++ != '+')   break;
    		    if (*pnt2++ != ' ')   break;
    		    while (*pnt2 == ' ')  pnt2++;

    		    while (pnt0 != pnt2) *pnt0++ = ' ';

    		    *pnt1++ = CHR_SSP;
    		    out_len++;
    		}

    		*pnt1++ = CHR_SEN;
    		out_len++;
    		return;

    	/*
    	 * Do a routine name type string.  This is a single string, and
    	 * letters are ALWAYS used alphabetically and in the exact case.
    	 */
    	case 5:	cnt1 = out_len;

    		while ((*pnt0 != ' ') && (*pnt0 != '\0') && (*pnt0 != ':'))
    		{
    		    *pnt1++ = *pnt0;
    		    *pnt0++ = ' ';
    		    out_len++;
    		}

    		if (out_len == cnt1)
    		{
    		    printf("Missing routine name.\n");
    		    dis_err(pnt0);
    		}

    		*pnt1++ = CHR_SEN;
    		out_len++;
    		return;

    	/*
    	 * Do a user prompt type string.  This is a single string, and
    	 * letters are ALWAYS used alphabetically and in the exact case.
    	 */
    	case 6:	if (! isalnum(trn_upper(*pnt0)) )
    		{
    		    pnt2 = pnt0++;
    		}

    		else
    		{
    		    printf("Missing prompt string delimiter (use non-alphanumeric).\n");
    		    dis_err(pnt0);
    		    pnt2 = pnt0 - 1;
    		}

    		cnt1 = out_len;

    		while ((*pnt0 != *pnt2) && (*pnt0 != '\0'))
    		{
    		    *pnt1++ = *pnt0;
    		    *pnt0++ = ' ';
    		    out_len++;
    		}

    		if (out_len == cnt1)
    		{
    		    printf("Missing prompt string.\n");
    		    dis_err(pnt0);
    		}

    		if (*pnt0 != '\0')  *pnt0 = ' ';
    		*pnt2 = ' ';

    		*pnt1++ = CHR_SEN;
    		out_len++;
    		return;
    }
}

/*
 * chk_match ***
 *
 * Check to see if a keyword string matches.
 *
 * Inputs:
 *	key0 = "char" pointer to keyword to check against.
 *	trm0 = "char" additional single legal terminator character.
 *	len0 = "int" minimum match length (0 means full match required).
 *
 *	The additional terminator can contain any specific character (NULL 
 *	indicates none).  The SPACE and NULL characters are always considered 
 *	legal terminators.
 *
 *	The string to check is the first non-blank in the input buffer.
 *
 * Outputs:
 *	"int" version of terminating character if success.
 *	"int" -1 if no match.
 *
 *	If match, keyword in string is blanked out.
 */     

static int
chk_match (
register char *key0,
char trm0,
int  len0)
{
    char *sav1 = key0;
    register char *str0 = fnd_nonblank(&inp_buf[0]);
    char *sav2 = str0;

    /*
     * Check for matching characters
     */
    while ((trn_upper(*key0) == trn_upper(*str0)) && (*key0 != '\0'))
    {
    	key0++;
    	str0++;
    }

    /*
     * Make sure at end of key, or, if not, that enough characters matched.
     */
    if (*key0 != '\0')
    {
    	if ( ((key0 - sav1) < len0) || (len0 == 0) )
    	{
    	    return (-1);
    	}
    }

    /*
     * Check for end of user's string
     */
    if ( isalnum(trn_upper(*str0)) )
    {
    	return (-1);
    }

    /*
     * Got some kind of match.  Blank out the string.
     */
    while (sav2 != str0)  *sav2++ = ' ';

    /*
     * Check for legal terminator
     */
    if ( (*str0 != ' ') && (*str0 != '\0') && (*str0 != trm0) )
    {
    	printf("Bad keyword or label termination character.\n");
    	dis_err(str0);

        while ( (*str0 != ' ') && (*str0 != '\0') && (*str0 != trm0) )
	    *str0++ = ' ';
    }

    /*
     * Full match
     */
    return (*str0 & 0xFF);
}

/*
 * chk_goto ***
 *
 * Make sure a record has a GOTO on it.
 *
 * Inputs:
 *
 * Outputs:
 */     

static int
chk_goto (void)
{
    /*
     * Make sure there is a GOTO.
     */
    if (goto_defined == NO)
    {
    	printf("Missing %s, %s, %s, or %s keyword.\n",
		&sub_got[0], &sub_nex[0], &sub_suc[0], &sub_err[0]);
    	dis_err(&inp_buf[0]);
    }
}

/*
 * rea_record ***
 *
 * Read a binary record into "out_buf".
 *
 * Inputs:
 *	fil0 = file pointer.
 *
 * Outputs:
 *	"int" 1 if no error, 0 if last record in file, -1 if premature EOF.
 *	If premature EOF, error has already been printed.
 */     

static int
rea_record (
FILE *fil0)
{
    register char *pnt1 = &out_buf[0];	/* Get pointr to buffer */
    register int cnt1;

    /*
     * Get record type.  If EOF or not legal record type, then error.
     */
    *pnt1 = cnt1 = fgetc(fil0);
    if (cnt1 == EOF)
    {
    	printf("Internal error.  Premature end-of-file encountered.\n\n");
    	return (-1);
    }

    if ((cnt1 < (REC_ELE & 0xFF)) || (cnt1 > (REC_EOF & 0xFF)))
    {
    	printf("Internal error.  Missing record lead-in character.\n\n");
    	return (-1);
    }

    /*
     * If This is the last record in the file, then we say that.
     */
    if (*pnt1++ == REC_EOF)
    {
    	return (0);
    }

    /*
     * Get the record length.
     */
    *pnt1++ = cnt1 = fgetc(fil0);
    if (cnt1 == EOF)
    {
    	printf("Internal error.  Premature end-of-file encountered.\n\n");
    	return (-1);
    }

    cnt1 -= 2;

    /*
     * Read in that many characters.  Just trust things at this point.
     */
    while (cnt1-- > 0)
    {
	int data = fgetc(fil0);

    	*pnt1++ = data;
    	if (data == EOF)
    	{
    	    printf("Internal error.  Premature end-of-file encountered.\n\n");
    	    return (-1);
    	}
    }

    /*
     * No errors.
     */
    return (1);
}

/*
 * wrt_record ***
 *
 * Write a binary record from "out_buf".
 *
 * Inputs:
 *	fil0 = file pointer.
 *	len0 = "int" length of the record.
 *
 * Outputs:
 *	"int" 1 if no error, 0 if putting last record, -1 if error.
 *	If error, message has already been printed.
 */     

static int
wrt_record (
FILE *fil0,
int len0)
{
    register char *pnt1 = out_buf;
    register int cnt1 = len0;

    /*
     * If this is the last record in the file, then just do that.
     */
    if (out_buf[XXX_FNC] == REC_EOF)
    {
	if (fputc(REC_EOF, fil0) == EOF)
    	{
    	    printf("Error writing to output file.\n\n");
	    return (-1);
    	}

        out_total++;
    	return (0);
    }

    /*
     * Get the record length and pointer
     */

    if (cnt1 > 255)
    {
	printf("Record is too long (%d bytes).  Maximum is 255 bytes after translation.\n", out_len);
	dis_err(&inp_buf[0]);
    	cnt1 = 255;
    }

    out_buf[XXX_LEN] = cnt1;

    /*
     * Write the record.
     */
    while (cnt1-- > 0)
    {
	if (fputc(*pnt1++, fil0) == EOF)
    	{
    	    printf("Error writing to output file.\n\n");
	    return (-1);
    	}
        out_total++;
    }

    return (1);
}

/*
 * dis_err ***
 *
 * Display an error.
 *
 * Inputs:
 *	pnt0 - character pointer to character in "inp_buf" which is in error.
 *
 * Outputs:
 *	...
 */     

static int
dis_err (
const char *pnt0)
{						  
    register int spcount = pnt0 - &inp_buf[0];

    err_cnt++;

    printf("Error in line %ld:\n%s\n", lin_cnt, &sav_buf[0]);

    while (spcount-- > 0)
    {
	printf(" ");
    }

    printf("^\n");
}

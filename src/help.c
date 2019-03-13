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
w
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
#if defined HAVE_NCURSESW_CURSES_H
# include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
# include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
# include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
# include <ncurses.h>
#elif defined HAVE_CURSES_H
# include <curses.h>
#endif
#include <errno.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_LIBREADLINE
# if defined(HAVE_READLINE_READLINE_H)
#  include <readline/readline.h>
# elif defined(HAVE_READLINE_H)
#  include <readline.h>
# else /* !defined(HAVE_READLINE_H) */
extern char *readline (); 
# endif /* !defined(HAVE_READLINE_H) */
#else /* !defined(HAVE_READLINE_READLINE_H) */
# error "readline is not found on this system"
#endif /* HAVE_LIBREADLINE */
#include "common.h"
#include "queue.h"
#define LINE_MAX 512
#define ALPHABET "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define WHITESPACE " \t"

    typedef struct _topic {
        struct _topic *parent; /**< Pointer to previous level */
        void *child;            /*<< Pointer to tree of children */
        char *title;            /**< Current topic */
        int level;              /**< Current level */
        off_t offset;           /**< Offset in file of help topic text */
    } TOPIC;

/*
 * Forward declaration
 */
    int act_help(void *ctx);
    static int open_help(void);
    static TOPIC *talloc(const char *topic, int level, off_t offset);
    static int ncompare(const void *p1, const void *p2);
    static int lookup_subtopic(const void *p1, const void *p2);
    static void list_subtopics(const void *nodep, const VISIT which, const int depth);
    static int navigate_help(const TOPIC *topic);
    static void print_topic(const TOPIC *topic);

/*
 * OWN storage
 */
    static TOPIC top = { 0 };
    static FILE *helpfl = 0;

/**
 * HELP command action routine
 *
 * @detail
 * This routine processes the pjlcp HELP command,  It also
 * triggers loading of the HELP file if it has not already
 * been done.
 *
 * @param ctx Parse context
 *
 * @return ...
 */
int act_help(void *ctx) {
    static struct ELEMENT {
        void *prev, *next;
        char *arg;
    } args;
    int status;
    struct ELEMENT *e;
    PCBDEF *pcbp = ctx;

    if (!helpfl)
        open_help();

    switch (pcbp->prs.av1) {
    case OP_INIT:
        queue_init(&args);
        break;

    case OP_STORE:
        e = malloc(sizeof(struct ELEMENT));
        if (e == 0) raise(SIGSEGV);

        e->arg = strndup(pcbp->prs.cur, pcbp->prs.end - pcbp->prs.cur);
        queue_insert(e, args.prev);
        break;

    case OP_FINISH:
        if (queue_empty(&args)) {
            status = navigate_help(&top);
        } else {
            debug ("HELP, with commands");

            e = args.next;
            while (e != &args) {
                e = e->next;
            }
        }

        if (status == EOF) fputc('\n', stdout);
        break;
    }

    return ACT_SUCCESS;
} /* act_help */

/**
 * Open the pjlcp helpfile
 *
 * @detail
 * This internal routine opens the pjlcp help (formatted as
 * a VMS help file) and loads it into memory.
 *
 * @return 0 Success
 * @return 1 Error loading help library
 */
static int open_help(void) {

    int level = 0;
    TOPIC *curp = &top;
    char *line, linebuf[LINE_MAX];

    debug("Opening help file at %s", "pjlcp.hlp");

    helpfl = fopen("pjlcp.hlp", "r");
    if (!helpfl)
        return 1;

    memset(&top, 0, sizeof(top));

    while (line = fgets(linebuf, LINE_MAX, helpfl)) {
        char *topic = 0, *eol = 0;
        int n;
        off_t offset;
        TOPIC *newp = 0, **nodep;

        /* Skip all comment lines */
        if (line[0] == '!')
            continue;

        /* Skip lines that are not topic headers */
        if (!isdigit(line[0]))
            continue;

        offset = ftell(helpfl);

        /* Fetch help ltopic level */
        n = strtol(line, &line, 10);

        /* Skip whitspace */
        line += strspn(line, WHITESPACE);

        /* Fetch topic name */
        topic = line;

        /* Terminate line, dropping \r\n */
        eol = line;
        do {
            eol += strspn(eol, ALPHABET);

            /* Convert a space to underscore */
            if (*eol == ' ')
                *eol = '_';
        } while(*eol == '_');
        *eol = '\0';

        debug("Reading topic %s at level %d", topic, n);
        if ((n == 0) || (n > (level + 1))) {
            debug("Help topic %s at level %d is illegal; discarding", topic, n);
            continue;
        }

        newp = talloc(topic, n, offset);

        if (n > level) {
            newp->parent = curp;
            nodep = (TOPIC **) tsearch(newp, &curp->child, ncompare);
        } else {
            if (n < level) {
                do {
                    curp = curp->parent;
                } while (curp->level != n);
            }

            newp->parent = curp->parent;
            nodep = (TOPIC **) tsearch(newp, &curp->parent->child, ncompare);
        }

        if (*nodep != newp) {
            debug("Help library inconsistent, topic already loaded!");
        }

        level = n;
        curp = newp;
    }

    return 0;
} /* open_help */

/**
 * Allocate an populate @ref TOPIC
 *
 * @detail
 * Allocate and populate a @ref TOPIC structure.  All storage needed
 * to fill the block is allocated in a single allocation.  This way
 * only a single call to free is necessary to release the TOPIC
 * back to heap.
 *
 * @note
 * This routine signals SIGSEGV when unable to  allocate storage, so
 * there is no need to test the result being returned.
 *
 * @param title Topic title
 * @param level Help topic level
 * @param offset Offset within @ref helpfl where topic content starts.
 *
 * @return Address of a TOPIC structure.
 */
static TOPIC *talloc(const char *title,
                     int level,
                     off_t offset) {
    char *buf = 0;
    TOPIC *np = 0;

    buf = calloc(1, sizeof(TOPIC) + strlen(title) + 1);
    if (!buf) raise(SIGSEGV);

    np = (TOPIC *) buf;
    np->title = buf + sizeof(TOPIC);

    np->level = level;
    np->offset = offset;
    strcpy(np->title, title);

    return np;
} /* talloc */

static int ncompare(const void *p1,
                    const void *p2) {
    const TOPIC *tp1 = p1, *tp2 = p2;

    return strcasecmp(tp1->title, tp2->title);
} /* ncompare */

/**
 * Lookup HELP subtopics
 *
 * @detail
 * This routine is used to compare the user's input in a help
 * request with the the available help topics.  So, it deliberately
 * makes partial matches. It is expected to be passed to tfind.
 *
 * @param p1 User supplied keyword
 * @param p2 Help topic keyword
 *
 * @return See strncasecmp for return value.
 */
static int lookup_subtopic(const void *p1,
                           const void *p2) {
    const TOPIC *tp1 = p1, *tp2 = p2;
    int result;

    result = strncasecmp(tp1->title, tp2->title,
                         MIN(strlen(tp1->title), strlen(tp2->title)));

    return result;
} /* lookup_subtopic */

/**
 * List HELP subtopics
 *
 * @detail
 * This routine lists the subtopics available for a particular
 * HELP topics.  It is expected to be passed to twalk.
 */
static void list_subtopics(const void *nodep,
                          const VISIT which,
                          const int depth) {
    TOPIC *tp = *(TOPIC **) nodep;

    if ((which == preorder) || (which == endorder))
        return;

    fprintf(stdout, "\t%s", tp->title);
} /* list_subtopics */

/**
 * Navigate HELP file
 *
 * @detail
 * Navigate a HELP topic and all its child topics.  This is
 * the main interface to the HELP command.
 *
 * @param topic TOPIC structure to be explored
 *
 * @return 0 Continue processing HELP commands
 * @return EOF Received an EOF, do not continue processing
 */
static int navigate_help(const TOPIC *topic) {
    char *next = 0, *prompt = 0;
    int action = EXIT_SUCCESS, status;

    /* Print help text, if there is some */
    print_topic(topic);

    if (topic->child != 0) {
        /* Build the HELP prompt */
        if (topic->title == 0) {
            prompt = strdup("Topic? ");
        } else {
            char *newp;
            const TOPIC *curp = topic;

            while (curp->parent != 0) {
                prompt = cat(prompt, "%s", curp->title);
                curp = curp->parent;
            }

            prompt = cat(prompt, "Subtopic? ");
        }

        /* Request intput from the user */
        next = readline(prompt);
        while ((next != 0) && (*next != '\0')) {
            TOPIC *child, *tmp;

            if (*next == '?') {
                print_topic(topic);
            } else {
                tmp = talloc(next, 0, 0);
                child = tfind(tmp, &topic->child, lookup_subtopic);
                if (child == 0) {
                    fprintf(stdout, "  Sorry, no documentation on %s\n\n", next);
                } else {
                    action = navigate_help(*(TOPIC **) child);
                    if (action == EOF) break;
                }
            
                free(tmp);
            }

            next = readline(prompt);
        }

        if (next == 0) action = EOF;

        free(prompt);
    }

    return action;
} /* navigate_help */

static void print_topic(const TOPIC *topic) {
    char linebuf[LINE_MAX], *line;
    int status;

#ifdef HAVE_CURSES
    putp(tigetstr("clear"));
#endif

    if (topic->title != 0) {
        status = fseek(helpfl, topic->offset, SEEK_SET);
        if (status != 0) {
            error(errno, "Unable to seek help file");
            raise(SIGBUS);
        }

        fprintf(stdout, "\n  %s\n", topic->title);

        while (line = fgets(linebuf, LINE_MAX, helpfl)) {
            /* Skip all comment lines */
            if (line[0] == '!')
                continue;

            /* If the line starts with a number, we have reached
             * next section.
             */
            if (isdigit(line[0])) break;
            fprintf(stdout, "    %s", line);
        }

        fputc('\n', stdout);
    }

    /* Print subtopics */
    if (topic->child) {
        fprintf(stdout, "  Additional information available: \n\n");
        twalk(topic->child, list_subtopics);
        fprintf(stdout, "\n\n");
    }
} /* print_topic */

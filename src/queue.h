/*
** QUEUE.H
**
**  Header file containing definitions related to queue manipulation.
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
**  22-OCT-2014  Sneddon    Initial coding.
*/
#ifndef PJLCP_QUEUE_H_
#define PJLCP_QUEUE_H_

/*
**  Status values.
*/

#define QUEUE_NOT_EMPTY     0
#define QUEUE_EMPTY         1

#define QUEUE_INSERT_ONLY   1
#define QUEUE_INSERT_MANY   2

#define QUEUE_REMOVE_EMPTY  0
#define QUEUE_REMOVE_MORE   1
#define QUEUE_REMOVE_NOMORE 2

/*
**  Queue header.
*/

    typedef struct _queue {
        struct _queue *next, *prev;
    } QUEUE;

/*
**  Initialize a queue.
*/

static inline void queue_init(void *__queue_head) {
    QUEUE *q = __queue_head;

    q->next = q->prev = q;
}

/*
**  Test if queue is empty
*/

static inline int queue_empty(void *__queue) {
    QUEUE *q = __queue;

    if (q->prev == q) {
        return QUEUE_EMPTY;
    } else {
        return QUEUE_NOT_EMPTY;
    }
}

/*
**  Insert an element.
*/
static inline int queue_insert(void *__new_entry,
                               void *__predecessor) {
    QUEUE *n = __new_entry, *p = __predecessor;
    int status;

    p->next->prev = n;
    n->next = p->next;
    p->next = n;
    n->prev = p;

    if (n->next == n->prev) {
        status = QUEUE_INSERT_ONLY;
    } else {
        status = QUEUE_INSERT_MANY;
    }

    return status;
}

/*
**  Remove an element.
*/

static inline int queue_remove(void *__entry) {
    QUEUE *e = __entry, *p = e->prev;
    int status;

    if (e == p) {
        status = QUEUE_REMOVE_EMPTY;
    } else {
        e->prev->next = e->next;
        e->next->prev = e->prev;

        if (p->next = p) {
            status = QUEUE_REMOVE_NOMORE;
        } else {
            status = QUEUE_REMOVE_MORE;
        }
    }

    return status;
}


#endif /* PJLCP_QUEUE_H_ */

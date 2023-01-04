/*
 * =====================================================================================
 *
 *       Filename:  gthread.h
 *
 *    Description:  This file defines the Data structure and APIs for Glue thread
 *
 *        Version:  1.0
 *        Created:  Monday 12 March 2018 02:01:51  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 *        This file is part of the SPFComputation distribution (https://github.com/sachinites).
 *        Copyright (c) 2017 Abhishek Sagar.
 *        This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation, version 3.
 *
 *        This program is distributed in the hope that it will be useful, but
 *        WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *        General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================================
 */

#ifndef __GLTHREAD__
#define __GLTHREAD__

typedef struct _gthread{

    struct _gthread *left;
    struct _gthread *right;
} gthread_t;

void
gthread_add_next(gthread_t *base_gthread, gthread_t *new_gthread);

void
gthread_add_before(gthread_t *base_gthread, gthread_t *new_gthread);

void
remove_gthread(gthread_t *gthread);

void
init_gthread(gthread_t *gthread);

void
gthread_add_last(gthread_t *base_gthread, gthread_t *new_gthread);

#define IS_GLTHREAD_LIST_EMPTY(gthreadptr)         \
    ((gthreadptr)->right == 0 && (gthreadptr)->left == 0)

#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name)                        \
    static inline structure_name * fn_name(gthread_t *gthreadptr){                   \
        return (structure_name *)((char *)(gthreadptr) - (char *)&(((structure_name *)0)->field_name)); \
    }

/* delete safe loop*/
/*Normal continue and break can be used with this loop macro*/

#define BASE(gthreadptr)   ((gthreadptr)->right)

#define ITERATE_GLTHREAD_BEGIN(gthreadptrstart, gthreadptr)                                      \
{                                                                                                  \
    gthread_t *_gthread_ptr = NULL;                                                              \
    gthreadptr = BASE(gthreadptrstart);                                                          \
    for(; gthreadptr!= NULL; gthreadptr = _gthread_ptr){                                        \
        _gthread_ptr = (gthreadptr)->right;

#define ITERATE_GLTHREAD_END(gthreadptrstart, gthreadptr)                                        \
        }}

#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(gthreadptr, offset)  \
    (void *)((char *)(gthreadptr) - offset)

void
delete_gthread_list(gthread_t *base_gthread);

unsigned int
get_gthread_list_count(gthread_t *base_gthread);

void
gthread_priority_insert(gthread_t *base_gthread,
                         gthread_t *gthread,
                         int (*comp_fn)(void *, void *),
                         int offset);


#if 0
void *
gl_thread_search(gthread_t *base_gthread,
        void *(*thread_to_struct_fn)(gthread_t *),
        void *key,
        int (*comparison_fn)(void *, void *));

#endif
#endif /* __GLTHREAD__ */

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

#ifndef _LIST_H_INCLUDED
#define _LIST_H_INCLUDED

#ifndef offsetof
    #define offsetof(type, member) ((size_t)&((type *)NULL)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member) (type *)((char *)ptr - offsetof(type, member))
#endif

struct list_node_t {
    struct list_node_t *prev;
    struct list_node_t *next;
};

typedef struct list_node_t list_t;

#define list_init(L)        \
    do {                    \
        (L)->prev = (L);    \
        (L)->next = (L);    \
    } while (0)

#define list_empty(L) ((L)->next == (L))

#define list_push_back(L, other)    \
    do {                            \
        (other)->next = (L);        \
        (other)->prev = (L)->prev;  \
        (L)->prev->next = (other);  \
        (L)->prev = (other);        \
    } while (0)

#define list_push_front(L, other)   \
    do {                            \
        (other)->next = (L)->next;  \
        (other)->prev = (L);        \
        (L)->next->prev = (other);  \
        (L)->next = (other);        \
    } while (0)

#define list_erase(node)                        \
        do {                                    \
            (node)->prev->next = (node)->next;  \
            (node)->next->prev = (node)->prev;  \
            (node)->prev = NULL;                \
            (node)->next = NULL;                \
        } while (0)

#define list_of(ptr, type, member) container_of(ptr, type, member)

#endif //!_LIST_H_INCLUDED
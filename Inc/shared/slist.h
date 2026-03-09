#ifndef INC_SLIST_H_
#define INC_SLIST_H_

#include <stddef.h>
#include <stdbool.h>

// Container of - Get the parent structure pointer from a member.
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

// slist_for_each - It iterates over all nodes in the list.
#define slist_for_each(pos, head) for((pos) = (head)->first; (pos) != NULL; (pos) = (pos)->next)

/* ================================================================== */
/*  Types                                                             */
/* ================================================================== */

typedef struct slist_node
{
    struct slist_node *next;
} slist_node_t;

typedef struct
{
    slist_node_t *first;
    unsigned      count;
} slist_head_t;

/* ================================================================== */
/*  Init                                                              */
/* ================================================================== */

void slist_init(slist_head_t *head);


/* ================================================================== */
/*  Insert/Remove                                                     */
/* ================================================================== */

void slist_push_front(slist_head_t *head, slist_node_t *node);
void slist_push_back(slist_head_t *head, slist_node_t *node);
void slist_remove(slist_head_t *head, slist_node_t *node);

/* ================================================================== */
/*  Consult                                                           */
/* ================================================================== */

bool slist_empty(const slist_head_t *head);
unsigned slist_count(const slist_head_t *head);
bool slist_contains(const slist_head_t *head, const slist_node_t *node);

#endif /* INC_SLIST_H_ */
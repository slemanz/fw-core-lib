#include "shared/slist.h"

void slist_init(slist_head_t *head)
{
    head->first = NULL;
    head->count = 0u;
}

void slist_push_front(slist_head_t *head, slist_node_t *node)
{
    node->next  = head->first;
    head->first = node;
    head->count++;
}

void slist_push_back(slist_head_t *head, slist_node_t *node)
{
    node->next = NULL;

    if (head->first == NULL)
    {
        head->first = node;
    }
    else
    {
        slist_node_t *cur = head->first;
        while (cur->next != NULL) cur = cur->next;
        cur->next = node;
    }

    head->count++;
}

void slist_remove(slist_head_t *head, slist_node_t *node)
{
    slist_node_t *cur  = head->first;
    slist_node_t *prev = NULL;

    while (cur != NULL)
    {
        if (cur == node)
        {
            if (prev == NULL) head->first = cur->next;
            else              prev->next  = cur->next;

            node->next = NULL;
            head->count--;
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}


bool slist_empty(const slist_head_t *head)
{
    return (head->first == NULL);
}

unsigned slist_count(const slist_head_t *head)
{
    return head->count;
}

bool slist_contains(const slist_head_t *head, const slist_node_t *node)
{
    slist_node_t *cur = head->first;
    while (cur != NULL)
    {
        if (cur == node) return true;
        cur = cur->next;
    }
    return false;
}
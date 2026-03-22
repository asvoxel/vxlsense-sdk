/*
 * Copyright (c) 2024 ASVoxel
 * SPDX-License-Identifier: MIT
 *
 * as_list.h - Intrusive doubly linked list (Linux kernel style)
 *
 * Usage:
 *   struct my_item {
 *       int data;
 *       as_list_node_t node;  // embed in your struct
 *   };
 *
 *   as_list_t list;
 *   as_list_init(&list);
 *
 *   struct my_item item;
 *   as_list_add_tail(&list, &item.node);
 *
 *   // iterate
 *   as_list_node_t *pos;
 *   as_list_for_each(pos, &list) {
 *       struct my_item *entry = as_list_entry(pos, struct my_item, node);
 *   }
 */

#ifndef __AS_LIST_H__
#define __AS_LIST_H__

#include "as_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* List node (embedded in user struct) */
typedef struct as_list_node {
    struct as_list_node *prev;
    struct as_list_node *next;
} as_list_node_t;

/* List head */
typedef struct as_list {
    as_list_node_t head;
} as_list_t;

/*
 * Get container struct from list node
 */
#define as_list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/*
 * Initialize list
 */
AS_INLINE void as_list_init(as_list_t *list)
{
    list->head.prev = &list->head;
    list->head.next = &list->head;
}

/*
 * Initialize a standalone node
 */
AS_INLINE void as_list_node_init(as_list_node_t *node)
{
    node->prev = node;
    node->next = node;
}

/*
 * Check if list is empty
 */
AS_INLINE bool as_list_empty(as_list_t *list)
{
    return list->head.next == &list->head;
}

/*
 * Internal: insert node between prev and next
 */
AS_INLINE void as_list_insert(as_list_node_t *node,
                               as_list_node_t *prev,
                               as_list_node_t *next)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
}

/*
 * Add node to head of list
 */
AS_INLINE void as_list_add_head(as_list_t *list, as_list_node_t *node)
{
    as_list_insert(node, &list->head, list->head.next);
}

/*
 * Add node to tail of list
 */
AS_INLINE void as_list_add_tail(as_list_t *list, as_list_node_t *node)
{
    as_list_insert(node, list->head.prev, &list->head);
}

/*
 * Remove node from list
 */
AS_INLINE void as_list_del(as_list_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node;
    node->next = node;
}

/*
 * Get first node (or NULL if empty)
 */
AS_INLINE as_list_node_t *as_list_first(as_list_t *list)
{
    if (as_list_empty(list)) {
        return NULL;
    }
    return list->head.next;
}

/*
 * Get last node (or NULL if empty)
 */
AS_INLINE as_list_node_t *as_list_last(as_list_t *list)
{
    if (as_list_empty(list)) {
        return NULL;
    }
    return list->head.prev;
}

/*
 * Iterate over list
 */
#define as_list_for_each(pos, list) \
    for ((pos) = (list)->head.next; \
         (pos) != &(list)->head; \
         (pos) = (pos)->next)

/*
 * Iterate over list (safe for removal)
 */
#define as_list_for_each_safe(pos, tmp, list) \
    for ((pos) = (list)->head.next, (tmp) = (pos)->next; \
         (pos) != &(list)->head; \
         (pos) = (tmp), (tmp) = (pos)->next)

/*
 * Iterate over list entries
 */
#define as_list_for_each_entry(pos, list, type, member) \
    for ((pos) = as_list_entry((list)->head.next, type, member); \
         &(pos)->member != &(list)->head; \
         (pos) = as_list_entry((pos)->member.next, type, member))

/*
 * Iterate over list entries (safe for removal)
 */
#define as_list_for_each_entry_safe(pos, tmp, list, type, member) \
    for ((pos) = as_list_entry((list)->head.next, type, member), \
         (tmp) = as_list_entry((pos)->member.next, type, member); \
         &(pos)->member != &(list)->head; \
         (pos) = (tmp), \
         (tmp) = as_list_entry((pos)->member.next, type, member))

/*
 * Count nodes in list
 */
AS_INLINE size_t as_list_count(as_list_t *list)
{
    size_t count = 0;
    as_list_node_t *pos;

    as_list_for_each(pos, list) {
        count++;
    }
    return count;
}

#ifdef __cplusplus
}
#endif

#endif /* __AS_LIST_H__ */

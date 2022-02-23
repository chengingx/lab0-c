#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

struct list_head *mergesort(struct list_head *head);
struct list_head *mergeTwoLists(struct list_head *left,
                                struct list_head *right);

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *cur = NULL, *next = NULL;

    list_for_each_entry_safe (cur, next, head, list)
        q_release_element(cur);

    free(head);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *q_node = malloc(sizeof(element_t));

    if (!q_node)
        return false;

    q_node->value = strdup(s);

    if (!q_node->value) {
        q_release_element(q_node);
        return false;
    }

    list_add(&q_node->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *q_node = malloc(sizeof(element_t));

    if (!q_node)
        return false;

    q_node->value = strdup(s);

    if (!q_node->value) {
        q_release_element(q_node);
        return false;
    }

    list_add_tail(&q_node->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *removed_node = list_first_entry(head, element_t, list);

    list_del_init(&removed_node->list);

    if (sp) {
        strncpy(sp, removed_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return removed_node;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *removed_node = list_last_entry(head, element_t, list);

    list_del_init(&removed_node->list);

    if (sp) {
        strncpy(sp, removed_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return removed_node;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li = NULL;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *fast_ptr = head->next;
    struct list_head *slow_ptr = head->next;

    while (fast_ptr != head && fast_ptr != head->prev) {
        fast_ptr = fast_ptr->next->next;
        slow_ptr = slow_ptr->next;
    }

    element_t *mid_node = list_entry(slow_ptr, element_t, list);
    list_del_init(&mid_node->list);
    q_release_element(mid_node);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    q_sort(head);

    element_t *slow = NULL, *fast = NULL;

    list_for_each_entry_safe (slow, fast, head, list) {
        if (slow->list.next != head && strcmp(slow->value, fast->value) == 0) {
            list_del_init(&slow->list);
            q_release_element(slow);
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    if (!head)
        return;

    for (struct list_head *node = head->next;
         (node != head) && (node->next != head); node = node->next) {
        struct list_head *next_node = node->next;
        list_del_init(node);
        list_add(node, next_node);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node = NULL, *safe = NULL;

    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
struct list_head *mergeTwoLists(struct list_head *left, struct list_head *right)
{
    struct list_head *head = NULL, **ptr = &head, **node = NULL;

    for (; left && right; *node = (*node)->next) {
        node = (strcmp(list_entry(left, element_t, list)->value,
                       list_entry(right, element_t, list)->value) < 0)
                   ? &left
                   : &right;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((__UINTPTR_TYPE__) left |
                                 (__UINTPTR_TYPE__) right);

    return head;
}

struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head, *slow = head, *mid;

    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }

    mid = slow;
    slow->prev->next = NULL;

    struct list_head *left = mergesort(head);
    struct list_head *right = mergesort(mid);
    return mergeTwoLists(left, right);
}

void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    head->prev->next = NULL;
    head->next = mergesort(head->next);

    struct list_head **ptr = &head;

    for (; (*ptr)->next != NULL; ptr = &((*ptr)->next)) {
        (*ptr)->next->prev = *ptr;
    }

    (*ptr)->next = head;
    head->prev = *ptr;
}
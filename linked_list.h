#ifndef CODERODDE_LINKED_LIST_H
#define CODERODDE_LINKED_LIST_H

#include <stdbool.h>
#include <stdlib.h>

enum ll_error_code {
    LL_NO_ERROR,
    LL_INDEX_OUT_OF_BOUNDS,
    LL_LIST_IS_EMPTY,
    LL_ITERATOR_OUT_OF_BOUNDS,
    LL_ITERATOR_NULL
};

struct linked_list;
struct linked_list_iterator;

struct linked_list* linked_list_create(void);

// Insertion functions.
bool linked_list_push_front(struct linked_list* list, void* value);
bool linked_list_push_back(struct linked_list* list, void* value);
bool linked_list_insert(struct linked_list* list, size_t index, void* value);
bool linked_list_push_front_array(struct linked_list* list, void** values, size_t count);
bool linked_list_push_back_array(struct linked_list* list, void** values, size_t count);
bool linked_list_insert_array(struct linked_list* list, size_t index, void** values, size_t count);

// Size querying functions.
size_t linked_list_size(struct linked_list* list);
bool linked_list_is_empty(struct linked_list* list);

// Access functions.
bool linked_list_front(struct linked_list* list, void** value);
bool linked_list_back(struct linked_list* list, void** value);
bool linked_list_get(struct linked_list* list, size_t index, void** value);

// Removal functions.
bool linked_list_pop_front(struct linked_list* list, void** value);
bool linked_list_pop_back(struct linked_list* list, void** value);
bool linked_list_remove(struct linked_list* list, size_t index, void** value);
bool linked_list_remove_sub_list(struct linked_list* list, size_t index, size_t count);
void linked_list_clear(struct linked_list* list);

// Iterator functions.
struct linked_list_iterator* linked_list_iterator_create(struct linked_list* list, size_t index);
bool linked_list_iterator_has_next(struct linked_list_iterator* iterator);
bool linked_list_iterator_next(struct linked_list_iterator* iterator, void** value);
bool linked_list_iterator_has_prev(struct linked_list_iterator* iterator);
bool linked_list_iterator_prev(struct linked_list_iterator* iterator, void** value);
size_t linked_list_iterator_next_index(struct linked_list_iterator* iterator, enum ll_error_code* err);
size_t linked_list_iterator_prev_index(struct linked_list_iterator* iterator, enum ll_error_code* err);
bool linked_list_iterator_set(struct linked_list_iterator* iterator, void* value);
bool linked_list_iterator_add(struct linked_list_iterator* iterator, void* value);
bool linked_list_iterator_remove(struct linked_list_iterator* iterator);
void linked_list_iterator_destroy(struct linked_list_iterator* iterator);

// Destruction function.
void linked_list_destroy(struct linked_list* list);

#endif // CODERODDE_LINKED_LIST_H
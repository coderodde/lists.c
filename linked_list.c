#include "linked_list.h"
#include <stdbool.h>
#include <stdlib.h>

struct linked_list_node {
    void* value;
    struct linked_list_node* next;
    struct linked_list_node* prev;
};

struct linked_list {
    struct linked_list_node* head;
    struct linked_list_node* tail;
    size_t size;
};

struct linked_list_iterator {
    struct linked_list* list;
    struct linked_list_node* last_returned;
    struct linked_list_node* next;
    size_t next_index;
};

struct linked_list* linked_list_create();

// Insertion functions.
bool linked_list_push_front(struct linked_list* list, void* value)
{
    
}

bool linked_list_push_back(struct linked_list* list, void* value);
bool linked_list_insert(struct linked_list* list, size_t index, void* value);
bool linked_list__push_front_array(struct linked_list* list, void** values, size_t count);
bool linked_list__push_back_array(struct linked_list* list, void** values, size_t count);
bool linked_list_insert_array(struct linked_list* list, size_t index, void** values, size_t count);

// Size querying functions.
size_t linked_list_size(struct linked_list* list);
bool linked_list_is_empty(struct linked_list* list);

// Access functions.
bool linked_list_front(struct linked_list* list, void** value);
bool linked_list_back(struct linked_list* list, void** value);
bool linked_list_get(struct linked_list* list, size_t index, void** value);

// Removal functions.
void* linked_list_pop_front(struct linked_list* list);
void* linked_list_pop_back(struct linked_list* list);
void* linked_list_remove(struct linked_list* list, size_t index);
bool linked_list_remove_sub_list(struct linked_list* list, size_t index, size_t count);
void linked_list_clear(struct linked_list* list);

// Access the node at the given index. 
// This function is used internally by other functions to access nodes efficiently.
static struct linked_list_node* get_node(struct linked_list* list, size_t index)
{
    size_t left_distance  = index;
    size_t right_distance = linked_list_size(list) - index - 1;

    if (left_distance < right_distance) 
    {
        struct linked_list_node* current = list->head;

        for (size_t i = 0; i < index; ++i) 
        {
            current = current->next;
        }

        return current;
    } else {
        struct linked_list_node* current = list->tail;
        
        for (size_t i = 0; i < right_distance; ++i) {
            current = current->prev;
        }

        return current;
    }
}

// Iterator functions.
struct linked_list_iterator* linked_list_iterator_create(struct linked_list* list, size_t index)
{
    struct linked_list_iterator* iterator = malloc(sizeof(struct linked_list_iterator));

    if (iterator == NULL) {
        return NULL;
    }

    if (linked_list_is_empty(list)) {
        iterator->next = (index == linked_list_size(list)) ? NULL : get_node(list, index);
    }

    iterator->next_index = index;
    return iterator;
}

bool linked_list_iterator_has_next(struct linked_list_iterator* iterator)
{
    return iterator != NULL && iterator->next_index < linked_list_size(iterator->list);
}

bool linked_list_iterator_next(struct linked_list_iterator* iterator, void** value)
{
    if (iterator == NULL) 
    {
        return false;
    }

    if (!linked_list_iterator_has_next(iterator))
    {
        return false;
    }
    
    iterator->last_returned = iterator->next;
    iterator->next          = iterator->next->next;
    iterator->next_index++;

    return true;
}

bool linked_list_iterator_has_prev(struct linked_list_iterator* iterator)
{
    return iterator != NULL && iterator->next_index > 0;
}

bool linked_list_iterator_prev(struct linked_list_iterator* iterator, void** value)
{
    if (iterator == NULL)
    {
        return false;
    }

    if (!linked_list_iterator_has_prev(iterator))
    {
        return false;
    }

    iterator->last_returned = iterator->next = (iterator->next == NULL)
                            ? iterator->list->tail 
                            : iterator->next->prev;
    iterator->next_index--;

    return true;
}

size_t linked_list_iterator_next_index(struct linked_list_iterator* iterator, enum ll_error_code* err)
{
    if (iterator == NULL)
    {
        if (err != NULL)
        {
            *err = LL_ITERATOR_NULL;
        }

        return 0;
    }

    if (!linked_list_iterator_has_next(iterator))
    {
        if (err != NULL)
        {
            *err = LL_ITERATOR_OUT_OF_BOUNDS;
        }

        return 0;
    }

    if (err != NULL) {
        *err = LL_NO_ERROR;
    }

    return iterator->next_index;
}

size_t linked_list_iterator_prev_index(struct linked_list_iterator* iterator, enum ll_error_code* err)
{
    if (iterator == NULL)
    {
        if (err != NULL)
        {
            *err = LL_ITERATOR_NULL;
        }

        return 0;
    }

    if (!linked_list_iterator_has_prev(iterator))
    {
        if (err != NULL)
        {
            *err = LL_ITERATOR_OUT_OF_BOUNDS;
        }

        return 0;
    }

    if (err != NULL) {
        *err = LL_NO_ERROR;
    }

    return iterator->next_index - 1;
}

bool linked_list_iterator_set(struct linked_list_iterator* iterator, void* value)
{
    if (iterator == NULL || iterator->last_returned == NULL)
    {
        return false;
    }

    iterator->last_returned->value = value;
    return true;
}

static bool link_before(struct linked_list* list, 
                        struct linked_list_node* succ, 
                        void* value)
{
    struct linked_list_node* pred     = succ->prev;
    struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));

    if (new_node == NULL)
    {
        return false;
    }

    new_node->value = value;
    new_node->prev = pred;
    new_node->next = succ;

    if (pred == NULL)
    {
        list->head = new_node;
    }
    else
    {
        pred->next = new_node;
    }

    list->size++;
    return true;
}

bool linked_list_iterator_add(struct linked_list_iterator* iterator, void* value)
{
    if (iterator == NULL || iterator->list == NULL)
    {
        return false;
    }

    iterator->last_returned = NULL;

    if (iterator->next == NULL) 
    {
        if (!linked_list_push_back(iterator->list, value))
        {
            return false;
        }
    } 
    else 
    {
        if (!link_before(iterator->list, iterator->next, value))
        {
            return false;
        }
    }
    
    iterator->next_index++;
    return true;
}

bool linked_list_iterator_remove(struct linked_list_iterator* iterator)
{
    if (iterator == NULL || iterator->last_returned == NULL)
    {
        return false;
    }

    struct linked_list_node* last_next = iterator->last_returned->next;

    unlink(last_next);

    if (iterator->next == iterator->last_returned)
    {
        iterator->next = last_next;
    } else
    {
        iterator->next_index--;
    }

    iterator->last_returned = NULL;
    return true;
}

void linked_list_iterator_destroy(struct linked_list_iterator* iterator) 
{
    if (iterator != NULL)
    {
        iterator->list = NULL; 
        iterator->last_returned = NULL;
        iterator->next = NULL;
        iterator->next_index = SIZE_MAX;
        free(iterator);
    }
 }

// Destruction function.
void linked_list_destroy(struct linked_list* list)
{
    if (list != NULL)
    {
        linked_list_clear(list);
        free(list);
    }
}
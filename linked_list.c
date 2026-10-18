#include "linked_list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void* check_not_null(void* ptr, const char* file, int line, const char* func)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "Null pointer error in %s at %s:%d\n", func, file, line);
        exit(EXIT_FAILURE);
    }

    return ptr;
}

#ifdef NDEBUG
#define CHECK_NOT_NULL(ptr) (ptr)
#else
#define CHECK_NOT_NULL(ptr) check_not_null(ptr, __FILE__, __LINE__, __func__)
#endif

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

struct linked_list* linked_list_create()
{
    struct linked_list* list = malloc(sizeof *list);

    if (list == NULL)
    {
        return list;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

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
    } 
    else 
    {
        struct linked_list_node* current = list->tail;

        for (size_t i = 0; i < right_distance; ++i) {
            current = current->prev;
        }

        return current;
    }
}

static void unlink_node(struct linked_list* list, struct linked_list_node* node)
{
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = list->head->next;
    }

    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = list->tail->prev;
    }
}

static void check_access_index_bounds(struct linked_list* list, size_t index, enum ll_error_code* err)
{
    if (index >= linked_list_size(list))
    {
        if (err != NULL)
        {
            *err = LL_INDEX_OUT_OF_BOUNDS;
        }
    } 
    else
    {
        if (err != NULL)
        {
            *err = LL_NO_ERROR;
        }
    }
}

static void check_insertion_index_bounds(struct linked_list* list, size_t index, enum ll_error_code* err)
{
    if (index > linked_list_size(list))
    {
        if (err != NULL)
        {
            *err = LL_INDEX_OUT_OF_BOUNDS;
        }
    } 
    else
    {
        if (err != NULL)
        {
            *err = LL_NO_ERROR;
        }
    }
}

// Insertion functions.
bool linked_list_push_front(struct linked_list* list, void* value)
{
    if (list == NULL)
    {
        return false;
    }

    struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));

    if (new_node == NULL)
    {
        return false;
    }

    new_node->value = value;
    new_node->next  = list->head;
    new_node->prev  = NULL;

    if (list->head != NULL)
    {
        list->head->prev = new_node;
    }
    else
    {
        list->tail = new_node;
    }

    list->head = new_node;
    list->size++;

    return true;
}

bool linked_list_push_back(struct linked_list* list, void* value) 
{
    if (list == NULL)
    {
        return false;
    }

    struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));
    
    if (new_node == NULL)
    {
        return false;
    }

    new_node->value = value;
    new_node->next = NULL;
    new_node->prev = list->tail;

    if (list->tail != NULL)
    {
        list->tail->next = new_node;
    }
    else
    {
        list->head = new_node;
    }

    list->tail = new_node;
    list->size++;

    return true;
}

bool linked_list_insert(struct linked_list* list, size_t index, void* value)
{
    if (list == NULL)
    {
        return false;
    }

    enum ll_error_code err;

    check_insertion_index_bounds(list, index, &err);

    if (err != LL_NO_ERROR)
    {
        return false;
    }

    if (index == 0)
    {
        return linked_list_push_front(list, value);
    } 
    else if (index == linked_list_size(list))
    {
        return linked_list_push_back(list, value);
    }
    else
    {
        struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));

        if (new_node == NULL)
        {
            return false;
        }

        struct linked_list_node* succ     = get_node(list, index);
        struct linked_list_node* pred     = succ->prev;

        new_node->value = value;
        new_node->next  = succ;
        new_node->prev  = pred;
        pred->next      = new_node;
        succ->prev      = new_node;
        
        list->size++;

        return true;
    }
}

static void rollback_on_push_front_array(struct linked_list* list, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        struct linked_list_node* node_to_remove = list->head;

        list->head = node_to_remove->next;

        if (list->head != NULL)
        {
            list->head->prev = NULL;
        } 
        else
        {
            list->tail = NULL;
        }

        free(node_to_remove);
    }

    list->size -= count;
}

static void rollback_on_push_back_array(struct linked_list* list, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        struct linked_list_node* node_to_remove = list->tail;

        list->tail = node_to_remove->prev;

        if (list->tail != NULL)
        {
            list->tail->next = NULL;
        }
        else
        {
            list->head = NULL;
        }

        free(node_to_remove);
    }

    list->size -= count;
}

bool linked_list_push_front_array(struct linked_list* list, void** values, size_t count)
{
    if (list == NULL || count == 0) {
        return false;
    }

    size_t successfully_pushed = 0;

    for (size_t i = count; i > 0; --i)
    {
        void* value = values[i - 1];

        struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));

        if (new_node == NULL)
        {
            // Rollback the successfully pushed elements.
            rollback_on_push_front_array(list, successfully_pushed);
            return false;
        }

        new_node->value = value;
        new_node->next = list->head;

        if (list->head == NULL)
        {
            list->tail = new_node;
            new_node->prev = NULL;
        } 
        else
        {
            list->head->prev = new_node;
            new_node->next = list->head;
        }

        list->head = new_node;
        list->size++;
        successfully_pushed++;
    }

    return true;
}

bool linked_list_push_back_array(struct linked_list* list, void** values, size_t count)
{
    if (list == NULL || count == 0) {
        return false;
    }

    size_t successfully_pushed = 0;

    for (size_t i = 0; i < count; ++i)
    {
        void* value = values[i];

        struct linked_list_node* new_node = malloc(sizeof(struct linked_list_node));

        if (new_node == NULL)
        {
            // Rollback the successfully pushed elements.
            rollback_on_push_back_array(list, successfully_pushed);
            return false;
        }

        new_node->value = value;
        new_node->next = NULL;

        if (list->tail == NULL)
        {
            list->head = new_node;
            new_node->prev = NULL;
        } 
        else
        {
            list->tail->next = new_node;
            new_node->prev = list->tail;
        }

        list->tail = new_node;
        list->size++;
        successfully_pushed++;
    }

    return true;
}

static void purge_local_list(struct linked_list_node* local_head)
{
    while (local_head)
    {
        struct linked_list_node* next = local_head->next;

        free(local_head);

        local_head = next;
    }
}

bool linked_list_insert_array(struct linked_list* list, size_t index, void** values, size_t count)
{
    if (list == NULL || count == 0)
    {
        return false;
    }

    enum ll_error_code err;

    check_insertion_index_bounds(list, index, &err);

    if (err != LL_NO_ERROR)
    {
        return false;
    }

    if (index == 0)
    {
        return linked_list_push_front_array(list, values, count);
    }
    
    if (index == linked_list_size(list))
    {
        return linked_list_push_back_array(list, values, count);
    }

    // Construct the linked list to insert:
    struct linked_list_node* local_head = malloc(sizeof(*local_head));
    struct linked_list_node* local_tail = local_head;
    struct linked_list_node* local_pred;
    struct linked_list_node* local_succ;

    if (local_head == NULL)
    {
        return false;
    }

    local_head->value = values[0];
    local_head->next  = NULL;

    for (size_t i = 1; i < count; ++i)
    {
        struct linked_list_node* new_local_node = malloc(sizeof(*new_local_node));

        if (new_local_node == NULL)
        {
            purge_local_list(local_head);
            return false;
        }

        local_tail->next     = new_local_node;
        new_local_node->prev = local_tail;
        local_tail           = new_local_node;
    }

    // Stitch the new linked list something in between:
    local_pred->next = local_head;
    local_head->prev = local_pred;
    local_tail->next = local_succ;
    local_succ->prev = local_tail;

    return true;
}

// Size querying functions.
size_t linked_list_size(struct linked_list* list)
{
    return list != NULL ? list->size : 0;
}

bool linked_list_is_empty(struct linked_list* list)
{
    return list != NULL ? list->size == 0 : true;
}

// Access functions.
bool linked_list_front(struct linked_list* list, void** value)
{
    if (list == NULL)
    {
        return false;
    }

    if (list->size == 0)
    {
        return false;
    }
    
    *value = list->head->value;
    return true;
}

bool linked_list_back(struct linked_list* list, void** value)
{
    if (list == NULL)
    {
        return false;
    }

    if (list->size == 0)
    {
        return false;
    }

    *value = list->tail->value;
    return true;
}

bool linked_list_get(struct linked_list* list, size_t index, void** value)
{
    if (list == NULL)
    {
        return false;
    }

    if (list->size == 0)
    {
        return false;
    }

    enum ll_error_code err;

    check_access_index_bounds(list, index, &err);

    if (err != LL_NO_ERROR)
    {
        return false;
    }

    struct linked_list_node* node = get_node(list, index);
    *value = node->value;
    return true;
}

// Removal functions.
bool linked_list_pop_front(struct linked_list* list, void** value)
{
    if (list == NULL || list->size == 0)
    {
        return false;
    }

    struct linked_list_node* head = list->head;

    if (value != NULL)
    {
        *value = head->value;
    }

    unlink_node(list, head);
    return true;
}

bool linked_list_pop_back(struct linked_list* list, void** value)
{
    if (list == NULL || list->size == 0)
    {
        return false;
    }

    struct linked_list_node* tail = list->tail;

    if (value != NULL)
    {
        *value = tail->value;   
    }

    unlink_node(list, tail);
    return true;
}

bool linked_list_remove(struct linked_list* list, size_t index, void** value)
{
    if (list == NULL || list->size == 0)
    {
        return false;
    }

    enum ll_error_code err;

    check_access_index_bounds(list, index, &err);

    if (err != LL_NO_ERROR)
    {
        return false;
    }

    struct linked_list_node* node = get_node(list, index);

    if (value != NULL)
    {
        *value = node->value;
    }

    unlink_node(list, node);
    return true;
}

bool linked_list_remove_sub_list(struct linked_list* list, size_t index, size_t count)
{
    if (list == NULL || count == 0)
    {
        return false;
    }

    enum ll_error_code err;

    check_access_index_bounds(list, index, &err);

    if (err != LL_NO_ERROR)
    {
        return false;
    }

    size_t to_index = index + count;

    if (to_index > list->size)
    {
        return false;
    }

    struct linked_list_node* remove_head  = get_node(list, index);
    struct linked_list_node* remove_tail  = get_node(list, to_index - 1);
    struct linked_list_node* pred         = remove_head->prev;
    struct linked_list_node* succ         = remove_tail->next;
    struct linked_list_node* current_node = remove_head;
    
    for (size_t i = 0; i < count; ++i)
    {
        struct linked_list_node* next = current_node->next;

        free(current_node);
        
        current_node = next;
    }

    if (pred != NULL)
    {
        pred->next = succ;

        if (succ != NULL)
        {
            succ->prev = pred;
        }
    }
    else
    {
        list->head = succ;
    }

    if (succ != NULL)
    {
        succ->prev = pred;

        if (pred != NULL)
        {
            pred->next = succ;
        }
    }
    else
    {
        list->tail = pred;
    }

    return true;
}

void linked_list_clear(struct linked_list* list)
{
    if (list == NULL)
    {
        return;
    }

    struct linked_list_node* node = list->head;

    while (node != NULL)
    {
        struct linked_list_node* next = node->next;
        free(node);
        node = next;
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
    
    *value                  = iterator->last_returned->value;
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

    *value                  = iterator->last_returned->value;
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

    unlink_node(iterator->list, last_next);

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
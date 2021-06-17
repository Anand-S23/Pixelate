#include "drift_data_structures.h"

/* Linked List */

// Makes a new node using malloc
internal node *CreateNode(void *data, node* next)
{
    node *ret_node = (node *)malloc(sizeof(node));
    ret_node->data = data;
    ret_node->next = next;
    return ret_node;
}

// Makes a new empty list
internal linked_list CreateList()
{
    linked_list list = {0};
    list.head = NULL;
    list.tail = NULL;
    list.length = 0;
    return list; 
} 

// Pushes node on the beginning of the list
internal void Push(linked_list *list, void *data)
{
    node *new_node = CreateNode(data, NULL);
    new_node->next = list->head;
    list->head = new_node;

    if (list->tail == NULL)
    {
        list->tail = list->head->next;
    }

    ++list->length;
}

#define PushType(list, type, data)               \
{                                                \
    type x = data;                               \
    Push(list, &x);                              \
}                                                \

// Appends node to the end of the list
internal void Append(linked_list *list, void *data)
{
    node *new_node = CreateNode(data, NULL);

    if (list->head == NULL)
    {
        list->head = new_node;
    }
    else if (list->tail == NULL)
    {
        list->tail = new_node;
        list->head->next = list->tail;
    }
    else
    {
        list->tail->next = new_node;
        list->tail = list->tail->next;
    }

    ++list->length;
}

#define AppendType(list, type, data)             \
{                                                \
    type x = data;                               \
    Append(list, &x);                            \
}                                                \

// Inserts a node into specified position
internal b32 Insert(linked_list *list, void *data, int pos)
{
    if (pos > list->length || pos < 0)
    {
        return 0;
    }

    node *current = list->head;
    for (int i = 0; i < pos - 1; ++i)
    {
        current = current->next;
    }

    node *new_node = CreateNode(data, NULL);
    new_node->next = current->next;
    current->next = new_node;

    ++list->length;
    return 1;
}

#define InsertType(list, type, data, pos)        \
{                                                \
    type x = data;                               \
    Insert(list, &x, pos);                       \
}                                                \

// Removes the first element from the list
internal void *Pop(linked_list *list)
{
    void *data = list->head->data;
    node *temp = list->head;
    list->head = list->head->next;
    free(temp);

    if (list->tail != NULL)
    {
        if (list->head->next == NULL)
        {
            list->tail = NULL;
        }
    }

    --list->length;
    return data;
}

#define PopType(list, type) ((type *)Pop(list, pos))

// Removes the last element from the list
internal void *PopLast(linked_list *list)
{
    node *current = list->head;
    for (int i = 0; i < list->length - 2; ++i)
    {
        current = current->next;
    }

    void *data = list->tail->data;
    node *temp = current->next;
    list->tail = current;
    free(temp);

    return data;
}

#define PopLastType(list, type) ((type *)PopLast(list, pos))

// Removes element form position
internal void *Remove(linked_list *list, int pos)
{
    if (pos >= list->length || pos < 0)
    {
        return NULL;
    }

    node *current = list->head;
    for (int i = 0; i < pos - 1; ++i)
    {
        current = current->next;
    }

    if (current->next == NULL)
    {
        current->next = NULL;
        return NULL;
    }

    node *temp = current->next;
    void *data = temp->data;
    current->next = current->next->next;
    free(temp);

    return data;
}

#define RemoveType(list, pos, type) ((type *)Remove(list, pos))
    
// Returns the data of the top node
internal void *Top(linked_list *list)
{
    return list->head->data;
}

#define TopType(list, type) ((type *)Top(list))

// Returns the data of the last node
internal void *Bottom(linked_list *list)
{
    return list->tail->data;
}

#define BottomType(list, type) ((type *)Bottom(list))

// Checks if the list is empty
internal b32 Empty(linked_list *list)
{
    return list->head == NULL;
}

// TODO: Get rid of this
internal u32 *Get(linked_list *ll, int index)
{
    u32 *result = NULL;

    if (index >= 0 && index < ll->length)
    {
        node *current = ll->head;
        for (int i = 0; i < index; ++i)
        {
            current = current->next;
        }

        result = (u32 *)current->data;
    }

    return result;
}

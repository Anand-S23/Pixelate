#include "linked_list.h"

internal node *CreateNode(pixel *data)
{
    node *ret_node = (node *)malloc(sizeof(node));
    ret_node->buffer = data;
    ret_node->next = NULL;
    ret_node->prev = NULL;
    return ret_node;
}

internal linked_list CreateLinkedList()
{
    linked_list ret_list = {0};
    ret_list.head = NULL;
    ret_list.tail = NULL;
    ret_list.size = 0;
    return ret_list;
}

internal node *Push(linked_list *ll, pixel *data)
{
    node *new_node = CreateNode(data);

    if (ll->head == NULL)
    {
        ll->head = new_node;
    }
    else if (ll->tail == NULL)
    {
        ll->head->prev = new_node;
        new_node->next = ll->head;
        ll->tail = ll->head;
        ll->head = new_node;
    }
    else
    {
        ll->head->prev = new_node;
        new_node->next = ll->head;
        ll->head = new_node;
    }

    ++ll->size;

    return new_node;
}

internal node *Append(linked_list *ll, pixel *data)
{
    node *new_node = CreateNode(data);

    if (ll->head == NULL)
    {
        ll->head = new_node;
    }
    else if (ll->tail == NULL)
    {
        ll->head->next = new_node;
        new_node->prev = ll->head;
        ll->tail = new_node;
    }
    else
    {
        ll->tail->next = new_node;
        new_node->prev = ll->tail;
        ll->tail = new_node;
    }

    ++ll->size;
    return new_node;
}

internal pixel Pop(linked_list *ll)
{
    pixel pop_val = *ll->head->buffer;
    ll->head = ll->head->next;
    ll->head->prev = NULL;
    --ll->size;
    return pop_val;
}

internal pixel PopLast(linked_list *ll)
{
    pixel pop_val = *ll->tail->buffer;
    ll->tail = ll->tail->prev;
    ll->tail->next = NULL;
    --ll->size;
    return pop_val;
}

internal pixel Remove(linked_list *ll, int index)
{
    if (index < ll->size && index >= 0)
    {
        if (index == 0)
        {
            return Pop(ll);
        }
        else if (index == ll->size - 1)
        {
            return PopLast(ll);
        }
        else
        {
            node *current = ll->head;

            for (int i = 0; i < index; ++i)
            {
                current = current->next;
            }

            node *temp = current->next;
            pixel result = *temp->buffer;
            current->next->next->prev = current;
            current->next = current->next->next;
            free(temp);

            return result;
        }
    }
}

internal pixel *Get(linked_list *ll, int index)
{
    pixel *result = NULL;

    if (index >= 0 && index < ll->size)
    {
        node *current = ll->head;
        for (int i = 0; i < index; ++i)
        {
            current = current->next;
        }

        result = current->buffer;
    }

    return result;
}
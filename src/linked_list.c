#include "linked_list.h"

internal node* CreateNode(app_state *state, pixel_buffer data)
{
    node *ret_node = (node *)malloc(sizeof(node));
    ret_node->data = data; 
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
}

internal void push(app_state *state, linked_list *ll, pixel_buffer data)
{
    if (ll->head == NULL)
    {
        ll->head = CreateNode(state, data);
        ll->tail.prev = ll->head;
        ll->head.next = ll->tail;
    }
    else
    {
        node *node = CreateNode(state, data);
        node *temp = ll->head; 
        ll->head = node; 
        ll->head->next = temp;
        temp->prev = ll->head;
    }

    ++ll->size;
}

internal void append(app_state *state, linked_list *ll, pixel_buffer data)
{
    node *node = Createnode(state, data);
    ll->tail->next = node; 
    ll->tail = node;
}

internal int remove(linked_list *ll, int index)
{
    node *current = ll->head; 

    if (index < ll->size && index >= 0)
    {
        if (index == 0)
        {
            pop(ll); 
        }
        else
        {
            for (int i = 0; i < index; ++i)
            {
                current = current->next; 
            }
            node *temp = current->next; 
            current->next = current->next->next; 
            free(temp);
        }
    }
}
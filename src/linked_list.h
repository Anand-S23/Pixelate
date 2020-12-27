#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node
{
    u32 *buffer;
    struct node* next;
    struct node* prev;
} node;

typedef struct linked_list
{
    node *head;
    node *tail;
    int size;
} linked_list;

internal node *CreateNode(u32 *data);
internal linked_list CreateLinkedList();
internal node *Push(linked_list *ll, u32 *data);
internal node *Append(linked_list *ll, u32 *data);
internal u32 Pop(linked_list *ll);
internal u32 PopLast(linked_list *ll);
internal u32 Remove(linked_list *ll, int index);
internal u32 *Get(linked_list *ll, int index);

#endif
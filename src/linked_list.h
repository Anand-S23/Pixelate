#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node
{
    pixel *buffer;
    struct node* next;
    struct node* prev;
} node;

typedef struct linked_list
{
    node *head;
    node *tail;
    int size;
} linked_list;

internal node *CreateNode(pixel *data);
internal linked_list CreateLinkedList();
internal node *Push(linked_list *ll, pixel *data);
internal node *Append(linked_list *ll, pixel *data);
internal pixel Pop(linked_list *ll);
internal pixel PopLast(linked_list *ll);
internal pixel Remove(linked_list *ll, int index);
internal pixel *Get(linked_list *ll, int index);

#endif
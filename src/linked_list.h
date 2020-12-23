#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node
{
    pixel_buffer data;
    struct node* next;
    struct node* prev;
} node;

typedef struct linked_list
{
    node *head;
    node *tail;
    int size;
} linked_list;

#endif
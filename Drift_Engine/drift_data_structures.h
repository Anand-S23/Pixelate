#ifndef DRIFT_DATA_STRUCTURES_H
#define DRIFT_DATA_STRUCTURES_H

typedef struct node
{
    struct node *next;
    void *data;
} node;

typedef struct linked_list
{
    node *head;
    node *tail;
    int length;
} linked_list;

typedef linked_list stack;
typedef linked_list queue;

#endif

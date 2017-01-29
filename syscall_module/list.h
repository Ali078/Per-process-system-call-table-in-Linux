#ifndef _SYSCALL_TBL_LIST_H
#define _SYSCALL_TBL_LIST_H

#include <linux/list.h>

#define XNODE_DATA(node, type) ((type)node->data)
#define XNODE_LIST(node) (&((node)->entry))

struct node {
        struct list_head entry;
        void * data;
} typedef node_t;

void add_node(node_t * node, node_t * head, long (*fn)(node_t *, node_t *));
void add_node_end(node_t * node, node_t * head);
void remove_node(node_t * node);
node_t * get_node(node_t * find, node_t * head, long (*fn)(node_t *, node_t *));

#endif

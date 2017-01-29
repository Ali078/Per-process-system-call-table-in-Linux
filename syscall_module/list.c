#include "list.h"

void add_node(node_t * node, node_t * head, long (*fn)(node_t *, node_t *)) {
	struct list_head * pos = NULL;
	INIT_LIST_HEAD(XNODE_LIST(node));
	list_for_each(pos, XNODE_LIST(head)) {
		if(fn(node, list_entry(pos, node_t, entry)) >= 1) break;
	}
	list_add_tail(XNODE_LIST(node), pos);
}

void add_node_end(node_t * node, node_t * head) {
	INIT_LIST_HEAD(XNODE_LIST(node));
	list_add_tail(XNODE_LIST(node), XNODE_LIST(head));
}

void remove_node(node_t * node) {
	list_del(XNODE_LIST(node));
}

node_t * get_node(node_t * find, node_t * head, long (*fn)(node_t *, node_t *)) {
	struct list_head * pos = NULL;
	node_t * node = NULL;
	list_for_each(pos, XNODE_LIST(head)) {
		node = list_entry(pos, node_t, entry);
		if(fn(node, find) == 0) return node;
        }
	return NULL;
}



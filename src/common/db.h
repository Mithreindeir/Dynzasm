#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KVAL 2
#define MAX_CHILD (KVAL+1)
#define MAX_KEYS (KVAL)
#define CMP(k1, k2) (db_key_compare(k1, k2))
#define CPY(k1) (db_key_copy(k1))

/*B+ Tree database*/
struct db_node {
	struct db_key *keys;
	int num_keys;
	struct db_node **child;
	int num_child;
	int leaf;
	struct db_node *next;
	struct db_node *parent;
};

struct db_key {
	size_t ksize, psize;
	void * key, *ptr;
};


struct db_node *db_node_init(struct db_node *parent);
void db_node_destroy(struct db_node *node);

void db_node_print(struct db_node *root, int indent);

void *db_lookup_value(struct db_node *root, struct db_key k);
struct db_node *db_lookup(struct db_node *root, struct db_key key);
struct db_node *db_insert(struct db_node *root, struct db_key key);
struct db_node *db_insert_internal(struct db_node *root, struct db_key key);
struct db_node *db_delete(struct db_node *root, struct db_key key);
struct db_node *db_delete_internal(struct db_node *root, struct db_key key);

void db_node_merge(struct db_node *n1, struct db_node *n2);
struct db_key db_node_split(struct db_node *node, struct db_node *left, struct db_node *right);
struct db_node *db_get_last_sibling(struct db_node *child);
struct db_node *db_get_first_sibling(struct db_node *child);
void db_update_key(struct db_node *c1, struct db_node *c2);
struct db_key db_get_shared_key(struct db_node *s1, struct db_node *s2);
struct db_key db_get_max_key(struct db_node *node);
struct db_key db_get_min_key(struct db_node *node);

void db_addkey(struct db_node *node, struct db_key key);
void db_delkey(struct db_node *node, struct db_key key);
void db_remkey(struct db_node *node, struct db_key key);

void db_addchild(struct db_node *node, struct db_node *child);
void db_delchild(struct db_node *node, struct db_node *child);

struct db_key db_key_copy(struct db_key key);
int db_key_compare(struct db_key k1, struct db_key k2);
void db_key_print(struct db_key k);
int db_getleaves(struct db_node *root, struct db_node **list);

#endif

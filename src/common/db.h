#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_RECORD
#define DB_LEAF 2
#define DB_BRANCH 1

#define KVAL 4
#define MAX_CHILD (KVAL+1)
#define MAX_KEYS (KVAL)

/*B+ Tree database*/
struct db_node {
	char ** keys;
	int num_keys;

	struct db_node **child;
	int num_child;
	int leaf;
	struct db_node *parent;
};

struct db_node *db_node_init(struct db_node *parent);
void db_node_destroy(struct db_node *node);

void db_node_print(struct db_node *root, int indent);

struct db_node *db_lookup(struct db_node *root, char * key);
struct db_node *db_insert(struct db_node *root, char * key);
struct db_node *db_insert_internal(struct db_node *root, char * key);

char * db_node_split(struct db_node *node, struct db_node *left, struct db_node *right);
void db_addchild(struct db_node *node, struct db_node *child, int cidx);
void db_addkey(struct db_node *node, char * key);
void db_remchild(struct db_node *node, struct db_node *child);

char *strdup(char *str);
#endif

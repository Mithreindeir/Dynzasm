#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*String Database
 * B+ Tree*/

#define K_SIZE 3

struct zdb_node {
	struct zdb_node **child;
	int num_children;

	struct zdb_keypair ** elem;
	int num_elem;

	struct zdb_node *parent;
};

struct zdb_keypair {
	unsigned char key;
	void *value;
};

struct zdb_node *zdb_node_init();
void zdb_node_destroy(struct zdb_node *zdb);

struct zdb_node *zdb_insert(struct zdb_node *root, struct zdb_keypair *kp);

void zdb_splitchild(struct zdb_node *parent, int cidx);
void zdb_addchild(struct zdb_node *parent, struct zdb_node *child);
void zdb_addpair(struct zdb_node *node, struct zdb_keypair *kp);
void zdb_remchild(struct zdb_node *parent, int cidx);
void zdb_rempair(struct zdb_node *parent, int idx);
void zdb_delete(struct zdb_node *root, struct zdb_node *onode);


#endif

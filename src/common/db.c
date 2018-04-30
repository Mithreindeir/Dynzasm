#include "db.h"

struct zdb_node *zdb_node_init(struct zdb_node *parent)
{
	struct zdb_node *node = malloc(sizeof(struct zdb_node));

	node->num_elem = 0, node->num_children = 0;
	node->elem = NULL, node->child = NULL;
	node->parent = parent;

	return node;
}

void zdb_node_destroy(struct zdb_node *zdb)
{
	if (!zdb) return;

	for (int i = 0; i < zdb->num_children; i++)
			zdb_node_destroy(zdb->child[i]);

	free(zdb->child);
	free(zdb->elem);
	free(zdb);
}

/*Tree Lookup*/
struct zdb_node *zdb_lookup(struct zdb_node *root, unsigned long key)
{
	if (!root->num_children || root->num_children < root->num_elem)
		return root;
	for (int i = 0; i < root->num_elem; i++) {
		if (key < root->elem[i]->key)
			return zdb_lookup(root->child[i], key);
	}
	return zdb_lookup(root->child[root->num_elem], key);
}

/*B+ Tree insertion. Returns new root or old root*/
struct zdb_node *zdb_insert(struct zdb_node *root, struct zdb_keypair *kp)
{
	if (root->num_elem < K_SIZE) {
		zdb_addpair(root, kp);
	} else {
		struct zdb_node *oldroot = root;
		int median = root->num_elem / 2;
		zdb_splitchild(root, median);
		root = zdb_node_init(oldroot->parent);

	}
	return root;
}

/*Splits a nodes child into 2 children */
void zdb_splitchild(struct zdb_node *parent, int cidx)
{
	if (!parent || cidx < 0 || cidx >= parent->num_elem)
		return;
	struct zdb_node *left = zdb_node_init(parent);
	struct zdb_node *right= zdb_node_init(parent);

	/*Add all children with keys less than median to left node and the reset to right*/
	unsigned char mkey = parent->elem[cidx]->key;
	for (int i = 0; i < parent->num_elem; i++) {
		if (parent->elem[i]->key < mkey) {
			zdb_addpair(left, parent->elem[i]);
			for (int j = 0; j < (i+1); j++) {
				zdb_addchild(left, parent->child[j]);
			}
		} else if (parent->elem[i]->key > mkey) {
			zdb_addpair(right, parent->elem[i]);
			for (int j = 0; j < (i+1); j++) {
				zdb_addchild(right, parent->child[j]);
			}
		} else {
			continue;
		}
		/*Remove all used elements and chidren from the parent*/
		zdb_rempair(parent, i);
		for (int j = 0; j < (i+1); j++)
			zdb_remchild(parent, j);
	}
	zdb_addchild(parent, left);
	zdb_addchild(parent, right);
}

/*Insertion sort of keypair into node*/
void zdb_addpair(struct zdb_node *node, struct zdb_keypair *kp)
{
	int idx = 0;
	while ((idx < node->num_elem) && (kp->key >= node->elem[idx]->key)) idx++;

	node->num_elem++;
	if (!node->elem)
		node->elem = malloc(sizeof(struct zdb_keypair*));
	else
		node->elem = realloc(node->elem, sizeof(struct zdb_keypair*)*node->num_elem);

	int size = (node->num_elem-(idx+1))*sizeof(struct zdb_keypair*);
	if (idx > 1 && idx < (node->num_elem-1)) {
		memmove(node->elem+idx+1, node->elem+idx, size);
	}
	node->elem[idx] = kp;
}

void zdb_rempair(struct zdb_node *node, int idx) {
	if (!node || (idx < 0 || idx >= node->num_elem))
			return;
	node->num_elem--;
	node->elem = realloc(node->elem, sizeof(struct zdb_keypair*)*node->num_elem);
	int size = (node->num_elem-idx) * sizeof(struct zdb_keypair*);
	memmove(node->elem+idx,node->elem+idx+1, size);
}

void zdb_addchild(struct zdb_node *node, struct zdb_node *child)
{
	node->num_children++;
	if (!node->child)
		node->child = malloc(sizeof(struct zdb_node*));
	else
		node->child = realloc(node->child, sizeof(struct zdb_node*)*node->num_children);
	node->child[node->num_children-1] = child;
}

void zdb_remchild(struct zdb_node *node, int cidx)
{
	if (!node || (cidx < 0 || cidx >= node->num_children))
			return;
	node->num_children--;
	node->child = realloc(node->child, sizeof(struct zdb_node*)*node->num_children);
	int size = (node->num_children-cidx) * sizeof(struct zdb_node*);
	memmove(node->child+cidx,node->child+cidx+1, size);
}

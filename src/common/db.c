#include "db.h"

struct db_node *db_node_init(struct db_node *parent)
{
	struct db_node *node = malloc(sizeof(struct db_node));

	node->keys = NULL, node->child = NULL;
	node->num_keys = 0, node->num_child = 0;
	node->leaf = 1;
	node->parent = parent;

	return node;
}

void db_node_destroy(struct db_node *node)
{
	if (!node) return;

	for (int i = 0; i < node->num_child; i++) {
		db_node_destroy(node->child[i]);
	}
	for (int i = 0; i < node->num_keys; i++) {
		free(node->keys[i]);
	}
	free(node->child);
	free(node->keys);
	free(node);
}

void db_node_print(struct db_node *root, int indent)
{
	if (!root) return;
	for (int i = 0; i < indent; i++)
		printf("\t");
	for (int i = 0; i < root->num_keys; i++) {
		printf("%s, ", root->keys[i]);
	}
	printf("\n");
	for (int i = 0; i < root->num_child; i++) {
		db_node_print(root->child[i], indent+1);
		printf("\n");
	}
}


struct db_node *db_lookup(struct db_node *root, char * key)
{
	if (!root) return NULL;
	struct db_node *node = root;
	int found = 0;
	while (node && !node->leaf) {
		found = 0;
		for (int i = 0; i < node->num_keys; i++) {
			if (strcmp(key, node->keys[i]) < 0) {
				node = node->child[i];
				found = 1;
				break;
			} else if (!strcmp(key, node->keys[i]))
				return node;
		}
		if (!found)
			node = node->child[node->num_child-1];
	}
	return node;
}

struct db_node *db_insert(struct db_node *root, char * key)
{
	struct db_node * closest = db_lookup(root, key);
	struct db_node *nroot = root;
	if (closest)
		nroot = db_insert_internal(closest, strdup(key));
	return !nroot->parent ? nroot : root;
}

struct db_node *db_insert_internal(struct db_node *closest, char * key)
{
	if (closest->num_keys < MAX_KEYS) {
		db_addkey(closest, key);
		return closest;
	} else if (closest->parent && closest->parent->num_keys < MAX_KEYS) {
		db_addkey(closest, key);
		struct db_node *left = db_node_init(NULL);
		struct db_node *right = db_node_init(NULL);
		struct db_node *parent = closest->parent;
		db_remchild(parent, closest);
		key = db_node_split(closest, left, right);
		db_insert_internal(parent, key);
		int idx = -1;
		for (int i = 0; i < parent->num_keys; i++) {
			if (!strcmp(parent->keys[i], key))
				idx = i;
		}
		if (idx >= 0) {
				db_addchild(parent, left, idx);
				db_addchild(parent, right, idx+1);
		} else printf("fail2\n");
		return parent;
	} else if (!closest->parent) {
		db_addkey(closest, key);
		struct db_node *left = db_node_init(NULL);
		struct db_node *right = db_node_init(NULL);
		key = db_node_split(closest, left, right);
		struct db_node *newroot = db_node_init(NULL);
		db_addkey(newroot, key);
		db_addchild(newroot, left, 0);
		db_addchild(newroot, right, 1);
		return newroot;
	} else {
		db_addkey(closest, key);
		struct db_node *left = db_node_init(NULL);
		struct db_node *right = db_node_init(NULL);
		struct db_node *parent = closest->parent;
		db_remchild(parent, closest);
		key = db_node_split(closest, left, right);
		parent = db_insert_internal(parent, key);
		struct db_node *lk = db_lookup(parent, key);
		struct db_node * ret=parent;
		parent = lk;
		int idx = -1;
		for (int i = 0; i < parent->num_keys; i++) {
			if (!strcmp(parent->keys[i], key))
				idx = i;
		}
		if (idx >= 0) {
				db_addchild(parent, left, idx);
				db_addchild(parent, right, idx+1);
		}
		return ret;
	}
	return closest;
}

char * db_node_split(struct db_node *node, struct db_node *left, struct db_node *right)
{
	int median = node->num_keys/2;
	int high = node->num_keys;
	char * * okeys = node->keys;
	char * mkey = okeys[median];
	struct db_node ** ochild = node->child;
	int nochild = node->num_child;

	int wasleaf = node->leaf;
	node->child = NULL;
	node->num_child = 0;
	node->keys = NULL;
	node->num_keys = 0;
	db_node_destroy(node);

	for (int i = 0; i < high; i++) {
		if (i > median)
			db_addkey(right, okeys[i]);
		else if (i < median)
			db_addkey(left, okeys[i]);
	}
	if (wasleaf)
		db_addkey(right, strdup(okeys[median]));
	for (int i = 0; i < nochild; i++) {
		if (i > median)
			db_addchild(right, ochild[i], right->num_child);
		else if (i <= median)
			db_addchild(left, ochild[i], left->num_child);
	}

	free(okeys);
	free(ochild);

	return mkey;
}

void db_addchild(struct db_node *node, struct db_node *child, int cidx)
{
	if (cidx > node->num_child)
		return;
	node->leaf = 0;
	child->parent = node;
	node->num_child++;
	if (!node->child)
		node->child=malloc(sizeof(struct db_node*));
	else
		node->child=realloc(node->child, sizeof(struct db_node*)*node->num_child);
	int size = (node->num_child - (cidx+1))*sizeof(struct db_node*);
	if (cidx < (node->num_child-1))
		memmove(node->child+cidx+1, node->child+cidx, size);
	node->child[cidx] = child;
}

void db_remchild(struct db_node *node, struct db_node *child)
{
	if (!node) return;
	int idx = -1;
	for (int i = 0; i < node->num_child; i++) {
		if (node->child[i] == child) {
			idx = i;
			break;
		}
	}
	if (idx < 0) return;
	int size = ((node->num_child-1)-idx) * sizeof(struct db_node*);
	if (idx < node->num_child)
		memmove(node->child+idx,node->child+idx+1, size);
	node->num_child--;
	if (!node->num_child) {
		node->leaf = 1;
		free(node->child);
		node->child = NULL;
	} else {
		node->child = realloc(node->child, sizeof(struct db_node*)*node->num_child);
	}
}

void db_addkey(struct db_node *node, char * key)
{
	int idx = 0;
	while ((idx < node->num_keys) && (strcmp(key,node->keys[idx]) > 0)) idx++;
	if (idx < node->num_keys && node->keys[idx] == key) return;
	node->num_keys++;
	if (!node->keys)
		node->keys=malloc(sizeof(char *));
	else
		node->keys=realloc(node->keys, sizeof(char *)*node->num_keys);
	int size = (node->num_keys - (idx+1))*sizeof(char *);
	if (idx < (node->num_keys-1))
		memmove(node->keys+idx+1, node->keys+idx, size);
	node->keys[idx] = key;
}

char *strdup(char *str)
{
	int len = strlen(str);
	char *cpy = malloc(len+1);
	strncpy(cpy, str, len);
	cpy[len] = 0;
	return cpy;
}

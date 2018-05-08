#include "db.h"

struct db_node *db_node_init(struct db_node *parent)
{
	struct db_node *node = malloc(sizeof(struct db_node));

	node->next = NULL;
	node->leaf = 1;
	node->parent = parent;
	node->keys = NULL, node->child = NULL;
	node->num_keys = 0, node->num_child = 0;

	return node;
}

void db_node_destroy(struct db_node *node)
{
	if (!node) return;

	for (int i = 0; i < node->num_child; i++) {
		db_node_destroy(node->child[i]);
	}
	for (int i = 0; i < node->num_keys; i++) {
		free(node->keys[i].key);
		free(node->keys[i].ptr);
	}
	free(node->child);
	free(node->keys);
	free(node);
}

void db_node_print(struct db_node *root, int indent)
{
	if (!root) return;
	if (root->num_child)
		db_node_print(root->child[0], indent+1);
	for (int i = 0; i < root->num_keys; i++) {
		for (int j = 0; j < indent; j++)
			printf("\t");
		printf("(");
		db_key_print(root->keys[i]);
		printf(")\n");
		if ((i+1) < root->num_child)
			db_node_print(root->child[i+1], indent+1);
	}
	/*
	if (!root) return;
	for (int i = 0; i < indent; i++)
		printf("\t");
	for (int i = 0; i < root->num_keys; i++) {
		printf("(");
		db_key_print(root->keys[i]);
		printf("), ");
	}
	printf("\n");
	for (int i = 0; i < root->num_child; i++) {
		db_node_print(root->child[i], indent+1);
		printf("\n");
	}*/
}

void db_key_print(struct db_key k)
{
	if (k.ptr)
		printf("(%s)", (char*)k.ptr);
	printf("%.*s", (int)k.ksize, (char*)k.key);
	//printf("%d", *(int*)k.key);
	return;
	int ks = k.ksize;
	for (int j = 0; j < ks; j++)
		printf("%c", ((char*)k.key)[j]);
}

void *db_lookup_value(struct db_node *node, struct db_key key)
{
	if (node->leaf) {
		for (int i = 0; i < node->num_keys; i++) {
			if (!CMP(key, node->keys[i])) {
				return node->keys[i].ptr;
			}
		}
		return NULL;
	}
	for (int i = 0; i < node->num_keys; i++) {
		if (CMP(key, node->keys[i]) < 0)
			return db_lookup_value(node->child[i], key);
	}
	return db_lookup_value(node->child[node->num_child-1], key);
}

struct db_node *db_lookup(struct db_node *node, struct db_key key)
{
	if (node->leaf) return node;
	for (int i = 0; i < node->num_keys; i++) {
		if (CMP(key, node->keys[i]) < 0)
			return db_lookup(node->child[i], key);
	}
	return db_lookup(node->child[node->num_child-1], key);
}

struct db_node *db_delete(struct db_node *root, struct db_key key)
{
	struct db_node *close = db_lookup(root, key);
	struct db_node *nroot = root;
	if (close)
		nroot = db_delete_internal(close, key);
	return !nroot->parent ? nroot : root;
}

struct db_node *db_delete_internal(struct db_node *closest, struct db_key key)
{
	if ((closest->num_keys-1) >= (MAX_KEYS/2)) {
		db_delkey(closest, key);
		/*If parent update sibling keys*/
		if (closest->parent) {
			db_update_key(db_get_first_sibling(closest), closest);
			db_update_key(closest, db_get_last_sibling(closest));
		}
		return closest;
	} else if (closest->parent) {
		db_delkey(closest, key);
		struct db_node *s1 = db_get_first_sibling(closest);
		struct db_node *s2 = db_get_last_sibling(closest);
		int dis = 0;
		if (s1 && s1->num_keys > (MAX_KEYS/2)) {
			db_addkey(closest, s1->keys[s1->num_keys-1]);
			db_remkey(s1, s1->keys[s1->num_keys-1]);
			if (s1->num_child) {
				db_addchild(closest, s1->child[s1->num_child-1]);
				db_delchild(s1, s1->child[s1->num_child-1]);
			}
			db_update_key(s1, closest);
			dis = 1;
		} else if (s2 && s2->num_keys > (MAX_KEYS/2)) {
			db_addkey(closest, s2->keys[0]);
			db_remkey(s2, s2->keys[0]);
			if (s2->num_child) {
				db_addchild(closest, s2->child[0]);
				db_delchild(s2, s2->child[0]);
			}
			db_update_key(closest, s2);
			dis = 1;
		}
		if (dis) {
			for (int i = 0; (i < closest->num_keys) && (i < closest->num_child-1); i++) {
				free(closest->keys[i].key);
				closest->keys[i] = CPY(db_get_max_key(closest->child[i]));
			}
		}
		/*If that fails then merge with sibling and delete shared key*/
		if (!dis && (s1 || s2)) {
			struct db_key k;
			if (s1) {
				k = db_get_shared_key(s1, closest);
				db_node_merge(closest, s1);
			} else if (s2) {
				k = db_get_shared_key(closest, s2);
				db_node_merge(closest, s2);
			}
			if (!closest->leaf) {
				db_addkey(closest, CPY(k));
			}
			struct db_node *par = closest->parent;
			return db_delete_internal(par, k);
		}
	} else {
		if (closest->num_child == 1) {
			struct db_node *c = closest->child[0];
			closest->child[0] = NULL;
			c->parent = NULL;
			db_node_destroy(closest);
			return c;
		}
		db_delkey(closest, key);
		return closest;
	}
	return closest;
}

struct db_key db_get_max_key(struct db_node *node)
{
	if (node->leaf) return node->keys[node->num_keys-1];
	return db_get_max_key(node->child[node->num_child-1]);
}

struct db_key db_get_min_key(struct db_node *node)
{
	if (node->leaf) return node->keys[0];
	return db_get_max_key(node->child[0]);
}

struct db_key db_get_shared_key(struct db_node *s1, struct db_node *s2)
{
	struct db_key k;
	k.key = NULL;
	k.ksize = 0;
	k.ptr = NULL;
	if (!s1 || !s2 || (s1->parent != s2->parent))
			return  k;
	struct db_node *parent = s1->parent;
	/*Find shared key and delete it*/
	for (int i = 0; i < parent->num_keys; i++) {
		if (parent->child[i]==s1 && parent->child[i+1]==s2) {
			return parent->keys[i];
		}
	}
	return k;
}

void db_node_merge(struct db_node *n1, struct db_node *n2)
{
	for (int i = 0; i < n2->num_keys; i++) {
		db_addkey(n1, n2->keys[i]);
	}
	for (int i = 0; i < n2->num_child; i++) {
		db_addchild(n1, n2->child[i]);
	}
	free(n2->keys);
	free(n2->child);
	n2->num_child = 0, n2->num_keys = 0;
	n2->keys = NULL, n2->child = NULL;
	db_delchild(n2->parent, n2);
	db_node_destroy(n2);
}

void db_update_key(struct db_node *c1, struct db_node *c2)
{
	if (!c1 || !c2 || (c1->parent != c2->parent))
			return;
	struct db_node *parent = c1->parent;
	if (!parent) return;
	for (int i = 0; i < parent->num_keys; i++) {
		if (parent->child[i] == c1 && parent->child[i+1] == c2) {
			free(parent->keys[i].key);
			//parent->keys[i] = CPY(c2->keys[0]);
			parent->keys[i] = CPY(db_get_min_key(c2));
			return;
		}
	}
}

struct db_node *db_get_first_sibling(struct db_node *child)
{
	struct db_node *p = child->parent;
	if (!p) return NULL;
	for (int i = 1; i < p->num_child; i++) {
		if (p->child[i]==child)
			return p->child[i-1];
	}
	return NULL;
}

struct db_node *db_get_last_sibling(struct db_node *child)
{
	struct db_node *p = child->parent;
	if (!p) return NULL;
	for (int i = 0; i < p->num_child-1; i++) {
		if (p->child[i]==child)
			return p->child[i+1];
	}
	return NULL;
}

struct db_node *db_insert(struct db_node *root, struct db_key key)
{
	struct db_node * closest = db_lookup(root, key);
	struct db_node *nroot = root;
	struct db_key cpy = CPY(key);
	cpy.ptr = key.ptr, cpy.psize = key.psize;
	if (closest)
		nroot = db_insert_internal(closest, cpy);
	return !nroot->parent ? nroot : root;
}

struct db_node *db_insert_internal(struct db_node *closest, struct db_key key)
{
	if (closest->num_keys < MAX_KEYS) {
		db_addkey(closest, key);
		return closest;
	} else if (!closest->parent) {
		db_addkey(closest, key);
		struct db_node *left = db_node_init(NULL);
		struct db_node *right = db_node_init(NULL);
		key = db_node_split(closest, left, right);
		struct db_node *newroot = db_node_init(NULL);
		db_addkey(newroot, key);
		db_addchild(newroot, left);
		db_addchild(newroot, right);
		return newroot;
	} else {
		db_addkey(closest, key);
		struct db_node *left = db_node_init(NULL);
		struct db_node *right = db_node_init(NULL);
		struct db_node *parent = closest->parent;
		db_delchild(parent, closest);
		key = db_node_split(closest, left, right);
		db_addchild(parent, left);
		db_addchild(parent, right);
		return db_insert_internal(parent, key);
	}
	return closest;
}

struct db_key db_node_split(struct db_node *node, struct db_node *left, struct db_node *right)
{
	int median = node->num_keys/2;
	int high = node->num_keys;
	struct db_key * okeys = node->keys;
	struct db_key mkey = okeys[median];
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
		db_addkey(right, okeys[median]);
	for (int i = 0; i < nochild; i++) {
		if (i > median)
			db_addchild(right, ochild[i]);
		else if (i <= median)
			db_addchild(left, ochild[i]);
	}/*
	if (wasleaf) {
		left->next = right;
	} else {
		struct db_node *rc=left,*lc=right;
		while (rc && !rc->leaf && rc->num_child) rc=rc->child[rc->num_child-1];
		while (lc && !lc->leaf && lc->num_child) lc=lc->child[0];
		rc->next = lc;
		printf("\n");
		db_node_print(lc, 0);
		db_node_print(rc, 0);
		printf("\n");
	}*/

	free(okeys);
	free(ochild);

	return wasleaf ? CPY(mkey) : mkey;
}

int db_getleaves(struct db_node *node, struct db_node **list)
{
	int len = 0;
	for (int i = 0; i < node->num_child; i++) {
		len += db_getleaves(node->child[i], list);
	}

	if (node->leaf) {
		if (!(*list)) {
			*list = node;
		} else {
			struct db_node *cur = *list;
			while (cur->next) cur = cur->next;
			cur->next = node;
		}
		len+= node->num_keys;
	}
	return len;
}

void db_addchild(struct db_node *node, struct db_node *child)
{
	int idx = 0;
	while ((idx < node->num_child) && (CMP(child->keys[0], node->child[idx]->keys[0]) > 0)) idx++;
	node->leaf = 0;
	child->parent = node;
	node->num_child++;
	if (!node->child)
		node->child = malloc(sizeof(struct db_node*));
	else
		node->child = realloc(node->child, sizeof(struct db_node*)*node->num_child);
	int size = (node->num_child - (idx+1))*sizeof(struct db_node*);
	if (idx < (node->num_child-1))
		memmove(node->child+idx+1, node->child+idx, size);
	node->child[idx] = child;
}

void db_delchild(struct db_node *node, struct db_node *child)
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

void db_addkey(struct db_node *node, struct db_key key)
{
	int idx = 0;
	while ((idx < node->num_keys) && (CMP(key, node->keys[idx]) >= 0)) idx++;
	node->num_keys++;
	if (!node->keys)
		node->keys=malloc(sizeof(struct db_key));
	else
		node->keys=realloc(node->keys, sizeof(struct db_key)*node->num_keys);
	int size = (node->num_keys - (idx+1))*sizeof(struct db_key);
	if (idx < (node->num_keys-1))
		memmove(node->keys+idx+1, node->keys+idx, size);
	node->keys[idx] = key;
}

void db_delkey(struct db_node *node, struct db_key key)
{
	if (!node) return;
	int idx = -1;
	for (int i = 0; i < node->num_keys; i++) {
		if (!CMP(key, node->keys[i])) {
			idx = i;
			break;
		}
	}
	if (idx < 0) return;
	free(node->keys[idx].key);
	free(node->keys[idx].ptr);
	int size = ((node->num_keys-1)-idx) * sizeof(struct db_key);
	if (idx < node->num_keys)
		memmove(node->keys+idx,node->keys+idx+1, size);
	node->num_keys--;
	if (!node->num_keys) {
		free(node->keys);
		node->keys = NULL;
	} else {
		node->keys = realloc(node->keys, sizeof(struct db_key)*node->num_keys);
	}
}

void db_remkey(struct db_node *node, struct db_key key)
{
	if (!node) return;
	int idx = -1;
	for (int i = 0; i < node->num_keys; i++) {
		if (!CMP(key, node->keys[i])) {
			idx = i;
			break;
		}
	}
	if (idx < 0) return;
	//free(node->keys[idx].key);
	//free(node->keys[idx].ptr);
	int size = ((node->num_keys-1)-idx) * sizeof(struct db_key);
	if (idx < node->num_keys)
		memmove(node->keys+idx,node->keys+idx+1, size);
	node->num_keys--;
	if (!node->num_keys) {
		free(node->keys);
		node->keys = NULL;
	} else {
		node->keys = realloc(node->keys, sizeof(struct db_key)*node->num_keys);
	}
}

struct db_key db_key_copy(struct db_key key)
{
	struct db_key cpy;
	cpy.key = malloc(key.ksize);
	cpy.ksize = key.ksize;
	cpy.ptr = NULL;
	cpy.psize = 0;
	memcpy(cpy.key, key.key, key.ksize);
	return cpy;
}

int db_key_compare(struct db_key k1, struct db_key k2)
{
	int result = 0;
	int len = k1.ksize < k2.ksize ? k1.ksize : k2.ksize;
	int diff = 0;
	for (int i = 0; i < len; i++) {
		diff = ((unsigned char*)k1.key)[i] - ((unsigned char*)k2.key)[i];
		if (diff != 0) {
			result = diff;
			break;
		}
	}
	if (result == 0)
		result = k1.ksize > k2.ksize ? 1 : k1.ksize == k2.ksize ? 0 : -1;
	return result;
}

#include "trie.h"

struct trie_node *trie_init(unsigned char key, void *value)
{
	struct trie_node *node = malloc(sizeof(struct trie_node));

	node->dist = 0;
	node->key = key;
	node->value = value;
	node->children = NULL;
	node->num_children = 0;
	node->flags = 0;

	return node;
}

void trie_destroy(struct trie_node *node)
{
	if (!node) return;

	for (int i = 0; i < node->num_children; i++) {
		trie_destroy(node->children[i]);
	}

	free(node->children);
	free(node->value);
	free(node);
}

struct trie_node *trie_lookup(struct trie_node *node, unsigned char *stream, long max)
{
	for (int i = 0; i < node->num_children; i++) {
		if (node->children[i]->key == *stream) {
			node = node->children[i];
			if (!(--max)) return node;
			stream++;
			i = -1;
		}
	}
	return node;
}

void trie_insert(struct trie_node *root, unsigned char *stream, long max, void *value, unsigned char flags)
{
	struct trie_node *far = root;
	while (max > 0) {
		int found = 0;
		for (int i = 0; i < far->num_children; i++) {
			if (far->children[i]->key == stream[0]) {
				found = 1;
				far = far->children[i];
				max--;
				stream++;
				break;
			}
		}
		if (!found) break;
	}

	while (max > 0) {
		far->num_children++;
		if (!far->children)
			far->children=malloc(sizeof(struct trie_node*));
		else
			far->children=realloc(far->children,sizeof(struct trie_node*)*far->num_children);
		far->children[far->num_children-1] = trie_init(stream[0], NULL);
		far->children[far->num_children-1]->dist = far->dist+1;
		far = far->children[far->num_children-1];
		stream++;
		max--;
	}
	far->value = value;
	far->flags = flags;
}

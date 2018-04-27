#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>

/*Opaque Trie structure for disassembly indexing.
 * Branch Nodes can hold flags to resolve lookup conflicts.*/
struct trie_node {
	//Trie children
	struct trie_node **children;
	int num_children;

	//Distance from the root node
	int dist;

	//Key byte
	unsigned char key;
	//Opaque pointer to hold entry
	void *value;
};

struct trie_node *trie_init(unsigned char key, void *value);
void trie_destroy(struct trie_node *node);

//Creates path and inserts a value.
void trie_insert(struct trie_node *root, unsigned char *stream, long max, void *value);
//Returns the leaf node or branch if there is a lookup conflict
struct trie_node *trie_lookup(struct trie_node *root, unsigned char *stream, long max);

#endif

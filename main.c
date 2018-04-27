#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "arch/x86.h"
#include "arch/x86load.h"
#include "include/trie.h"

int main(int argc, char ** argv)
{
	struct trie_node *root = trie_init(0, NULL);
	x86_parse(root);

	unsigned char arr[] = { 0xd0, 0x13, 0x00, 0x00, 0x00};
	clock_t ct = clock();
	struct trie_node *n = trie_lookup(root, arr, 5);
	clock_t at = clock();
	if (n && !n->value) {
		printf("fail %p %x\n", (void*)n, n->key);
		arr[1] &= 7;
		n = trie_lookup(n, arr+1, 5);
		struct x86_instr_entry *e = n->value;
		if (n->value) printf("Woah %s\n", e->mnemonic);
		else printf("test %x\n", arr[1]);
	}
	else if (n) {
		struct x86_instr_entry *e = n->value;
		printf("%s\n", e->mnemonic);
		printf("time %0.8f", (double)(at-ct)/CLOCKS_PER_SEC);
	}
	trie_destroy(root);
}

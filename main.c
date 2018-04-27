#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "arch/x86.h"
#include "arch/x86load.h"
#include "common/trie.h"

int main(int argc, char ** argv)
{
	struct trie_node *root = trie_init(0, NULL);
	x86_parse(root);
	unsigned char stream[] = {0x31, 0xc0};
	struct dis disas;
	x86_disassemble(root, stream, 2, &disas);

	trie_destroy(root);
	return 0;

	/*Example of using tries*/
	unsigned char buf[16];
	memset(buf, 0, 16);
	for (int i = 0; i <= 0xff; i++) {
		buf[0] = i;
		struct trie_node *n = trie_lookup(root, buf, 1);
		/* Handle rm extension */
		if (n &&  n->flags == 2) {
			struct trie_node *n2;
			for (int i = 0; i < 8; i++) {
				unsigned char c = i;
				n2 = trie_lookup(n, &c, 1);
				if ((n2 != n) && n2 && n2->value) {
					struct x86_instr_entry *e = n2->value;
					printf("%s\n", e->mnemonic);
				}
			}
		}
		else if (n && n->value) {
			struct x86_instr_entry *e = n->value;
			printf("%s\n", e->mnemonic);
		}
	}

	trie_destroy(root);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "arch/x86.h"
#include "arch/x86load.h"
#include "common/trie.h"

int main()
{
	struct trie_node *root = trie_init(0, NULL);
	x86_parse(root);

	char ch;
	char bbuf[64];
	unsigned char out[64];
	memset(bbuf, 0, 64);
	memset(out, 0, 64);
	int iter = 0;
	while (read(STDIN_FILENO, &ch, 1) > 0) {
		if ((ch>='a'&&ch<='f') || (ch >= 0x30 && ch <= 0x39)) {
			bbuf[iter++]=ch;
		}
	}
	long max = ascii_to_hex(out, bbuf, iter);
	long bit = 0;

	while (bit < max) {
		struct dis disas;
		int oldbit = bit;
		bit+=x86_disassemble(root, out+bit, max-bit, &disas);
		int w = 10;
		for (int i = oldbit; i < bit; i++) {
			printf("%02x ", out[i]);
			w--;
		}
		while (w) {
			printf("   ");
			w-= 1;
		}
		char buf[256];
		printf("%s\t", disas.mnemonic);
		for (int i = 0; i < disas.num_operands; i++) {
			operand_squash(buf, 256, disas.operands[i]);
			printf(" %s%c", buf, (i+1)<disas.num_operands?',':' ');
			operand_tree_free(disas.operands[i]);
		}
		free(disas.operands);
		printf("\n");
	}

	trie_destroy(root);
	return 0;
}

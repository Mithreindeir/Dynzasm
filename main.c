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
	long allocd=100;
	char *bbuf = malloc(allocd);
	int iter = 0;
	while (read(STDIN_FILENO, &ch, 1) > 0) {
		if ((iter + 1) >= allocd) {
			allocd += 100;
			bbuf = realloc(bbuf, allocd);
			memset(bbuf+iter, 0, allocd-iter-1);
		}
		if ((ch>='a'&&ch<='f') || (ch >= 0x30 && ch <= 0x39)) {
			bbuf[iter++]=ch;
		}
	}
	int maxout = iter/2 + 1;
	unsigned char *out = malloc(maxout);
	memset(out, 0, maxout);
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

	free(bbuf);
	free(out);
	trie_destroy(root);
	return 0;
}

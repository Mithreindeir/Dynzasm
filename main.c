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
	unsigned char stream[] = {0x63, 0x00, 0x5f, 0x69, 0x6e, 0x73, 0x65};
	struct dis disas;
	x86_disassemble(root, stream, 7, &disas);
	char buf[256];
	printf("%s\t", disas.mnemonic);
	for (int i = 0; i < disas.num_operands; i++) {
		operand_squash(buf, 256, disas.operands[i]);
		printf(" %s%c", buf, (i+1)<disas.num_operands?',':' ');
	}
	printf("\n");

	trie_destroy(root);
	return 0;
}

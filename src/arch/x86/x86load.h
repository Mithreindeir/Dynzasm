#ifndef X86_LOAD_H
#define X86_LOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/trie.h"

struct x86_instr_entry {
	char mnemonic[12];
	char operand[3][12];
	int num_op;
};

long ascii_to_hex(unsigned char *out, char *in, long len);
int get_line(FILE *f, char *buf, long max);
void x86_parse(struct trie_node *root);
void x86_lookup(struct trie_node *instrl, unsigned char *stream, long max);

#endif

#ifndef X86_LOAD_H
#define X86_LOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/trie.h"

struct x86_instr_entry {
	char mnemonic[12];
	char operand[3][12];
	int num_op;
};

int get_line(FILE *f, char *buf, long max);
void x86_parse(struct trie_node *root);
void x86_lookup(struct trie_node *instrl, unsigned char *stream, long max);

#endif

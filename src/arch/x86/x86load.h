#ifndef X86_LOAD_H
#define X86_LOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/trie.h"

#define MAX_MNEM_SIZE 12
#define MAX_OPER_LEN 12
#define MAX_OPERANDS 3

/*A copy of the MODE macros in disas*/
#ifndef MODE_32B
#define MODE_32B 1
#endif
#ifndef MODE_64B
#define MODE_64B 2
#endif

struct x86_instr_entry {
	char mnemonic[MAX_MNEM_SIZE];
	char operand[MAX_OPERANDS][MAX_OPER_LEN];
	int num_op;
};

long ascii_to_hex(unsigned char *out, char *in, long len);
int get_line(FILE *f, char *buf, long max);
void x86_parse(struct trie_node *root, int mode);
void x86_lookup(struct trie_node *instrl, unsigned char *stream, long max);

#endif

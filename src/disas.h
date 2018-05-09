#ifndef DISAS_H
#define DISAS_H

/*Disassembler*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "arch/x86/x86load.h"
#include "arch/x86/x86.h"
#include "dis.h"
#include "common/trie.h"

#define X64_ARCH 1

struct disassembler {
	int arch;

	struct dis **instr;
	int num_instr;

	struct trie_node *root;
};

struct disassembler *ds_init(int isa);

void ds_decode(struct disassembler *ds, unsigned char *stream, int size, uint64_t entry);

void ds_destroy(struct disassembler *ds);

#endif

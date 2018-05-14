#ifndef DISAS_H
#define DISAS_H

/*Disassembler*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "arch/x86/x86load.h"
#include "arch/x86/x86.h"
#include "arch/mips/mload.h"
#include "arch/mips/mips.h"
#include "dis.h"
#include "common/trie.h"

#define DS_FOREACH(ds, cur)\
       for (int i = 0; i < ds->num_instr && (cur=ds->instr[i]); i++)

#define X86_ARCH 1
#define MIPS_ARCH 2

#ifndef MODE_64B
#define MODE_64B MODE_X64
#endif
#ifndef MODE_32B
#define MODE_32B MODE_X86
#endif


struct disassembler {
	int arch, mode;

	struct dis **instr;
	int num_instr;

	struct trie_node *root;
};

struct disassembler *ds_init(int isa, int mode);

void ds_decode(struct disassembler *ds, unsigned char *stream, int size, uint64_t entry);
void ds_addinstr(struct disassembler *ds, struct dis *dis);

void ds_destroy(struct disassembler *ds);

#endif

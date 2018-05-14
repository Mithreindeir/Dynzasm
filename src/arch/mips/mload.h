#ifndef MIPS_LOAD_H
#define MIPS_LOAH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/file.h"
#include "../../common/trie.h"

#define MAX_MNEM_SIZE_MIPS 12

/*MIPS Instruction Entry*/
struct mips_instr_entry {
	char mnemonic[MAX_MNEM_SIZE_MIPS];
	char instr_type;
};

void mips_parse(struct trie_node *root, int mode);

#endif

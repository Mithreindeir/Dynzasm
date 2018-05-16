#ifndef ARM_LOAD_H
#define ARM_LOAD_H

#include <stdio.h>
#include <stdlib.h>
#include "../../common/file.h"
#include "../../common/trie.h"

#define MAX_MNEM_SIZE_ARM 128

/*ARM Instruction Entry*/
struct arm_instr_entry {
	char mnemonic[MAX_MNEM_SIZE_ARM];
	int type;
};

void arm_parse(struct trie_node *root, int mode);

#endif

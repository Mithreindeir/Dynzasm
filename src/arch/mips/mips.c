#include "mips.h"

struct dis *mips_disassemble(int mode, struct trie_ndoe *node, u8 *stream, long max, uint64_t addr, int *used_bytes)
{
	uint32_t instruction = *((uint64_t*)stream);
	*used_bytes = 4;

	struct trie_node *n = trie_lookup(root, OPCODE(instruction), 1);
	/*Some instructions have a func field that specifies the mnemonic*/
	if (n->flags & INSTR_FUNC) {
		n = trie_lookup(root, FUNC(instruction), 1);
	}

	if (!n || !n->value) {
		return NULL;
	}

	struct mips_instr_entry *e = n->value;
	struct dis *disas = dis_init();

	memcpy(disas->mnemonic, e->mnemonic, strlen(e->mnemonic));


	return disas;
}

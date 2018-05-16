#include "arm.h"


struct dis *arm_disassemble(int mode, struct trie_node *node, u8 *stream, long max, uint64_t addr, int *used_bytes)
{
	(void)mode;
	(void)addr;
	if (max < 4) {
		*used_bytes = max;
		return NULL;
	}

	uint32_t instruction = *((uint32_t*)stream);
	*used_bytes = 4;

	unsigned char tbits = THREE_OBITS(instruction);
	struct trie_node *n = trie_lookup(node, &tbits, 1);
	/*Invalid Data processing commands avoid the ambiguous instruction encoding*/
	if (CHECK_FLAG(n->flags, D_CROSS) && VALID_DPROC(instruction)) {
		unsigned char opcode = DATA_OPCODE(instruction);
		n = trie_lookup(n, &opcode, 1);
	} else if (CHECK_FLAG(n->flags, D_CROSS)) {
		if (IS_MULT(instruction)) {
		}
	}

	if (!n || !n->value) {
		return NULL;
	}

	struct dis *disas = dis_init();

	return disas;
}

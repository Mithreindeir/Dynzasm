#include "mips.h"

struct dis *mips_disassemble(int mode, struct trie_node *node, u8 *stream, long max, uint64_t addr, int *used_bytes)
{
	if (max < 4) {
		*used_bytes = max;
		return NULL;
	}
	(void)mode;
	(void)addr;
	uint32_t instruction = *((uint32_t*)stream);
	*used_bytes = 4;

	unsigned char opcode = OPCODE(instruction);
	struct trie_node *n = trie_lookup(node, &opcode, 1);
	/*Some instructions have a func field that specifies the mnemonic*/
	if (n->flags & INSTR_FUNC) {
		opcode = FUNC(instruction);
		n = trie_lookup(n, &opcode, 1);
	}

	if (!n || !n->value) {
		return NULL;
	}

	struct mips_instr_entry *e = n->value;
	struct dis *disas = dis_init();

	memcpy(disas->mnemonic, e->mnemonic, strlen(e->mnemonic));
	mips_decode_operands(disas, e, instruction);

	return disas;
}

void mips_decode_operands(struct dis *disas, struct mips_instr_entry *e, uint32_t instruction)
{
	switch (e->instr_type) {
		case 'R':
			dis_add_operand(disas, operand_reg(mips_registers[RD(instruction)]));
			dis_add_operand(disas, operand_reg(mips_registers[RS(instruction)]));
			dis_add_operand(disas, operand_reg(mips_registers[RT(instruction)]));
			break;
		case 'I':
			/*Store and Load Instruction have a different disassembly format*/
			if (e->mnemonic[0]=='s' || e->mnemonic[0]=='l') {
				dis_add_operand(disas, operand_reg(mips_registers[RT(instruction)]));
				struct operand_tree *indir = operand_tree_init(DIS_BRANCH);
				operand_tree_add(indir, operand_reg(mips_registers[RS(instruction)]));
				operand_tree_add(indir, operand_imm((int64_t)(int16_t)IMM(instruction)));
				strcpy(TREE_FORMAT(indir), "$1($0)");
				dis_add_operand(disas, indir);
			} else {
				dis_add_operand(disas, operand_reg(mips_registers[RT(instruction)]));
				dis_add_operand(disas, operand_reg(mips_registers[RS(instruction)]));
				dis_add_operand(disas, operand_imm((int64_t)(int16_t)IMM(instruction)));
			}
			break;
		case 'J':
			dis_add_operand(disas, operand_addr(ADDR(instruction)));
			break;
	}
}

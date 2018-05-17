#include "arm.h"


struct dis *arm_disassemble(int mode, struct trie_node *node, u8 * stream,
			    long max, uint64_t addr, int *used_bytes)
{
	(void) mode;
	if (max < 4) {
		*used_bytes = max;
		return NULL;
	}

	uint32_t instruction = *((uint32_t *) stream);
	unsigned char cond = COND(instruction);
	*used_bytes = 4;

	unsigned char tbits = THREE_OBITS(instruction);
	struct trie_node *n = trie_lookup(node, &tbits, 1);
	/*Invalid Data processing commands avoid the ambiguous instruction encoding */
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
	struct arm_instr_entry *e = n->value;

	struct dis *disas = dis_init();
	/*The end mnemonic contains several fields*/
	int miter = 0;
	miter += snprintf(disas->mnemonic, MAX_MNEM_SIZE_ARM, "%s", e->mnemonic);
	if (S_FIELD(instruction) && e->instr_type == DATA_PROCESS)
		miter += snprintf(disas->mnemonic+miter, MAX_MNEM_SIZE_ARM-miter, "s");
	if (L_FIELD(instruction) && e->instr_type == BRANCH)
		miter += snprintf(disas->mnemonic+miter, MAX_MNEM_SIZE_ARM-miter, "l");
	if (cond != ALWAYS_EXECUTE)
		snprintf(disas->mnemonic+miter, MAX_MNEM_SIZE_ARM-miter, "%s", arm_conditions[cond]);

	arm_decode_operands(disas, e, addr, instruction, n->flags);

	return disas;
}

void arm_decode_operands(struct dis *disas, struct arm_instr_entry *e,
			 uint64_t addr, uint32_t instr, u8 flags)
{
	(void)flags;
	switch (e->instr_type) {
		/*Data processing Instruction*/
		case 'D':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			if (!CHECK_FLAG(flags, D_NORN))
				dis_add_operand(disas, operand_reg(arm_registers[ARM_RN(instr)]));
			arm_shifter_operand(disas, instr, e->instr_type);
			break;
		/*Branch instruction (word aligned so botton 2 bits are always 0*/
		case 'B':;
			dis_add_operand(disas, operand_addr((ARM_OFFSET24(instr)<<2)+(addr+8)));
			break;
	}
}

void arm_shifter_operand(struct dis *disas, uint32_t instr, int type)
{
	switch (type) {
		case 'D':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RM(instr)]));
			if (ARM_SHIFT_AMOUNT(instr)) {
				struct operand_tree *shift = operand_tree_init(DIS_BRANCH);
				unsigned char st = ARM_SHIFT(instr);
				unsigned char sa = ARM_SHIFT_AMOUNT(instr);
				unsigned char rm = ARM_RS(instr);
				if (st==ARM_LSLI) {
					operand_tree_add(shift, operand_imm(sa));
					strcpy(TREE_FORMAT(shift), "lsl #$0");
				} else if (st == ARM_LSLR) {
					operand_tree_add(shift, operand_reg(arm_registers[rm]));
					strcpy(TREE_FORMAT(shift), "lsl $0");
				} else if (st==ARM_LSRI) {
					operand_tree_add(shift, operand_imm(sa));
					strcpy(TREE_FORMAT(shift), "lsr #$0");
				}
				dis_add_operand(disas, shift);
			}
			break;
	}
}

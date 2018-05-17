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
	/*Differentiate between load and store instructions*/
	} else if (CHECK_FLAG(n->flags, LDSTC)) {
		unsigned char sto = LD_L_FIELD(instruction);
		n = trie_lookup(n, &sto, 1);
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
	if (B_L_FIELD(instruction) && e->instr_type == BRANCH)
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
	char fmt[64];
	int fiter = 0;
	switch (e->instr_type) {
		/*Data processing Instruction*/
		case 'D':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			if (!CHECK_FLAG(flags, ARM_NORN))
				dis_add_operand(disas, operand_reg(arm_registers[ARM_RN(instr)]));
			arm_shifter_operand(disas, NULL, instr, e->instr_type);
			break;
		case 'I':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			if (!CHECK_FLAG(flags, ARM_NORN))
				dis_add_operand(disas, operand_reg(arm_registers[ARM_RN(instr)]));
			uint32_t rot = ARM_ROTATE_IMM(instr);
			uint32_t rimm = ARM_IMM8(instr);
			rimm = (rimm >> rot) | (rimm>>(32-rot));
			dis_add_operand(disas, operand_imm(rimm));
			break;
		/*Branch instruction (word aligned so botton 2 bits are always 0*/
		case 'B':;
			dis_add_operand(disas, operand_addr((ARM_OFFSET24(instr)<<2)+(addr+8)));
			break;
		/*Load/Store Immediate Offset*/
		case 'O':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			struct operand_tree *ioff = operand_tree_init(DIS_BRANCH);
			operand_tree_add(ioff, operand_reg(arm_registers[ARM_RN(instr)]));
			operand_tree_add(ioff, operand_imm(ARM_OFFSET12(instr)));
			fiter += snprintf(fmt+fiter, 64-fiter, "[$0");
			int ineg = !ARM_ADDSUB(instr);
			if (ARM_PREINDEX(instr)) fiter+=snprintf(fmt+fiter,64-fiter, ",%s$1]", ineg?" -":" ");
			else fiter+=snprintf(fmt+fiter,64-fiter, "],%s$1", ineg?" -":" ");
			strcpy(TREE_FORMAT(ioff), fmt);
			dis_add_operand(disas, ioff);
			break;
		/*Load/Store Register Offset*/
		case 'R':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			struct operand_tree *ireg = operand_tree_init(DIS_BRANCH);
			operand_tree_add(ireg, operand_reg(arm_registers[ARM_RN(instr)]));
			operand_tree_add(ireg, operand_reg(arm_registers[ARM_RM(instr)]));
			fiter += snprintf(fmt+fiter, 64-fiter, "[$0");
			int rneg = !ARM_ADDSUB(instr);
			if (ARM_PREINDEX(instr)) fiter+=snprintf(fmt+fiter,64-fiter, ",%s$1]", rneg?" -":" ");
			else fiter+=snprintf(fmt+fiter,64-fiter, "],%s$1", rneg?" -":" ");
			strcpy(TREE_FORMAT(ireg), fmt);
			arm_shifter_operand(disas, ireg, instr, e->instr_type);
			dis_add_operand(disas, ireg);
			break;
		/*Load/Store Multiple*/
		case 'L':
			dis_add_operand(disas, operand_reg(arm_registers[ARM_RD(instr)]));
			break;
	}
}

void arm_shifter_operand(struct dis *disas, struct operand_tree *opr, uint32_t instr, int type)
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
		case 'R':
			if (ARM_SHIFT_AMOUNT(instr)||ARM_SHIFT(instr)) {
				unsigned char st = ARM_SHIFT(instr);
				unsigned char si = ARM_SHIFT_AMOUNT(instr);
				char *fmt = strtok(TREE_FORMAT(opr), "]");
				fmt = fmt + strlen(fmt);
				if (!fmt) return;
				if (st==ARM_LSLI) {
					operand_tree_add(opr, operand_imm(si));
					strcpy(fmt, ", lsl #$2]");
				} else if (st==ARM_LSRI) {
					operand_tree_add(opr, operand_imm(!si?32:si));
					strcpy(fmt, ", lsr #$2]");
				} else if (st==ARM_ASR) {
					operand_tree_add(opr, operand_imm(!si?32:si));
					strcpy(fmt, ", asr #$2]");
				} else if (st==ARM_ROR_RRX) {
					if (!si) {
						operand_tree_add(opr, operand_imm(si));
						strcpy(fmt, ", ror #$2]");
					} else {
						strcpy(fmt, ", rrx]");
					}
				}
			}
			break;
	}
}

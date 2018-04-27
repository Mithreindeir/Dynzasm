#include "x86.h"

void x86_disassemble(unsigned char *stream, long max, struct dis *disas)
{
	dis_init(disas);
}

struct operand_tree *x86_disassemble_operand(int addr_mode, int opr_size, int addr_size, unsigned char *stream, long max)
{
	struct operand_tree *reg = malloc(sizeof(struct operand_tree));
	operand_tree_init(reg, DIS_OPER);
	reg->body.operand.operand_type = DIS_REG;
	strcpy(TREE_REG(reg), "eax");
	return reg;
}

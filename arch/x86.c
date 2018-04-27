#include "x86.h"

void x86_disassemble(struct trie_node *root, unsigned char *stream, long max, struct dis *disas)
{
	dis_init(disas);
	/*Find instruction match in the trie*/
	struct trie_node *n = trie_lookup(root, stream, max);
	unsigned char flags = 0;
	while (n->flags == PREFIX_FLAG) {
		stream++;
		max--;
		n = trie_lookup(root, stream, max);
	}
	/**/
	if (n->flags == REG_EXT_FLAG) {
		unsigned char sv = stream[1];
		//Zero out everything but the reg 3 bits
		sv &= 0x38;
		n = trie_lookup(root, &sv, 1);
	}
	/*If the instruction is not found, then die*/
	if (!n || !n->value) {
		printf("Unable to disassemble\n");
		return;
	}

	/*Create operands based on addressing modes*/
	struct x86_instr_entry *e = n->value;
	for (int i = 0; i < e->num_op; i++) {
		long s = x86_decode_operand(e->operand[i], flags, stream, max);
		stream += max;
		max -= s;
	}
}

/*Decodes operand information and passes it on to be disassembled. Returns used bytes*/
long x86_decode_operand(char *operand, unsigned char flags, unsigned char *stream, long max)
{
	/*Stream iterator*/
	long si = 0;
	/*Set initial size based on defaults and flags*/
	int operand_size = DEF_OPER_SIZE + CHECK_FLAG(flags, REX_W);
	int addr_size = DEF_ADDR_SIZE - CHECK_FLAG(flags, ADDR_SIZE_OVERRIDE);

	/*If the operand is an addressing mode, then it will be a capital letter, otherwise a value*/
	if (operand[0] >= 'A' && operand[0] <= 'Z') {
		/*Set operand size*/
		operand_size = x86_operand_size(operand_size, operand[1], flags);
	}
	printf("Operand Size: %d, Addr Size: %d\n", operand_size, addr_size);
	return si;
}

/*Disassembles operand and returns used bytes*/
long x86_disassemble_operand(unsigned char addr_mode, int op_size, int addr_size, unsigned char flags)
{
	/*Construct operand based on addressing mode, size, and flags*/
	switch (addr_mode) {
		/*MODRM Encoding*/
		case 'E':


			break;
	}
}

int x86_operand_size(int op_size, char size_byte, unsigned char flags)
{
	switch (size_byte) {
		/*Byte*/
		case 'b': return 1;
		/*Byte Or Word*/
		case 'c': return 1 + CHECK_FLAG(flags, OPER_SIZE_OVERRIDE);
		/*Double Word or Word*/
		case 'd': return 3 - CHECK_FLAG(flags, OPER_SIZE_OVERRIDE);
		/*Quad word, double word or word*/
		case 'v': return 3 - CHECK_FLAG(flags, OPER_SIZE_OVERRIDE) + CHECK_FLAG(flags, REX_W);
		/*Word*/
		case 'w': return 2;
		/*Quad word*/
		case 'q': return 4;
	}
	return op_size;
}

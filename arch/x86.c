#include "x86.h"

void x86_disassemble(struct trie_node *root, u8 *stream, long max, struct dis *disas)
{
	if (!max) return;
	dis_init(disas);
	/*Find instruction match in the trie*/
	struct trie_node *n = trie_lookup(root, stream, max);
	u8 flags = 0;
	while (n->flags == PREFIX_FLAG) {
		char prefix = n->key;
		if (prefix >= 0x40 && prefix <= 0x50) {
			prefix = prefix & 0x0f;
			if (prefix & 0x1)
				flags |= REX_B;
			if (prefix & 0x2)
				flags |= REX_X;
			if (prefix & 0x4)
				flags |= REX_R;
			if (prefix & 0x8)
				flags |= REX_W;
		}
		stream++;
		max--;
		n = trie_lookup(root, stream, max);
	}
	/*Some instructions have an opcode extension in the reg part of the modrm byte*/
	if (n->flags == REG_EXT_FLAG) {
		u8 sv = stream[1];
		//Zero out everything but the reg 3 bits
		sv &= 0x38;
		n = trie_lookup(root, &sv, 1);
	}
	/*If the instruction is not found, then die*/
	if (!n || !n->value) {
		printf("Unable to disassemble\n");
		return;
	}

	stream += n->dist;
	max -= n->dist;

	u8 *operand_stream = stream;
	long operand_max = max;

	struct x86_instr_entry *e = n->value;
	/*Set mnemonic*/
	memcpy(disas->mnemonic, e->mnemonic, strlen(e->mnemonic));

	/*Create operands based on addressing modes*/
	for (int i = 0; i < e->num_op; i++) {
		struct operand_tree *operand = NULL;
		/*The reg operand in may need a byte previous in the stream, so pass in the first byte*/
		if (e->operand[i][0]=='G') {
			(void)x86_decode_operand(&operand, e->operand[i], flags, operand_stream, operand_max);
		} else {
			long s = x86_decode_operand(&operand, e->operand[i], flags, stream, max);
			stream += s;
			max -= s;
		}
		if (operand)
			dis_add_operand(disas, operand);
	}
}

/*Decodes operand information and passes it on to be disassembled. Returns used bytes*/
long x86_decode_operand(struct operand_tree **opt, char *operand, u8 flags, u8 *stream, long max)
{
	/*Stream iterator*/
	long iter = 0;
	/*Set initial size based on defaults and flags*/
	int operand_size = DEF_OPER_SIZE + CHECK_FLAG(flags, REX_W);
	int addr_size = DEF_ADDR_SIZE - CHECK_FLAG(flags, ADDR_SIZE_OVERRIDE);

	/*If the operand is an addressing mode, then it will be a capital letter, otherwise a value*/
	if (operand[0] >= 'A' && operand[0] <= 'Z') {
		/*Set operand size*/
		operand_size = x86_operand_size(operand_size, operand[1], flags);
		iter += x86_disassemble_operand(opt, operand[0], operand_size, addr_size, stream+iter, max-iter, flags);
	} else {
		/*Check if its a register*/
		int ridx = get_register_index(operand);
		if (ridx != -1) {
			*opt = malloc(sizeof(struct operand_tree));
			operand_reg(*opt, operand);
		} else {

		}
	}
	return iter;
}

/*Disassembles operand and returns used bytes*/
long x86_disassemble_operand(struct operand_tree **operand, u8 addr_mode, int op_size, int addr_size, u8 *stream, long max, u8 flags)
{
	long iter = 0;
	/*Construct operand based on addressing mode, size, and flags*/
	switch (addr_mode) {
		/*MODRM Encoding*/
		case 'E': /*Modrm encoding*/
			iter += x86_decode_modrm(operand, op_size, addr_size, stream, max, flags);
			break;
		case 'G':; /*Register modrm encoding*/
			u8 mrmreg = (stream[0] & 0x38) >> 3;
			const char *reg = get_register(mrmreg, op_size, CHECK_FLAG(flags, REX_B));
			*operand = malloc(sizeof(struct operand_tree));
			operand_reg(*operand, reg);
			break;
		case 'I':; /*Immediate Value*/
			uint64_t imm = 0;
			if (op_size==4) {
				imm=*((uint64_t*)(stream+iter));
				iter += 8;
			} else if (op_size==3) {
				imm = *((uint32_t*)(stream+iter));
				iter += 4;
			} else if (op_size == 2) {
				imm = *(uint16_t*)(stream+iter);
				iter += 2;
			} else {
				imm = stream[iter++];
			}
			*operand = malloc(sizeof(struct operand_tree));
			operand_imm(*operand, imm);
			break;
	}
	return iter;
}

/*Decodes modrm byte*/
long x86_decode_modrm(struct operand_tree **operand, int op_size, int addr_size, u8 *stream, long max, u8 flags)
{
	long iter = 0;
	/*Get modrm byte*/
	u8 modrm = stream[iter++];
	/*MODRM byte. mod = xx000000, reg = 00xxx000, rm = 00000xxx*/
	u8 mod = (modrm & 0xc0) >> 6;
	//u8 reg = (modrm & 0x38) >> 3;
	u8 rm = (modrm & 0x7);
	/*Scale Index Base Encoding when rm == 4 and mod != 3*/
	if (rm == SIB_RM && mod != MODRM_REG) {
		iter += x86_decode_sib(operand, op_size, addr_size, stream+iter, max-iter, flags);
		return iter;
	}
	/*MODRM: EBP is invalid rm byte in indirect mode, so it means disp only*/
	if (MODRM_DISPONLY(mod, rm)) {
		if (max < 4) return iter;
		*operand = x86_indir_operand_tree(op_size, "rip", NULL, 1, *(uint32_t*)stream);
	}

	const char *reg;
	/*Indirect registers are the address size, while the data size is the operand size*/
	switch (mod) {
		case MODRM_INDIR:
			reg = get_register(rm, addr_size, CHECK_FLAG(flags, REX_B));
			*operand = x86_indir_operand_tree(op_size, reg, NULL, 1, 0);
			break;
		case MODRM_1DISP:
			reg = get_register(rm, addr_size, CHECK_FLAG(flags, REX_B));
			unsigned char bdisp = stream[iter++];
			bdisp = bdisp > 0x80 ? 0x100 - bdisp : bdisp;
			*operand = x86_indir_operand_tree(op_size, reg, NULL, 1, bdisp);
			break;
		case MODRM_4DISP:
			reg = get_register(rm, addr_size, CHECK_FLAG(flags, REX_B));
			uint32_t disp = *(uint32_t*)(stream+iter);
			iter += 4;
			if (disp > 0x80000000)
				disp = 0x100000000 - disp;
			*operand = x86_indir_operand_tree(op_size, reg, NULL, 1, disp);
			break;
		case MODRM_REG:
			*operand = malloc(sizeof(struct operand_tree));
			operand_reg(*operand, get_register(rm, op_size, CHECK_FLAG(flags, REX_B)));
			break;
	}

	return iter;
}

/*Decodes sib byte*/
long x86_decode_sib(struct operand_tree **operand, int op_size, int addr_size, u8 *stream, long max, u8 flags)
{
	long iter = 0;

	/*This is safe because decode sib must be called from the decode modrm function*/
	u8 mod = ((stream[-1] & 0xc0) >> 6);
	u8 sib = stream[iter++];
	int power = (sib&0xc0)>>6;
	int scale = 1;
	/*Scale is a power of two*/
	for (int i = 0; i < power; i++)
		scale *= 2;
	u8 idx = ((sib&0x38)>>3);
	u8 bse = sib&0x7;
	const char *index = NULL, *base = NULL;
	if (!SIB_NO_INDEX(idx)) index = get_register(idx, addr_size, CHECK_FLAG(flags, REX_X));
	if (!SIB_NO_BASE(mod, bse)) base = get_register(bse, addr_size, CHECK_FLAG(flags, REX_B));

	long offset = 0;
	if (mod == 1) {
		offset = stream[iter++];
		offset = offset > 0x80 ? 0x100 - offset : offset;
	} else if (mod == 2) {
		offset = *(uint16_t*)(stream+iter);
		iter += 2;
		offset = offset > 0x8000 ? 0x10000 - offset : offset;

	} else if (mod == 0 && bse==5) {
		offset = *(uint32_t*)(stream+iter);
		iter += 4;
		offset = offset > 0x80000000 ? 0x100000000 - offset : offset;
	}
	*operand = x86_indir_operand_tree(op_size, base, index, scale, offset);

	return iter;
}


/*Create an operand tree given the possible parameters for an indirect memory address*/
struct operand_tree *x86_indir_operand_tree(int op_size, const char *base, const char *index, long scale, unsigned long offset)
{
	struct operand_tree *indir = malloc(sizeof(struct operand_tree));
	operand_tree_init(indir, DIS_BRANCH);

	/*Create a format string based on possible parameters*/
	char buf[32];
	memset(buf, 0, 32);
	int iter = 0;
	iter += snprintf(buf+iter, 32-iter, "%s [", operand_size_prefix[op_size-1]);

	/*Possible parameters: base op, index op, scale op, offset op*/
	struct operand_tree *bop = NULL, *iop = NULL, *sop = NULL, *oop = NULL;
	if (base) {
		bop = malloc(sizeof(struct operand_tree));
		operand_reg(bop, base);
		operand_tree_add(indir, bop);
		iter += snprintf(buf+iter, 32-iter, "$%d", TREE_NCHILD(indir)-1);
	}
	if (index) {
		iop = malloc(sizeof(struct operand_tree));
		operand_reg(iop, index);
		operand_tree_add(indir, iop);
		iter += snprintf(buf+iter, 32-iter, "+$%d", TREE_NCHILD(indir)-1);
	}
	if (scale != 1) {
		sop = malloc(sizeof(struct operand_tree));
		operand_imm(sop, scale);
		operand_tree_add(indir, sop);
		iter += snprintf(buf+iter, 32-iter, "*$%d", TREE_NCHILD(indir)-1);
	}
	if (offset != 0) {
		oop = malloc(sizeof(struct operand_tree));
		operand_addr(oop, offset);
		operand_tree_add(indir, oop);
		iter += snprintf(buf+iter, 32-iter, "+$%d", TREE_NCHILD(indir)-1);
	}
	iter += snprintf(buf+iter, 32-iter, "]");
	strcpy(TREE_FORMAT(indir), buf);
	return indir;
}

int x86_operand_size(int op_size, char size_byte, u8 flags)
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

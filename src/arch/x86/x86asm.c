#include "x86asm.h"

/*Loop through possible instructions and remove the ones with
 * mismatches addressing modes. Then attempt to encode using the
 * instructions encoding. Print errors if any. Resolve flags backwards
 * from leaf in trie node.
 * */
void x86_assemble(char **tokens, int num_tokens, struct hash_entry *instr_head)
{
//	for (int i = 0; i < num_tokens; i++)
//		printf("%s%s", tokens[i], (i+1)==num_tokens?"\n":" ");
	struct hash_entry *cur = instr_head;
	while (cur && !strcmp(cur->mnemonic, instr_head->mnemonic)) {
		struct trie_node *node = cur->value;
		struct x86_instr_entry *e = node->value;
		if (x86_classify_operand(tokens+1, num_tokens-1, e->operand, e->num_op)) {
#ifdef DEBUG
			printf("%s ", instr_head->mnemonic);
			for (int i = 0; i < e->num_op; i++)
				printf("%s%s", e->operand[i], (i+1)==e->num_op?"\n":", ");
#endif
			x86_encode(tokens+1, num_tokens-1, node, e);
		}
		cur = cur->next;
	}
}

/*Checks if the tokens match the operand addressing modes*/
int x86_classify_operand(char **tokens, int num_tokens, char operands[][MAX_OPER_LEN], int num_operands)
{
	if (!num_operands && !num_tokens) return 1;
	int idx = 0, len = 0, operand = 0;
	while ((idx=x86_next_operand(tokens, num_tokens, operand, &len)) != -1) {
		if (operand >= num_operands) return 0;
		if (!x86_match_operand(tokens+idx, len, operands[operand]))
			return 0;
		operand++;
	}
	return operand == num_operands;
}

/*Returns the token idx for a operand (delimiters are commas)*/
int x86_next_operand(char ** tokens, int nt, int op, int *len)
{
	int idx = 0, lidx=0;
	int opr = 0;
	while (idx <= nt) {
		if((idx<nt&&*tokens[idx]==',') || idx==nt) {
			if (opr == op) {
				*len = idx - lidx;
				return lidx;
			}
			lidx = idx+1;
			opr++;
		}
		idx++;
	}
	return -1;
}

/*Doesn't Check For Illegal Combinations, but just matches syntax to instructions*/
int x86_match_operand(char **tokens, int num_tokens, char *op_type)
{
	if (!tokens || !num_tokens) return 0;
	if (op_type[0] >= 'A' && op_type[0] <= 'Z') {
		int idx = 0;
		int size = op_type[1];
		int nsize = 0;
		uint64_t num= 0;
		char ot = op_type[0];
		if (ot=='G') {
			idx = get_register_index(tokens[0]);
			if (idx != -1 && X86_SIZE_COMPAT(size, REG_SIZE_IDX(idx)) && num_tokens == 1)
				return 1;
		} else if (ot == 'E' || ot == 'M') {
			if (x86_valid_modrm(tokens, num_tokens, op_type[1]))
				return 1;
		} else if (ot == 'I' || ot == 'J' || ot == 'A') {
			if (!isdigit(**tokens)) return 0;
			num = strtol(*tokens, NULL, 0);
			if (num < MAX(8)) nsize = 1;
			else if (num < MAX(16)) nsize = 2;
			else if (num < MAX(32)) nsize = 3;
			else nsize = 4;
			if (X86_SIZE_MIN(op_type[1], nsize)) return 1;
		}
		/*
		switch (op_type[0]) {
			case 'G':
				idx = get_register_index(tokens[0]);
				if (idx != -1 && X86_SIZE_COMPAT(size, REG_SIZE_IDX(idx)) && num_tokens == 1)
					return 1;
				break;
			case 'E':
				if (x86_valid_modrm(tokens, num_tokens, op_type[1]))
					return 1;
				break;
			case 'M':
				if (x86_valid_modrm(tokens, num_tokens, 0)||**tokens == '[')
					return 1;
				break;
			case 'I':
				if (!isdigit(**tokens)) return 0;
				num = strtol(*tokens, NULL, 0);
				if (num < MAX(8)) nsize = 1;
				else if (num < MAX(16)) nsize = 2;
				else if (num < MAX(32)) nsize = 3;
				else nsize = 4;
				if (X86_SIZE_MIN(op_type[1], nsize)) return 1;
				break;
			case 'J':
				if (!isdigit(**tokens)) return 0;
				num = strtol(*tokens, NULL, 0);
				if (num < MAX(8)) nsize = 1;
				else if (num < MAX(16)) nsize = 2;
				else if (num < MAX(32)) nsize = 3;
				else nsize = 4;
				if (X86_SIZE_MIN(op_type[1], nsize)) return 1;
				break;
		}
*/
	} else {
		if (!strcmp(tokens[0], op_type)) {
			return 1;
		}
	}
	return 0;
}

/*Doesn't check for illegal combinations but just checks that in format of size [ addr ]*/
int x86_valid_modrm(char **tokens, int num_tokens, int size)
{
	if (!tokens || !num_tokens) return 0;

	int ridx = get_register_index(tokens[0]);
	if (ridx != -1 && X86_SIZE_COMPAT(size, REG_SIZE_IDX(ridx)) && num_tokens == 1)
		return 1;
	if (X86_SIZE_COMPAT(size, x86_size(*tokens)))
		return 1;
	return 0;
}

int x86_size(char *tok)
{
	if (!tok) return 0;
	if (!strcmp(tok,"byte")) return 1;
	if (!strcmp(tok, "word")) return 2;
	if (!strcmp(tok, "dword")) return 3;
	if (!strcmp(tok, "qword")) return 4;
	return 0;
}

void x86_encode(char **tokens, int num_tokens, struct trie_node *n, struct x86_instr_entry *e)
{
	/*Figure out the default size, so that the encoder can set override bits*/
	int os = DEF_OPER_SIZE(2), as = DEF_ADDR_SIZE(2);
	u8 *barr = NULL;
	u8 flags = 0;
	int blen = 0, idx = 0, len = 0;
	for (int i = 0; i < e->num_op; i++) {
		if (!isupper(*e->operand[i])) continue;
		idx = x86_next_operand(tokens, num_tokens, i, &len);
		if (*e->operand[i]=='E' || *e->operand[i]=='M') {
			x86_encode_modrm(tokens+idx, len, &barr, &blen, os, as, &flags);
		} else if (*e->operand[i]=='G') {
			int reg = get_register_index(tokens[idx]);
			if (reg == -1) continue;
			reg = REG_BIN_IDX(reg);
			if (reg>7) {
				reg -= 8;
				SET_FLAG(flags, REX_R);
			}
			if (blen) barr[0] |= (reg<<3);
			else x86_add_byte(&barr, &blen, reg<<3);
		} else if (*e->operand[i]=='I'||*e->operand[i]=='J'||*e->operand[i]=='A') {
			if (e->operand[i][1]=='b') {
				int off = (*e->operand[i]=='J'||*e->operand[i]=='A')?n->dist+1:0;
				x86_add_byte(&barr, &blen, strtol(tokens[idx], NULL, 0)-off);
			} else if (e->operand[i][1]=='v'||e->operand[i][1]=='d') {
				uint32_t v = strtol(tokens[idx], NULL, 0);
				if (*e->operand[i]=='J'||*e->operand[i]=='A')
					v -= n->dist + 4;
				for (int i = 0; i < 4; i++)
					x86_add_byte(&barr, &blen, ((u8*)&v)[i]);
			}
		}
	}
	int ops = 0;
	struct trie_node *p = n;
	u8 tflags = 0;
	for (int i = 0; p && (p->value || p->parent); p=p->parent, i++) {
		if (p->parent) tflags = p->parent->flags;
		else tflags = 0;
		if (!tflags) {
			x86_add_pbyte(&barr, &blen, p->key);
			ops++;
		} else if (CHECK_FLAG(tflags, REG_EXT_FLAG)) {
			if (blen > ops) barr[ops] |= p->key;
		}
	}
	/*If rex prefix*/
	if ((flags>>2)) {
		x86_add_pbyte(&barr, &blen, 0x40 + (flags>>2));
	}
#ifdef DEBUG
	printf("bstream: ");
#endif
	for (int i = 0; i < blen; i++)
		printf("%02x ", barr[i]);
	printf("\n");
	free(barr);
}

void x86_encode_modrm(char **tokens, int num_tokens, u8 **barr, int *blen, int os, int as, u8 * flags)
{
	u8 modrm=0;
	int reg;
	if ((reg=get_register_index(*tokens)) != -1) {/*Direct Encoding: Mod=11*/
		modrm |= (3<<6);
		if (REG_SIZE_IDX(reg) == 4 && os == 3)
			SET_FLAG(*flags, REX_W);
		reg = REG_BIN_IDX(reg);
		if (reg > 7) {
			reg -= 8;
			SET_FLAG(*flags, REX_B);
		}
		modrm |= reg;
		if (!(*blen)) x86_add_byte(barr, blen, modrm);
		else (*barr)[0] |= modrm;
	} else {/*Indirect encoding: mod=0,01,10*/
		char *base=NULL,*index=NULL;
		int s=0, ds = 0;
		uint64_t disp = 0;
		int sz=x86_get_indir(tokens, num_tokens, &base,&index,&s,&disp,&ds);
		if (sz == 4 && os == 3) {
			SET_FLAG(*flags, REX_W);
		}
		u8 idxr = 0, bsr = 0;
		if (index) {
			idxr = REG_BIN_IDX(get_register_index(index));
			if (idxr > 7) {
				idxr -= 8;
				SET_FLAG(*flags, REX_X);
			}
		}
		if (base) {
			bsr = REG_BIN_IDX(get_register_index(base));
			if (bsr > 7) {
				bsr -= 8;
				SET_FLAG(*flags, REX_B);
			}
		}
		if (ds>1) ds = 4;
		if (s || index) {/*If scale or index, then using SIB encoding*/
			modrm |= 4;
			if (ds==1) modrm |= 1<<6;
			else if (ds==4) modrm |= 2<<6;
			u8 sib = 0;
			if (s) sib |= X64_SCALE(s)<<6;
			if (index) sib |= idxr<<3;
			else sib |= 4<<3;
			if (base) {
				sib |= bsr;
			} else {
				sib |= 5;
				if (!ds) disp=0,ds=4;
				else modrm ^= modrm&0xc0;
			}
			if (!(*blen)) x86_add_byte(barr, blen, modrm);
			else (*barr)[0] |= modrm;
			x86_add_byte(barr, blen, sib);
		} else {
			if (base && ds==1) modrm |= 1<<6;
			else if (base && ds > 1) modrm |= 2 << 6;
			if ((!base && ds) || (base && !strcmp(base, "rip"))) {
				ds = 4;
				modrm |= 5;
			} else if (base) modrm |= bsr;
			if (!(*blen)) x86_add_byte(barr, blen, modrm);
			else (*barr)[0] |= modrm;
		}
		for (int i = 0; i < ds; i++)
			x86_add_byte(barr, blen, ((u8*)&disp)[i]);
	}
}

/*Parses an Indirect Memory string and sets the base, index, scale, displacement, and displacement size*/
int x86_get_indir(char **tokens, int nt, char **b, char **i, int*s, uint64_t*d, int*ds)
{
	*b=NULL,*i=NULL,*s=0,*d=0,*ds=0;
	if (!nt||!tokens) return 0;
	int idx = 0, io=-1;
	char *iter=tokens[0],*l=NULL;
	int size=0, sz = 0;
	while (idx < nt && (l=iter) && (iter=tokens[idx++])) {
		if ((size=x86_size(iter)) && (sz=size)) continue;
		if (*iter=='[') {
			io=0;
			continue;
		}
		if (*iter==']') break;
		if (io==0&&get_register_index(iter)!=-1) io++, *b = iter;
		if ((*l=='+') &&io==1&&get_register_index(iter)!=-1) *i = iter;
		if ((*l=='*')&&io>0&&get_register_index(iter)==-1) io++, *s = strtol(iter, NULL, 0);
		if (((*l=='+'||*l=='-')&&get_register_index(iter)==-1)||(io==0&&get_register_index(iter)==-1)) {
			uint64_t num = strtol(iter, NULL, 0);
			int nsize = 0;
			if (num < MAX(8)) nsize = 1;
			else if (num < MAX(16)) nsize = 2;
			else if (num < MAX(32)) nsize = 3;
			else nsize = 4;
			num = *l=='-'?-num:num;
			*d = num, *ds = nsize;
			io++;
		}
	}
	return sz;
}

void x86_add_byte(u8 **barr, int *len, u8 b)
{
	int l = *len;
	l++;
	u8 *arr = *barr;
	if (!arr) arr = malloc(l);
	else arr = realloc(arr, l);
	arr[l-1] = b;
	*len = l;
	*barr = arr;
}

void x86_add_pbyte(u8 **barr, int *len, u8 b)
{
	int l = *len;
	l++;
	u8 *arr = *barr;
	if (!arr) arr = malloc(l);
	else arr = realloc(arr, l);
	memmove(arr+1, arr, l-1);
	arr[0] = b;
	*len = l;
	*barr = arr;
}

#include "x86asm.h"

/*Loop through possible instructions and remove the ones with
 * mismatches addressing modes. Then attempt to encode using the
 * instructions encoding. Print errors if any. Resolve flags backwards
 * from leaf in trie node.
 * */
void x86_assemble(char **tokens, int num_tokens, struct hash_entry *instr_head)
{
	for (int i = 0; i < num_tokens; i++)
		printf("%s%s", tokens[i], (i+1)==num_tokens?"\n":" ");
	unsigned char bbuf[32];
	struct hash_entry *cur = instr_head;
	while (cur && !strcmp(cur->mnemonic, instr_head->mnemonic)) {
		struct trie_node *node = cur->value, *p = cur->value;
		for (int i = 0; p; p=p->parent, i++)
			bbuf[node->dist-i-1] = p->key;
		struct x86_instr_entry *e = node->value;
		if (x86_classify_operand(tokens+1, num_tokens-1, e->operand, e->num_op)) {
			for (int i = 0; i < node->dist; i++)
				printf("%02x ", bbuf[i]);
			printf("%s ", instr_head->mnemonic);
			for (int i = 0; i < e->num_op; i++)
				printf("%s%s", e->operand[i], (i+1)==e->num_op?"\n":", ");
		}
		cur = cur->next;
	}
}

/*Checks if the tokens match the operand addressing modes*/
int x86_classify_operand(char **tokens, int num_tokens, char operands[][MAX_OPER_LEN], int num_operands)
{
	int idx = 0;
	int lidx = 0;
	int mop = 0;
	while (idx <= num_tokens) {
		if((idx<num_tokens&&*tokens[idx]==',') || idx==num_tokens) {
			if (mop >= num_operands) return 0;
			if (!x86_match_operand(tokens+lidx, idx-lidx, operands[mop]))
				return 0;
			lidx = idx+1;
			mop++;
		}
		idx++;
	}
	return mop == num_operands;
}

/*Doesn't Check For Illegal Combinations, but just matches syntax to instructions*/
int x86_match_operand(char **tokens, int num_tokens, char *op_type)
{
	if (!tokens || !num_tokens) return 0;
	if (op_type[0] >= 'A' && op_type[0] <= 'Z') {
		int idx = 0;
		int size = op_type[1];
		switch (op_type[0]) {
			case 'G':
				/*Register Encoding*/
				idx = get_register_index(tokens[0]);
				if (idx != -1 && X86_SIZE_COMPAT(size, REG_SIZE_IDX(idx)) && num_tokens == 1)
					return 1;
				break;
			case 'E':
				/*MODRM Encoding*/
				if (x86_valid_modrm(tokens, num_tokens, op_type[1]))
					return 1;

				break;
			case 'M':
				if (x86_valid_modrm(tokens, num_tokens, 0)||**tokens == '[')
					return 1;
				break;
			case 'I':
				if (!isdigit(**tokens)) return 0;
				int nsize = 0;
				unsigned long long num = strtol(*tokens, NULL, 0);
				if (num < MAX(8)) nsize = 1;
				else if (num < MAX(16)) nsize = 2;
				else if (num < MAX(32)) nsize = 3;
				else nsize = 4;
				if (X86_SIZE_MIN(op_type[1], nsize)) return 1;
				break;
		}
	} else {
		if (!strcmp(tokens[0], op_type))
			return 1;
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
	if (!strcmp(tok,"byte")) return 1;
	if (!strcmp(tok, "word")) return 2;
	if (!strcmp(tok, "dword")) return 3;
	if (!strcmp(tok, "qword")) return 4;
	return 0;
}

void x86_encode(char **tokens, int num_tokens, struct trie_node *n, struct x86_instr_entry *e)
{
	char op[3];
	for (int i = 0; i < e->num_op; i++)
		op[i] = *e->operand[i];
	/*If the instruction uses modrm encoding*/
	if ((op[0]=='G'&&op[1]=='E')||(op[0]=='E'&&op[1]=='G')) {

	}
}

#include "dis.h"

void dis_init(struct dis *dis)
{
	if (!dis) return;

	dis->id = 0;
	dis->operands = NULL;
	dis->num_operands = 0;
	memset(dis->mnemonic, 0, MNEM_SIZE);
	memset(dis->squashed, 0, SQUASH_SIZE);
	memset(dis->group, 0, GROUP_SIZE);
}

void dis_add_operand(struct dis *dis, struct operand_tree *tree)
{
	if (!dis) return;
	dis->num_operands++;
	if (!dis->operands)
		dis->operands = malloc(sizeof(struct operand_tree*));
	else
		dis->operands = realloc(dis->operands, sizeof(struct operand_tree*)*dis->num_operands);
	dis->operands[dis->num_operands-1] = tree;
}

void operand_tree_init(struct operand_tree *tree, int type)
{
	if (!tree) return;

	tree->type = type;
	if (type == DIS_OPER) {
		tree->body.operand.operand_type = DIS_UNSET;
	} else if (type == DIS_BRANCH) {
		tree->body.op_tree.num_operands = 0;
		tree->body.op_tree.operands = NULL;
	} else {
		//error
	}
}

void operand_reg(struct operand_tree *tree, const char *reg)
{
	tree->type = DIS_OPER;
	tree->body.operand.operand_type = DIS_REG;
	long len = strlen(reg);
	memcpy(TREE_REG(tree), reg, len>=REG_SIZE?(REG_SIZE-1):len);
}

void operand_imm(struct operand_tree *tree, const long imm)
{
	tree->type = DIS_OPER;
	tree->body.operand.operand_type = DIS_IMM;
	TREE_IMM(tree) = imm;
}

void operand_addr(struct operand_tree *tree, const long addr)
{
	tree->type = DIS_OPER;
	tree->body.operand.operand_type = DIS_IMM;
	TREE_IMM(tree) = addr;
}


int operand_squash(char *buf, long max, struct operand_tree *tree)
{
	if (!tree) return 0;

	long iter = 0;
	if (tree->type == DIS_OPER) {
		if (tree->body.operand.operand_type == DIS_ADDR) {
			iter+=snprintf(buf+iter,max-iter,"%#lx", TREE_ADDR(tree));
		} else if (tree->body.operand.operand_type == DIS_IMM) {
			iter+=snprintf(buf+iter,max-iter,"%ld", TREE_IMM(tree));
		} else if (tree->body.operand.operand_type == DIS_REG) {
			iter+=snprintf(buf+iter,max-iter,"%s", TREE_REG(tree));
		}
	} else if (tree->type == DIS_BRANCH) {
		for (int i = 0; i < TREE_NCHILD(tree); i++) {
			iter+=operand_squash(buf+iter, max-iter, TREE_CHILD(tree,i));
		}
	}
	return iter;
}

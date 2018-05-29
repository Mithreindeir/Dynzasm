#ifndef X86_ASM_H
#define X86_ASM_H

#include <ctype.h>
#include "../../common/table.h"
#include "../../common/trie.h"
#include "x86load.h"
#include "x86strings.h"

#define X86_SIZE_COMPAT(csize, size) ((csize=='q'&&size==4)\
||(csize=='v'&&(size==3||size==4))\
||(csize=='d'&&size==3)\
||(csize=='w'&&size==2)\
||(csize=='b'&&size==1)\
||(csize==0))

#define X86_SIZE_MIN(csize, size) ((csize=='q'&&size<=4)\
||(csize=='v'&&size<=4)\
||(csize=='d'&&size<=3)\
||(csize=='w'&&size<=2)\
||(csize=='b'&&size==1))

#define MAX(bits) (((unsigned long long)1<<(bits))-1)

void x86_assemble(char **tokens, int num_tokens, struct hash_entry *instr_head);
int x86_classify_operand(char **tokens, int num_tokens, char operands[][MAX_OPER_LEN], int num_operands);
int x86_match_operand(char **tokens, int num_tokens, char *op_type);

int x86_size(char *tok);
int x86_valid_modrm(char **tokens, int num_tokens, int size);

void x86_encode(char **tokens, int num_tokens, struct trie_node *n, struct x86_instr_entry *e);


#endif

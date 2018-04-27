#ifndef X86_H
#define X86_H

#include "../include/dis.h"
#include "../common/trie.h"
#include "x86load.h"

/*Macro to check a bit for a flag*/
#define CHECK_FLAG(byte, flag) (!!(byte & flag))

/*Trie Node Flags*/
#define REG_EXT_FLAG 2
#define PREFIX_FLAG 4

/*Instruction Flags*/
#define OPER_SIZE_OVERRIDE 	1 //01
#define ADDR_SIZE_OVERRIDE 	2 //10
#define REX_B 			4 //100
#define REX_X			8 //1000
#define REX_W 			16 //10000


/*Defaults*/
#define DEF_OPER_SIZE 3
#define DEF_ADDR_SIZE 4

void x86_disassemble(struct trie_node *node, unsigned char *stream, long max, struct dis *disas);
long x86_decode_operand(char *operand, unsigned char flags, unsigned char *stream, long max);
int x86_operand_size(int op_size, char size_byte, unsigned char flags);
long x86_disassemble_operand(unsigned char addr_mode, int op_size, int addr_size, unsigned char flags);

#endif

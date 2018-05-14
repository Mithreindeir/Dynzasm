#ifndef MIPS_H
#define MIPS_H

#include <stdio.h>
#include <stdint.h>
#include "../../dis.h"

/*Instruction Entry Flags*/
#define INSTR_FUNC 2

/*Instruction Types*/
#define TYPE_R 'R'
#define TYPE_I 'I'
#define TYPE_J 'J'

/*Macros*/
#define OPCODE(instr) ((instr>>26))
#define FUNC(instr) (instr&0x3f)
#define SHAMT(instr) ((instr>>6)&0x1f)
#define RS(instr) ((instr>>21)&0x1f)
#define RT(instr) ((instr>>16)&0x1f)
#define RD(instr) ((instr>>11)&0x1f)
#define ADDR(instr) ((instr&0x3ffffff))

struct dis *mips_disassemble(int mode, struct trie_node *node, u8 *stream, long max, uint64_t addr, int *used_bytes);


#endif

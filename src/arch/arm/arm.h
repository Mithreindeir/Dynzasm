#ifndef ARM_H
#define ARM_H

#include <stdio.h>
#include <stdlib.h>
#include "../../common/trie.h"
#include "../../common/common.h"
#include "../../dis.h"

/*Trie node flags*/
#define D_CROSS 1

/*Bitfield Extraction Macros*/
#define DATA_OPCODE(ins) (BITS(ins, 20, 24))
#define S_FIELD(ins) (BITS(ins, 19, 20))
#define THREE_OBITS(ins) (BITS(ins, 24, 27))

/*Instruction Type Macros*/
#define VALID_DPROC(instr) (!(!(THREE_OBITS(instr)<=1) || ((DATA_OPCODE(instr)>>2==0x2) && !S_FIELD(instr))))
#define IS_MULT(ins) ((BITS(ins, 25, 28)==0&&BITS(ins, 4,8)==0x9))

/*Instruction Encoding Types*/
#define DATA_PROCESS 'D'
#define DATA_PROCESSI 'I'
#define MULTIPLIES 'M'
#define LD_ST_OFF 'O'
#define LD_ST_REG 'R'
#define LD_ST_MUL 'L'
#define BRANCH 'B'


struct dis *arm_disassemble(int mode, struct trie_node *node, u8 *stream, long max, uint64_t addr, int *used_bytes);

#endif

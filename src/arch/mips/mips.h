#ifndef MIPS_H
#define MIPS_H

#include <stdio.h>
#include <stdint.h>
#include "../../common/type.h"
#include "../../dis.h"
#include "mload.h"
#include "mstrings.h"

/*Instruction Entry Flags*/
#define INSTR_FUNC 2
#define INSTR_SHIFT 4

/*Instruction Types*/
#define TYPE_R 'R'
#define TYPE_I 'I'
#define TYPE_J 'J'

/*Macros*/
#define OPCODE(instr) ((instr>>26))
#define FUNC(instr) ((instr&0x03f))
#define SHAMT(instr) ((instr>>6)&0x01f)
#define RS(instr) ((instr>>21)&0x01f)
#define RT(instr) ((instr>>16)&0x01f)
#define RD(instr) ((instr>>11)&0x01f)
#define ADDR(instr) ((instr&0x03ffffff))
#define IMM(instr) (instr&0x0ffff)

struct dis *mips_disassemble(int mode, struct trie_node *node, u8 *stream, long max, uint64_t addr, int *used_bytes);
void mips_decode_operands(struct dis *disas, struct mips_instr_entry *e, uint32_t instruction);
struct operand_tree *mips_decode_R(uint32_t instruction);
struct operand_tree *mips_decode_J(uint32_t instruction);
struct operand_tree *mips_decode_I(uint32_t instruction);

#endif

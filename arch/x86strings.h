#ifndef X86_STRINGS_H
#define X86_STRINGS_H

#include <stdio.h>
#include <string.h>

#define REG_BIN_IDX(idx) (idx / 4)

/*X86 General Registers*/
extern const char * general_registers[64];

/*X86 XMM Registers*/
extern const char * xmm_registers[8];

/*X86 MMX Registers*/
extern const char * mm_registers[8];

/*Operand Size Prefix Strings*/
extern const char *operand_size_prefix[4];

const char * get_register(int reg, int size, int rexb);
int get_register_index(const char *reg);

#endif

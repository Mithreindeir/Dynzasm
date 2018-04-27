#ifndef X86_H
#define X86_H

#define MODRM 1
#include "../include/dis.h"
#include "../include/trie.h"

void x86_disassemble(unsigned char *stream, long max, struct dis *disas);
struct operand_tree *x86_disassemble_operand(int addr_mode, int opr_size, int addr_size, unsigned char *stream, long max);

#endif

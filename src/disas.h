#ifndef DISAS_H
#define DISAS_H

/*Disassembler*/
#include "dis.h"

typedef void(*disassemble)(unsigned char *stream, long max, struct dis *disas);

#endif

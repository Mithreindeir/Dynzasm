#include "x86strings.h"

const char *get_register(int reg, int size, int rexb)
{
	int idx = reg * 4 + size-1 + rexb  * 32;
	if (idx >= (sizeof(general_registers)/sizeof(char*)) return NULL;
	return general_registers[idx];
}

int get_register_index(const char *reg)
{
	for (int i = 0; i < (sizeof(general_registers)/sizeof(char*)); i++) {
		if (!strcmp(reg, general_registers[i])) return i;
	}
	return -1;
}

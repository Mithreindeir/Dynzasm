#include "x86strings.h"

const char * general_registers[64] = {
	"al", "ax", "eax", "rax",
	"cl", "cx", "ecx", "rcx",
	"dl", "dx", "edx", "rdx",
	"bl", "bx", "ebx", "rbx",
	"ah", "sp", "esp", "rsp",
	"ch", "bp", "ebp", "rbp",
	"dh", "si", "esi", "rsi",
	"bh", "di", "edi", "rdi",
	"r8b", "r8w", "r8d", "r8",
	"r9b", "r9w", "r9d", "r9",
	"r10b", "r10w", "r10d", "r10",
	"r11b", "r11w", "r11d", "r11",
	"r12b", "r12w", "r12d", "r12",
	"r13b", "r13w", "r13d", "r13",
	"r14b", "r14w", "r14d", "r14",
	"r15b", "r15w", "r15d", "r15"
};

const char * operand_size_prefix[4] = {
	"byte", "word", "dword", "qword"
};

const char *get_register(int reg, int size, int rexb)
{
	int idx = reg * 4 + size-1 + rexb  * 32;
	if (idx >= (sizeof(general_registers)/sizeof(char*))) return NULL;
	return general_registers[idx];
}

int get_register_index(const char *reg)
{
	for (int i = 0; i < (sizeof(general_registers)/sizeof(char*)); i++) {
		if (!strcmp(reg, general_registers[i])) return i;
	}
	return -1;
}

#ifndef X86_STRINGS_H
#define X86_STRINGS_H

#define REG_BIN_IDX(idx) (idx / 4)

/*X86 General Registers*/
static const char * general_registers[] = {
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

const char * get_register(int reg, int size, int rexb);
int get_register_index(const char *reg);

#endif

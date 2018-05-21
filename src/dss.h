#ifndef DSS_H
#define DSS_H

/* Disassembly Semantic Specification
 * Group Numbers:
 *  - arithmetic 0
 *  - data processing 1
 *  - stack instruction 2
 *  - branch instruction 3
 *
 *
 * Flags (odiszapc):
 *  - overflow
 *  - direction
 *  - interrupt
 *  - sign
 *  - zero
 *  - acarry
 *  - parity
 *  - carry
 *
 * FORMAT:
 * [mnemonic] group-id
 * $w = $instruction-idx...
 * $r = $instruction-idx...
 * $rf = $read_flags
 * $mf = $modified_flags
 *
 *
 * [add] 0
 * $rw = $0, $r = $1
 * $rf=........
 * $mf=o..szapc
 *
 * [or] 0
 * $rw = $0, $r = $1
 * $rf=........
 * $mf=o..sz.pc
 *
 * [adc] 0
 * $rw = $0, $r = 1
 * $rf=.......c
 * $mf=o..szapc
 *
 * */

/*Disassembly Semantic*/
struct dsem {
	char *mnemonic;
	int *read, nread;
	int *write, nwrite;
	unsigned char flags;
	unsigned int group;
};

/*Returns populated dis semantic or null on error. Increments the file pointer to the end of the semantic*/
struct dsem *parse_semantic(const char **buffer, int size);

#endif

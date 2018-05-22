#ifndef DSS_H
#define DSS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DSEM_READ 1
#define DSEM_WRITE 2
#define DSEM_RW 3

#define O 128
#define D 64
#define I 32
#define S 16
#define Z 8
#define A 4
#define P 2
#define C 1

#define FVAL(c) (c=='o'?O:(c=='d'?D:(c=='i'?I:(c=='s'?S:(c=='z'?Z:(c=='a'?A:(c=='p'?P:(c=='c'?C:0))))))))
#define VALF(c) (c&O?'o':(c&D?'d':(c&I?'i':(c&S?'s':(c&Z?'z':(c&A?'a':(c&P?'p':(c&C?'c':'.'))))))))

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
 * f: $w = $instruction-idx...
 * f: $r = $instruction-idx...
 * f: $rf = $read_flags
 * f: $mf = $modified_flags
 *
 *
 * [add] 0
 * o: $rw = $0, $r = $1
 * f: $rf=........
 * f: $mf=o..szapc
 *
 * [or] 0
 * o: $rw = $0, $r = $1
 * f: $rf=........
 * f: $mf=o..sz.pc
 *
 * [adc] 0
 * o: $rw = $0, $r = 1
 * f: $rf=.......c
 * f: $mf=o..szapc
 *
 * */

/*Disassembly Semantic*/
struct dsem {
	char *mnemonic;
	int *read, nread;
	int *write, nwrite;
	unsigned char rflags;
	unsigned char mflags;
	unsigned int group;
};

/*Returns populated dis semantic or null on error. Increments the file pointer to the end of the semantic*/
struct dsem *parse_semantic(char **buffer);
/*Set operand */
void parse_rwoperands(struct dsem *sem, char *line);
/*Set flag bits*/
void parse_rwflags(struct dsem *sem, char *line);

struct dsem *dsem_init(char *mnemonic, int group);
void dsem_destroy(struct dsem *sem);
void dsem_add(struct dsem *sem, int val, int rw);
void dsem_print(struct dsem *sem);

#endif

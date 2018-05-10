#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "arch/x86/x86.h"
#include "arch/x86/x86load.h"
#include "common/trie.h"
#include "common/db.h"
#include "common/disk.h"
#include "common/ser.h"
#include "disas.h"

void disassemble(struct trie_node *root, unsigned char *out, long max)
{
	long bit = 0;
	long addr = 0;

	while (bit < max) {
		printf("%#08lx: ", addr);
		int used_bytes = 0;
		struct dis *disas = x86_disassemble(root, out+bit, max-bit, addr, &used_bytes);
		int oldbit = bit;
		bit += used_bytes;
		addr = bit;
		int w = 20;
		for (int i = oldbit; w>=2 && i < bit; i++) {
			printf("%02x", out[i]);
			w-=2;
		}
		while (w-->0) printf(" ");
		if (!disas) continue;
		char buf[256];
		w = 7;
		printf("%s ", disas->mnemonic);
		w -= strlen(disas->mnemonic);
		while (w-- > 0) printf(" ");
		for (int i = 0; i < disas->num_operands; i++) {
			operand_squash(buf, 256, disas->operands[i]);
			printf("%s%c ", buf, (i+1)<disas->num_operands?',':' ');
		}
		dis_destroy(disas);
		printf("\n");
	}
}

void disas_stdin(struct trie_node *root)
{
	char ch;
	long allocd=100;
	char *bbuf = malloc(allocd);
	int iter = 0;
	while (read(STDIN_FILENO, &ch, 1) > 0) {
		if ((iter + 1) >= allocd) {
			allocd += 100;
			bbuf = realloc(bbuf, allocd);
			memset(bbuf+iter, 0, allocd-iter-1);
		}
		if ((ch>='a'&&ch<='f') || (ch >= 0x30 && ch <= 0x39)) {
			bbuf[iter++]=ch;
		}
	}
	int maxout = iter/2 + 1;
	unsigned char *out = malloc(maxout);
	memset(out, 0, maxout);
	long max = ascii_to_hex(out, bbuf, iter);

	disassemble(root, out, max);

	free(bbuf);
	free(out);
}

int main(int argc, char ** argv)
{
	struct disassembler *ds = ds_init(X64_ARCH);

	//unsigned char bytes[] = "\x55\x89\xe5\x83\xec\x28\xc7\x45\xf4\x00\x00\x00\x00\x8b\x45\xf4\x8b\x00\x83\xf8\x0e\x75\x0c\xc7\x04\x24\x30\x87\x04\x08\xe8\xd3\xfe\xff\xff\xb8\x00\x00\x00\x00\xc9\xc3";
	char bytes[] = "554889e54883ec70897d9c4889759064"
	"488b042528000000488945f831c0bf01"
	"000000e822090000488945b0488b0596"
	"570000488b1597570000488945c04889"
	"55c8488b0590570000488b1591570000"
	"488945d0488955d8488b058a57000048"
	"8945e00fb70587570000668945e80fb6"
	"057e5700008845ea488d75c0488b45b0"
	"b900000000ba2a0000004889c7e8b009"
	"000048c745b800000000c745ac000000"
	"00eb33488b45b8488d4858488b45b848"
	"8d5038488b45b8488b80d80000004889"
	"c6488d3df1560000b800000000e887f8"
	"ffff8345ac01488b45b08b40103945ac"
	"7d23488b45b0488b40088b55ac4863d2"
	"48c1e2034801d0488b00488945";
	int size = sizeof(bytes)-1;
	unsigned char *out = malloc(size);
	memset(out, 0, size);
	long max = ascii_to_hex(out, bytes, size);

	ds_decode(ds, out, max, 0x0);
	struct dis *dis = NULL;
	DS_FOREACH(ds, dis) {
		printf("%#08lx:\t%s\t%s\n", dis->address, dis->mnemonic, dis->op_squash);
	}

	ds_destroy(ds);
	free(out);
	return 0;
	struct trie_node *root = trie_init(0, NULL);
	x86_parse(root);

	if (argc < 2) {
		disas_stdin(root);
	} else {
		FILE *fp = fopen(argv[1], "r");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			long size = ftell(fp);
			if (size > 0) {
				rewind(fp);
				unsigned char *buffer = malloc(size+1);
				memset(buffer, 0, size);
				if (fread(buffer, 1, size, fp) != (unsigned long)size) {
					printf("Error reading bytes from file\n");
					exit(1);
				}
				disassemble(root, buffer, size);
				free(buffer);
			} else {
				printf("Error ftell returns negative number\n");
				exit(1);
			}
		}
	}
	trie_destroy(root);
	return 0;
}

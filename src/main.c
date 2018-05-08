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

void disassemble(struct trie_node *root, unsigned char *out, long max)
{
	long bit = 0;
	long addr = 0;

	while (bit < max) {
		printf("%#08lx: ", addr);
		struct dis disas;
		int oldbit = bit;
		bit+=x86_disassemble(root, out+bit, max-bit, &disas);
		addr += bit;
		int w = 20;
		for (int i = oldbit; w>=2 && i < bit; i++) {
			printf("%02x", out[i]);
			w-=2;
		}
		while (w-->0) printf(" ");
		char buf[256];
		w = 7;
		printf("%s ", disas.mnemonic);
		w -= strlen(disas.mnemonic);
		while (w-- > 0) printf(" ");
		for (int i = 0; i < disas.num_operands; i++) {
			operand_squash(buf, 256, disas.operands[i]);
			printf("%s%c ", buf, (i+1)<disas.num_operands?',':' ');
			operand_tree_free(disas.operands[i]);
		}
		free(disas.operands);
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
	if (argc < 2) return 1;
	struct disk *d = disk_open(argv[1]);

	struct db_node *rt = deserialize(d);
	char input[256];
	struct db_key ink;
	while (1) {
		printf(">");
		fgets(input, 256, stdin);
		char * cmd = strtok(input, " \n");
		if (!cmd) continue;
		char *args[3] = { NULL, NULL, NULL };
		int iter = 0;
		while (iter < 3 && (args[iter++]=strtok(NULL, " \n")));
		//printf("command: %s, %s %s %s\n", cmd, args[0], args[1], args[2]);
		if (!strcmp(cmd, "set") && args[0] && args[1]) {
			ink.key = args[0];
			ink.ksize = strlen(args[0]);
			ink.psize = strlen(args[1])+1;
			ink.ptr = malloc(ink.psize);
			memcpy(ink.ptr, args[1], ink.psize-1);
			((char*)ink.ptr)[ink.psize-1] = 0;
			rt = db_insert(rt, ink);
			printf("%s -> %s\n", args[0], args[1]);
		} else if (!strcmp(cmd, "get") && args[0]) {
			ink.key = args[0];
			ink.ksize = strlen(args[0]);
			ink.ptr = NULL;
			char * str = db_lookup_value(rt, ink);
			printf("%s -> %s\n", args[0], str);
		} else if (!strcmp(cmd, "dump")) {
			db_node_print(rt, 0);
		} else if (!strcmp(cmd, "quit")) {
			break;
		} else {
			printf("Invalid command\n");
			break;
		}
	}
	db_node_print(rt, 0);

	serialize(rt, d);
	disk_write(d);

	db_node_destroy(rt);
	disk_destroy(d);
	return 0;
	struct db_node *rdb = db_node_init(NULL);
	struct db_key k1;
	char buf[64];
	char buf2[64];
	int total = 15;
	clock_t time = clock();
	for (int i = 0; i < total; i++) {
		//snprintf(buf2, 6, "%c%c%c", 'a'+i, 'b'+i, 'c'+i);
		snprintf(buf2, 6, "%d", i * 2 + 123);
		snprintf(buf, 64, "key%d", i);
		//snprintf(buf2, 64, "val%d", i);
		k1.key = buf;
		k1.ksize = strlen(buf);
		size_t b2 = strlen(buf2);
		char *str = malloc(b2+1);
		memcpy(str, buf2, b2);
		str[b2] = 0;
		k1.ptr = str;
		rdb = db_insert(rdb, k1);
	}
	clock_t elap = clock() - time;
	double et = (double)(elap)/CLOCKS_PER_SEC;
	printf("TIME %f AVG %f AND %f PER SEC\n", et, et/total, 1/(et/total));
	//db_node_print(rdb, 0);

	time = clock();
	struct db_key k;
	/*
	for (int i = 0 ; i < total; i++) {
		//snprintf(buf, 64, "key%d", 4);
		k.key = &i;
		k.ksize = sizeof(int);
		k.ptr = NULL;
		char * str = db_lookup_value(rdb, k);
		long n = strtol(str, NULL, 10);
		if (n!= (i*2+123))
				printf("ASKDLJASDJKASDJKSDA\n");
		//printf("%d\n", n);
	}*/
	elap = clock() - time;
	et = (double)(elap)/CLOCKS_PER_SEC;
	printf("TIME %f AVG %f AND %f PER SEC\n", et, et/total, 1/(et/total));
	db_node_print(rdb, 0);
	//serialize(rdb);

	db_node_destroy(rdb);
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

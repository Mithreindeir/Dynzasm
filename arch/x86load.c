#include "x86load.h"

int get_line(FILE *f, char *buf, long max)
{
	memset(buf, 0, max);
	char c;
	int iter = 0;
	int eof = 0;
	while ((c=fgetc(f)) != '\n' && !(eof=feof(f)))
		buf[iter++] = c;
	return eof;
}

void x86_parse(struct trie_node *root)
{
	FILE *fp = fopen("arch/x86.ins", "r");
	if (!fp) {
		printf("Error opening x86 instruction file\n");
		return;
	}

	char buf[64];
	while (!(get_line(fp, buf, 64))) {
		char *bytes = strtok(buf, " ");
		char *mnem = strtok(NULL, " ");
		char *op[3] = { NULL, NULL, NULL };
		int num_op = 0;
		while ((op[num_op++]=strtok(NULL, " ")));
		num_op--;
		if (!mnem) continue;
		struct x86_instr_entry *entry = malloc(sizeof(struct x86_instr_entry));
		strcpy(entry->mnemonic, mnem);
		for (int i = 0; i < num_op; i++)
			strcpy(entry->operand[i], op[i]);
		entry->num_op = num_op;
		unsigned char buffer[32];
		int len = strlen(bytes);
		char b = bytes[0];
		int bi = 0;
		for (int i = 0; (b = bytes[i], i < len); i+=2) {
			buffer[bi]=b>'9'?b-'a'+10:b-'0';
			b = bytes[i+1];
			buffer[bi] = (buffer[bi] << 4) | (b>'9'?b-'a'+10:b-'0');
			bi++;
		}
		trie_insert(root, buffer, len/2, entry);
	}

	fclose(fp);
}

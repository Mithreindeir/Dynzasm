#include "x86load.h"

/*Gets a line from a file or returns 1 on eof*/
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

/*Converts ascii hex to raw hex. EG "A" -> 0x0a */
long ascii_to_hex(unsigned char *out, char *in, long len)
{
	long j = 0;
	for (int i = 0; i < len; i+=2) {
		out[j]=in[i]>'9'?in[i]-'a'+10:in[i]-'0';
		out[j] = (out[j] << 4) | (in[i+1]>'9'?in[i+1]-'a'+10:in[i+1]-'0');
		j++;
	}
	return j;
}

void x86_parse(struct trie_node *root)
{
	FILE *fp = fopen("src/arch/x86/x86.ins", "r");
	if (!fp) {
		printf("Error opening x86 instruction file\n");
		return;
	}

	char buf[64];
	char *bytes = NULL, *mnem = NULL, *op[3] = { NULL, NULL, NULL };
	int num_op = 0;
	unsigned char flags = 0;
	/*Loop through lines in the files*/
	while (!(get_line(fp, buf, 64))) {
		bytes = strtok(buf, " ");
		mnem = strtok(NULL, " ");
		num_op = 0;
		flags = 0;
		/*Set the operands. If "f:" prefix then its a flag to set*/
		while ((op[num_op++]=strtok(NULL, " "))) {
			//Set flags
			if (!strncmp(op[num_op-1], "f:", 2)) {
				unsigned char n = strtol(op[num_op-1]+2, NULL, 10);
				op[num_op-1] = NULL;
				num_op--;
				flags |= n;
			}
		}
		num_op--;
		if (!mnem || !bytes) continue;

		/*Construct instruction entry from the lines strings*/
		struct x86_instr_entry *entry = malloc(sizeof(struct x86_instr_entry));
		strcpy(entry->mnemonic, mnem);
		for (int i = 0; i < num_op; i++)
			strcpy(entry->operand[i], op[i]);
		entry->num_op = num_op;

		/*Convert the key string to raw bytes and insert into trie*/
		unsigned char buffer[32];
		long blen = ascii_to_hex(buffer, bytes, strlen(bytes));
		trie_insert(root, buffer, blen, entry, flags);
	}

	fclose(fp);
}

#include "mload.h"

void mips_parse(struct trie_node *root, int mode)
{
	FILE *fp = NULL;
	if (mode == MODE_32B)
		fp = fopen("src/arch/mips/mips.ins", "r");
	if (!fp) {
		printf("Error opening mips instruction file\n");
		return;
	}
	char buf[64];
	char *bytes = NULL, *mnem = NULL, *type = NULL;
	while (!get_line(fp, buf, 64)) {
		bytes = strtok(buf, " ");
		mnem = strtok(NULL, " ");
		type = strtok(NULL, " ");
		if (!bytes || !mnem || !type) continue;
		/*Create instruction entry from line info: bytes mnem type func(optional)*/
		struct mips_instr_entry *entry = malloc(sizeof(struct x86_instr_entry));
		strncpy(entry->mnemonic, MAX_MNEM_SIZE_MIPS-1);
		entry->instr_type = *type;
		unsigned char buffer[32];
		long blen = ascii_to_hex(buffer, bytes, strlen(bytes));
		trie_insert(root, buffer, blen, entry, 0);
	}

	fclose(fp);
}

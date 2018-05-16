#include "mload.h"

void mips_parse(struct trie_node *root, int mode)
{
	FILE *fp = NULL;
	fp = fopen("./src/arch/mips/mips.ins", "r");
	(void)mode;/*32 and 64 mode have the same file, so mode isnt needed*/
	if (!fp) {
		printf("Error opening mips instruction file\n");
		return;
	}
	char buf[64];
	char *bytes = NULL, *mnem = NULL, *type = NULL, *sflags = NULL;
	unsigned char flags = 0;
	while (!get_line(fp, buf, 64)) {
		flags = 0;
		bytes = strtok(buf, " ");
		mnem = strtok(NULL, " ");
		type = strtok(NULL, " ");
		while ((sflags=strtok(NULL, " \n"))) {
			if (!strncmp(sflags, "f:", 2)) {
				unsigned char n = strtol(sflags+2, NULL, 10);
				flags |= n;
			}
		}
		if (!bytes || !mnem || !type) continue;
		/*Create instruction entry from line info: bytes mnem type func(optional)*/
		struct mips_instr_entry *entry = malloc(sizeof(struct mips_instr_entry));
		strncpy(entry->mnemonic, mnem, MAX_MNEM_SIZE_MIPS-1);
		entry->instr_type = *type;
		unsigned char buffer[32];
		long blen = ascii_to_hex(buffer, bytes, strlen(bytes));
		trie_insert(root, buffer, blen, entry, flags);
	}

	fclose(fp);
}

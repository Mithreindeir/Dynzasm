#include "aload.h"

void arm_parse(struct trie_node *root, int mode)
{
	FILE *fp = NULL;
	fp = fopen("./src/arch/arm/arm.ins", "r");
	if (!fp) {
		printf("Error opening mips instruction file\n");
		return;
	}
	char buf[64];

	char *bytes = NULL, *mnem = NULL, *sflags = NULL;
	unsigned char flags = 0;
	while (!get_line(fp, buf, 64)) {
		flags = 0;
		bytes = strtok(buf, " ");
		mnem = strtok(NULL, " ");

		while ((sflags=strtok(NULL, " \n"))) {
			if (!strncmp(sflags, "f:", 2)) {
				unsigned char n = strtol(sflags+2, NULL, 10);
				flags |= n;
			}
		}

		struct arm_instr_entry *entry = malloc(sizeof(struct arm_instr_entry));
		strncpy(entry->mnemonic, mnem, MAX_MNEM_SIZE_ARM-1);
		unsigned char buffer[32];
		long blen = ascii_to_hex(buffer, bytes, strlen(bytes));
		trie_insert(root, buffer, blen, entry, flags);
	}
	fclose(fp);
}

